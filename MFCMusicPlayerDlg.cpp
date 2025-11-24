
// MFCMusicPlayerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCMusicPlayer.h"
#include "MFCMusicPlayerDlg.h"
#include "afxdialogex.h"
#include <afxdlgs.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CProgressSliderCtrl, CSliderCtrl)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CMFCMusicPlayerDlg 对话框



CMFCMusicPlayerDlg::CMFCMusicPlayerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCMUSICPLAYER_DIALOG, pParent), music_player(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCMusicPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATICTIME, m_labelTime);
	DDX_Control(pDX, IDC_ALBUMART, m_labelAlbumArt);
	DDX_Control(pDX, IDC_SLIDERPROGRESS, m_sliderProgress);
	DDX_Control(pDX, IDC_SLIDERVOLUMECTRL, m_sliderVolumeCtrl);
	DDX_Control(pDX, IDC_BUTTONTRANSLATION, m_buttonTranslation);
}

BEGIN_MESSAGE_MAP(CMFCMusicPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTONOPEN, &CMFCMusicPlayerDlg::OnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTONPLAY, &CMFCMusicPlayerDlg::OnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTONSTOP, &CMFCMusicPlayerDlg::OnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTONPAUSE, &CMFCMusicPlayerDlg::OnClickedButtonPause)
	ON_MESSAGE(WM_PLAYER_FILE_INIT, &CMFCMusicPlayerDlg::OnPlayerFileInit)
	ON_MESSAGE(WM_PLAYER_TIME_CHANGE, &CMFCMusicPlayerDlg::OnPlayerTimeChange)
	ON_MESSAGE(WM_PLAYER_PAUSE, &CMFCMusicPlayerDlg::OnPlayerPause)
	ON_MESSAGE(WM_PLAYER_STOP, &CMFCMusicPlayerDlg::OnPlayerStop)
	ON_MESSAGE(WM_PLAYER_TIME_CHANGE, &CMFCMusicPlayerDlg::OnPlayerTimeChange)
	ON_MESSAGE(WM_PLAYER_ALBUM_ART_INIT, &CMFCMusicPlayerDlg::OnAlbumArtInit)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CMFCMusicPlayerDlg 消息处理程序

BOOL CMFCMusicPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	if (CMenu* pSysMenu = GetSystemMenu(FALSE); pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		UNREFERENCED_PARAMETER(bNameValid); // supress warning
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_sliderProgress.SetRangeMax(1000);
	m_sliderVolumeCtrl.SetRangeMax(100);
	m_sliderVolumeCtrl.SetPos(100);

	lrc_manager_wnd.SubclassDlgItem(IDC_LRCDISPLAY, this);

	m_buttonTranslation.ModifyStyle(0, BS_AUTOCHECKBOX | BS_PUSHLIKE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCMusicPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCMusicPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCMusicPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCMusicPlayerDlg::OnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, NULL, NULL, // NOLINT(*-use-nullptr)
	                OFN_FILEMUSTEXIST,
	                _T("Music Files (*.mp3;*.wav;*.flac;*.aac;*.ogg)|*.mp3;*.wav;*.flac;*.aac;*.ogg||"));
	if (dlg.DoModal() == IDOK)
	{
		CString path = dlg.GetPathName();   // Full path
		CString file = dlg.GetFileName();   // File name only
		CString ext = dlg.GetFileExt();    // Extension

		CString lrc_file = path.Left(path.GetLength() - ext.GetLength() - 1) + _T(".lrc");
		ATLTRACE(_T("info: lrc file: %s\n"), lrc_file.GetString());
		int result = lrc_manager_wnd.InitLrcControllerWithFile(lrc_file);
		ATLTRACE(_T("info: lrc controller init result: %d\n"), result);

		if (music_player) {
			// immediate stop playing
			delete music_player;
			music_player = nullptr;
		}
		music_player = new MusicPlayer();
		music_player->OpenFile(path, ext);
		if (!music_player->IsInitialized()) {
			delete music_player;
			music_player = nullptr;
			return;
		}
		// AfxMessageBox(_T("加载成功！"), MB_ICONINFORMATION);

		CString title = music_player->GetSongTitle();
		CString artist = music_player->GetSongArtist();
		this->PostMessage(WM_PLAYER_TIME_CHANGE, 0);
		if (title.IsEmpty() || artist.IsEmpty()) {
			this->SetWindowText(file);
		}
		else {
			CString windowTitle;
			windowTitle.Format(_T("%s - %s"), artist.GetString(), title.GetString());
			this->SetWindowText(windowTitle);
		}
	}
}

void CMFCMusicPlayerDlg::OnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	if (music_player) {
		music_player->Start();
	}
}

void CMFCMusicPlayerDlg::OnClickedButtonPause()
{
	// TODO: 在此添加控件通知处理程序代码
	if (music_player) {
		music_player->Pause();
	}
}

void CMFCMusicPlayerDlg::OnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	if (music_player) {
		music_player->Stop();
	}
}

LRESULT CMFCMusicPlayerDlg::OnPlayerFileInit(WPARAM wParam, LPARAM lParam) // NOLINT(*-convert-member-functions-to-static)
{
	// TODO: 在此添加控件通知处理程序代码
	fBasePlayTime = 0.f;
	PostMessage(WM_PLAYER_TIME_CHANGE, *reinterpret_cast<WPARAM*>(&fBasePlayTime));
	return 0;
}

LRESULT CMFCMusicPlayerDlg::OnPlayerPause(WPARAM wParam, LPARAM lParam) // NOLINT(*-convert-member-functions-to-static)
{
	return LRESULT();
}

LRESULT CMFCMusicPlayerDlg::OnPlayerStop(WPARAM wParam, LPARAM lParam) // NOLINT(*-convert-member-functions-to-static)
{
	fBasePlayTime = 0.f;
	PostMessage(WM_PLAYER_TIME_CHANGE, *reinterpret_cast<WPARAM*>(&fBasePlayTime));
	return LRESULT();
}

LRESULT CMFCMusicPlayerDlg::OnAlbumArtInit(WPARAM wParam, LPARAM lParam)
{
	if (HBITMAP album_art = reinterpret_cast<HBITMAP>(wParam)) {  // NOLINT(*-use-auto)
		m_labelAlbumArt.SetBitmap(album_art);
	}
	else {
		HBITMAP no_image = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_NOIMAGE));
		if (float aspect_ratio = GetSystemDpiScale(); fabs(aspect_ratio - 1.0f) > 1e-6) {
			HDC hScreenDC = ::GetDC(NULL); // NOLINT(*-use-nullptr)
			HDC hSrcDC = ::CreateCompatibleDC(hScreenDC);
			HDC hDstDC = ::CreateCompatibleDC(hScreenDC);

			HBITMAP hOldSrcBmp = (HBITMAP)::SelectObject(hSrcDC, no_image); // NOLINT(*-use-auto)
			HBITMAP hScaledBitmap = ::CreateCompatibleBitmap(hScreenDC, static_cast<int>(160 * aspect_ratio), static_cast<int>(160 * aspect_ratio));
			HBITMAP hOldDstBmp = (HBITMAP)::SelectObject(hDstDC, hScaledBitmap); // NOLINT(*-use-auto)
			::SetStretchBltMode(hDstDC, HALFTONE); // Better quality
			::StretchBlt(
				hDstDC, 0, 0, static_cast<int>(160 * aspect_ratio), static_cast<int>(160 * aspect_ratio),
				hSrcDC, 0, 0, 160, 160,
				SRCCOPY
			);
			::SelectObject(hSrcDC, hOldSrcBmp);
			::SelectObject(hDstDC, hOldDstBmp);
			::DeleteDC(hSrcDC);
			::DeleteDC(hDstDC);
			::ReleaseDC(nullptr, hScreenDC);
			m_labelAlbumArt.SetBitmap(hScaledBitmap);
		}
		else {
			m_labelAlbumArt.SetBitmap(no_image);
		}
	}
	return HRESULT();
}

// ReSharper disable once CppDFAConstantFunctionResult
LRESULT CMFCMusicPlayerDlg::OnPlayerTimeChange(WPARAM wParam, LPARAM lParam)
{
	static CString prev_timeStr = _T("");
	UINT32 raw = static_cast<UINT32>(wParam); // NOLINT(*-use-auto)
	float time = *reinterpret_cast<float*>(&raw);
	float length = music_player->GetMusicTimeLength();
	if (fBasePlayTime > 0.f)
	{
		if (fabs(time - fBasePlayTime) > 1.f)
		{
			ATLTRACE("info: abnormal time event, time=%f, base=%f", time, fBasePlayTime);
			time = fBasePlayTime;
		}
	}
	CString timeStr;
	int min = static_cast<int>(time) / 60, sec = static_cast<int>(time) % 60;
	timeStr.Format(_T("%02d:%02d / %02d:%02d"), min, sec, static_cast<int>(length) / 60, static_cast<int>(length) % 60);
	if (timeStr.Compare(prev_timeStr) != 0) {
		m_labelTime.SetWindowText(timeStr);
		prev_timeStr = timeStr;
	}
	// set slider
	float ratio = time / length;
	// if user is controlling the slider, do not adjust
	m_sliderProgress.SetPos(static_cast<int>(ratio * 1000));

	// re-post message to LrcManagerWnd
	lrc_manager_wnd.PostMessage(WM_PLAYER_TIME_CHANGE, wParam);
	if (fBasePlayTime > 0.f)
		fBasePlayTime = time;
	return LRESULT();
}

void CMFCMusicPlayerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnClose();
}

void CMFCMusicPlayerDlg::DestroyMediaPlayer()
{
	// TODO: 在此处添加实现代码.
	delete music_player;
}

void CMFCMusicPlayerDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	DestroyMediaPlayer();
	CDialogEx::OnCancel();
}
void CMFCMusicPlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (GetDlgItem(IDC_SLIDERVOLUMECTRL) == pScrollBar) {
		// volume change event
		int iMasterVolume = m_sliderVolumeCtrl.GetPos();
		if (music_player) {
			music_player->SetMasterVolume(static_cast<float>(iMasterVolume) / 100.0f);
		}
	}
	if (GetDlgItem(IDC_SLIDERPROGRESS) == pScrollBar && music_player && music_player->IsInitialized())
	{
		// seek event
		int iSliderPos = m_sliderProgress.GetPos();
		switch (nSBCode)
		{
		case SB_THUMBTRACK:
			if (!bIsMusicPlayingStateRecorded)
			{
				bIsMusicPlayingStateRecorded = true;
				bIsMusicPlaying = music_player->IsPlaying();
				if (music_player->IsPlaying())
					music_player->Pause();
			}
			break;
		case SB_THUMBPOSITION:
			bIsMusicPlayingStateRecorded = false;
			if (music_player && music_player->IsInitialized()) {
				float fCurSelectedTime = static_cast<float>(iSliderPos) / 1000.0f * music_player->GetMusicTimeLength();
				bool is_music_playing = bIsMusicPlaying;
				music_player->SeekToPosition(fCurSelectedTime, true);
				if (is_music_playing) {
					ATLTRACE("info: music is playing, resume from seek point\n");
					Sleep(5);
					music_player->Start();
					fBasePlayTime = fCurSelectedTime;
				}
			}
			break;
		default:
			return;
		}


	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
