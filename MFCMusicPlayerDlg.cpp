
// MFCMusicPlayerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCMusicPlayer.h"
#include "MFCMusicPlayerDlg.h"



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

BEGIN_MESSAGE_MAP(CProgressScrollBar, CScrollBar)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

// CMFCMusicPlayerDlg 对话框

std::initializer_list<CString> CMFCMusicPlayerDlg::music_ext_list = {
	CString(_T("mp3")),
	CString(_T("flac")),
	CString(_T("wav")),
	CString(_T("wma")),
	CString(_T("aac")),
	CString(_T("ogg")),
	CString(_T("m4a")),
	CString(_T("ape")),
};

CString CMFCMusicPlayerDlg::get_common_dialog_music_filter()
{
	static CString filter = _T("Music Files (");
	if (filter != _T("Music Files (")) {
		return filter;
	}
	for (const auto& ext : music_ext_list) {
		filter += _T("*.") + ext + _T(";");
	}
	filter = filter.Left(filter.GetLength() - 1) + _T(")|");
	for (const auto& ext : music_ext_list) {
		filter += _T("*.") + ext + _T(";");
	}
	filter = filter.Left(filter.GetLength() - 1) + _T("||");
	return filter;
}


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
	DDX_Control(pDX, IDC_BUTTONROMANIZATION, m_buttonRomanization);
	DDX_Control(pDX, IDC_BUTTONSINGLELOOP, m_buttonSingleLoop);
	DDX_Control(pDX, IDC_SCROLLBARLRCVERTICAL, m_scrollBarLrcVertical);
}

BEGIN_MESSAGE_MAP(CMFCMusicPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTONOPEN, &CMFCMusicPlayerDlg::OnClickedButtonOpen)
	ON_COMMAND(ID_MENU_OPENFILE, &CMFCMusicPlayerDlg::OnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTONPLAY, &CMFCMusicPlayerDlg::OnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTONSTOP, &CMFCMusicPlayerDlg::OnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTONPAUSE, &CMFCMusicPlayerDlg::OnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTONTRANSLATION, &CMFCMusicPlayerDlg::OnClickedButtonTranslation)
	ON_BN_CLICKED(IDC_BUTTONROMANIZATION, &CMFCMusicPlayerDlg::OnClickedButtonRomanization)
	ON_BN_CLICKED(IDC_BUTTONSINGLELOOP, &CMFCMusicPlayerDlg::OnClickedButtonSingleLoop)
	ON_MESSAGE(WM_PLAYER_FILE_INIT, &CMFCMusicPlayerDlg::OnPlayerFileInit)
	ON_MESSAGE(WM_PLAYER_TIME_CHANGE, &CMFCMusicPlayerDlg::OnPlayerTimeChange)
	ON_MESSAGE(WM_PLAYER_PAUSE, &CMFCMusicPlayerDlg::OnPlayerPause)
	ON_MESSAGE(WM_PLAYER_STOP, &CMFCMusicPlayerDlg::OnPlayerStop)
	ON_MESSAGE(WM_PLAYER_TIME_CHANGE, &CMFCMusicPlayerDlg::OnPlayerTimeChange)
	ON_MESSAGE(WM_PLAYER_ALBUM_ART_INIT, &CMFCMusicPlayerDlg::OnAlbumArtInit)
	ON_COMMAND(ID_MENU_ABOUT, &CMFCMusicPlayerDlg::OnMenuAbout)
	ON_COMMAND(ID_MENU_EXIT, &CMFCMusicPlayerDlg::OnMenuExit)
	ON_COMMAND(ID__32771, &CMFCMusicPlayerDlg::OnMenuOpenCustomLrc)
	ON_COMMAND(ID_32773, &CMFCMusicPlayerDlg::OnMenuSettingPlayingTextFont)
	ON_COMMAND(ID_32774, &CMFCMusicPlayerDlg::OnMenuSettingTranslationTextFont)
	ON_COMMAND(ID_32777, &CMFCMusicPlayerDlg::OnMenuSettingPlayedTextColor)
	ON_COMMAND(ID_32778, &CMFCMusicPlayerDlg::OnMenuSettingUnplayedTextColor)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DROPFILES()
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
	m_scrollBarLrcVertical.SetScrollRange(0, 1000, TRUE);
	m_scrollBarLrcVertical.EnableWindow(FALSE);
	m_buttonTranslation.EnableWindow(FALSE);
	m_buttonRomanization.EnableWindow(FALSE);

	settings_manager.LoadIniOrDefault();
	lrc_manager_wnd.SubclassDlgItem(IDC_LRCDISPLAY, this);
	lrc_manager_wnd.LoadSettingsFromManager(settings_manager);

	m_buttonTranslation.ModifyStyle(0, BS_AUTOCHECKBOX | BS_PUSHLIKE);
	m_buttonRomanization.ModifyStyle(0, BS_AUTOCHECKBOX | BS_PUSHLIKE);

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICONFILEOPEN);
	CButton* pBtn = reinterpret_cast<CButton*>(GetDlgItem(IDC_BUTTONOPEN));
	pBtn->ModifyStyle(0, BS_ICON);
	pBtn->SetIcon(hIcon);
	hIcon = AfxGetApp()->LoadIcon(IDI_ICONSINGLELOOP);
	pBtn = reinterpret_cast<CButton*>(GetDlgItem(IDC_BUTTONSINGLELOOP));
	pBtn->ModifyStyle(0, BS_ICON | BS_AUTOCHECKBOX | BS_PUSHLIKE);
	pBtn->SetIcon(hIcon);

	// attach IDR_MENUMAIN to main window
	{
		CMenu menu;
		if (menu.LoadMenu(IDR_MENUMAIN))
		{
			CRect client_prev, wnd_prev;
			GetClientRect(&client_prev);
			GetWindowRect(&wnd_prev);
			HMENU h_menu = menu.Detach();
			this->SetMenu(CMenu::FromHandle(h_menu));
			CRect client_next;
			GetClientRect(&client_next);
			int delta = client_prev.Height() - client_next.Height();
			if (delta > 0)
			{
				// fix: client area delta
				SetWindowPos(nullptr, 0, 0, wnd_prev.Width(), wnd_prev.Height() + delta, SWP_NOMOVE | SWP_NOZORDER);
			}
		}
	}
	
	// get command line and judge, with a path then try open
	LPTSTR cmd_line = GetCommandLine();
	int argc;
	ATLTRACE(_T("info: command line: %s\n"), cmd_line);
	// skip exe path
	LPTSTR* str = CommandLineToArgvW(cmd_line, &argc); // NOLINT(*-unused-function)
	for (int i = 0; i < argc; i++)
	{
		ATLTRACE(_T("info: argv[%d]=%s\n"), i, str[i]);
	}
	if (argc == 2 && str[1] != nullptr) {
		CString file_path = str[1];
		ATLTRACE(_T("info: hit argv[1], file path = %s\n"), file_path.GetString());
		CString ext = file_path.Mid(file_path.ReverseFind(_T('.')) + 1);
		if (ext.IsEmpty()) {
			ATLTRACE(_T("err: file ext is empty!\n"));
		}
		else {
			OpenMusic(file_path, ext);
		}
		LocalFree(str);
	}
	DragAcceptFiles(TRUE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCMusicPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (UINT command_item = (nID & 0xFFF0); command_item == IDM_ABOUTBOX)
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

void CMFCMusicPlayerDlg::OpenMusic(const CString& file_path, const CString& ext)
{
	if (!file_path.IsEmpty() && !ext.IsEmpty())
	{
		delete music_player;
		music_player = new MusicPlayer();
		// judge ext is music file?
		if (std::ranges::find(music_ext_list, ext) == music_ext_list.end()) {
			ATLTRACE(_T("err: file ext %s not supported!\n"), ext.GetString());
			AfxMessageBox(_T("不支持的文件格式！"), MB_ICONERROR);
			return;
		}
		music_player->OpenFile(file_path, ext);
		if (!music_player->IsInitialized()) {
			delete music_player;
			music_player = nullptr;
		}
		else {
			this->PostMessage(WM_PLAYER_TIME_CHANGE, 0);
			CString title = music_player->GetSongTitle();
			CString artist = music_player->GetSongArtist();
			if (title.IsEmpty() || artist.IsEmpty()) {
				this->SetWindowText(file_path);
			}
			else {
				CString windowTitle;
				windowTitle.Format(_T("%s - %s"), artist.GetString(), title.GetString());
				this->SetWindowText(windowTitle);
			}
			CString lrc_file = file_path.Left(file_path.GetLength() - ext.GetLength() - 1) + _T(".lrc");
			ATLTRACE(_T("info: lrc file: %s\n"), lrc_file.GetString());
			LoadLyric(lrc_file);
		}
	}
}

void CMFCMusicPlayerDlg::OpenMusic(const CStringArray& array)
{
	playlist_controller.ClearPlaylist();
	for (int i = 0; i < array.GetCount(); i++) {
		ATLTRACE(_T("info: playlist file[%d]: %s\n"), i, array.GetAt(i).GetString());
		CString path = array.GetAt(i);
		playlist_controller.AddMusicFile(path);
	}
	if (array.GetCount() > 0)
	{
		CString ext = array.GetAt(0).Mid(array.GetAt(0).ReverseFind(_T('.')) + 1);;
		OpenMusic(array.GetAt(0), ext);
	}
}


void CMFCMusicPlayerDlg::OnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, NULL, NULL, // NOLINT(*-use-nullptr)
	                OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT,
	                get_common_dialog_music_filter());
	constexpr DWORD bufferSize = 64 * 1024;
	std::vector<TCHAR> buffer(bufferSize);
	dlg.m_ofn.lpstrFile = buffer.data();
	dlg.m_ofn.nMaxFile = bufferSize;
	if (dlg.DoModal() == IDOK)
	{
		CStringArray paths;
		POSITION pos = dlg.GetStartPosition();
		while (pos)
		{
			CString path = dlg.GetNextPathName(pos);
			ATLTRACE(_T("info: selected file path: %s\n"), path.GetString());
			paths.Add(path);
			// 在这里处理每个文件
		}
		OpenMusic(paths);
	}
}

void CMFCMusicPlayerDlg::OnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	if (music_player) {
		float volume = static_cast<float>(m_sliderVolumeCtrl.GetPos()) / 100.0f;
		music_player->SetMasterVolume(volume);
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

void CMFCMusicPlayerDlg::OnClickedButtonTranslation()
{
	// get button check state
	BOOL bCheckedTranslation = m_buttonTranslation.GetCheck() == BST_CHECKED;
	if (BOOL bCheckedRomanization = m_buttonRomanization.GetCheck(); bCheckedRomanization == BST_CHECKED) {
		m_buttonRomanization.SetCheck(BST_UNCHECKED);
		lrc_manager_wnd.SetRomanizationEnabled(false);
	}
	lrc_manager_wnd.SetTranslationEnabled(bCheckedTranslation);
}

void CMFCMusicPlayerDlg::OnClickedButtonRomanization() {
	// TODO: 在此添加控件通知处理程序代码
	BOOL bCheckedRomanization = m_buttonRomanization.GetCheck() == BST_CHECKED;
	if (BOOL bCheckedTranslation = m_buttonTranslation.GetCheck(); bCheckedTranslation == BST_CHECKED) {
		m_buttonTranslation.SetCheck(BST_UNCHECKED);
		lrc_manager_wnd.SetTranslationEnabled(false);
	}
	lrc_manager_wnd.SetRomanizationEnabled(bCheckedRomanization);
}

void CMFCMusicPlayerDlg::OnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	if (music_player) {
		music_player->Stop();
	}
	fBasePlayTime = 0.f;
	m_scrollBarLrcVertical.SetScrollPos(0, TRUE);
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
	ATLTRACE("info: player notify stop, reset fBasePlayTime\n");
	fBasePlayTime = 0.f;
	PostMessage(WM_PLAYER_TIME_CHANGE, *reinterpret_cast<WPARAM*>(&fBasePlayTime));
	m_scrollBarLrcVertical.SetScrollPos(0, TRUE);
	if (bSingleLoop)
	{
		music_player->Start();
	}
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
	if (!(fBasePlayTime < 0.f))
	{
		if (fabs(time - fBasePlayTime) > 1.f)
		{
			ATLTRACE("info: abnormal time event, time=%f, base=%f", time, fBasePlayTime);
			return LRESULT();
		} else
		{
			fBasePlayTime = time;
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

	if (lrc_manager_wnd.IsValid() && !bIsAdjustingLrcVertical)
	{
		int current_lrc_node_index = lrc_manager_wnd.GetCurrentLrcNodeIndex();
		int lrc_node_count = lrc_manager_wnd.GetLrcNodeCount();
		int iCurVerticalPos = static_cast<int>(static_cast<float>(current_lrc_node_index) * 1000.0f / static_cast<float>(lrc_node_count));
	    m_scrollBarLrcVertical.SetScrollPos(iCurVerticalPos, TRUE);
	}

	// re-post message to LrcManagerWnd
	if (!bIsAdjustingLrcVertical)
		lrc_manager_wnd.PostMessage(WM_PLAYER_TIME_CHANGE, wParam);
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
	music_player = nullptr;
}

void CMFCMusicPlayerDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	auto trackPopupMenu = [](UINT32 nIDResource, CWnd* pWnd, CPoint point)
	{

		CMenu menu;
		if (menu.LoadMenu(nIDResource))
		{
			CMenu* pPopup = menu.GetSubMenu(0);
			ASSERT(pPopup != nullptr);

			pWnd->ClientToScreen(&point);
			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
								   point.x, point.y, AfxGetMainWnd());
		}
	};
	CRect rect;
	lrc_manager_wnd.GetClientRect(&rect);
	lrc_manager_wnd.ScreenToClient(&point);
	if (rect.PtInRect(point))
	{
		trackPopupMenu(IDR_MENULYRICCONTROL, &lrc_manager_wnd, point);
	}
	CDialogEx::OnContextMenu(pWnd, point);
}

void CMFCMusicPlayerDlg::LoadLyric(const CString &file_path) {
	lrc_manager_wnd.DestroyLrcController();
	int result = lrc_manager_wnd.InitLrcControllerWithFile(file_path);
	ATLTRACE(_T("info: lrc controller init result: %d\n"), result);
	if (lrc_manager_wnd.IsValid())
	{
		m_scrollBarLrcVertical.EnableWindow(TRUE);
		if (lrc_manager_wnd.IsAuxiliaryInfoEnabled(LrcAuxiliaryInfo::Translation))
		{
			m_buttonTranslation.EnableWindow(TRUE);
			m_buttonTranslation.SetCheck(BST_CHECKED);
			m_buttonRomanization.SetCheck(BST_UNCHECKED);
			lrc_manager_wnd.SetTranslationEnabled(true);
		} else
		{
			m_buttonTranslation.SetCheck(BST_UNCHECKED);
			m_buttonTranslation.EnableWindow(FALSE);
			lrc_manager_wnd.SetTranslationEnabled(false);
		}
		if (lrc_manager_wnd.IsAuxiliaryInfoEnabled(LrcAuxiliaryInfo::Romanization))
		{
			m_buttonRomanization.EnableWindow(TRUE);
		} else
		{
			m_buttonRomanization.EnableWindow(FALSE);
		}
	} else
	{
		m_scrollBarLrcVertical.EnableWindow(FALSE);
		m_buttonTranslation.SetCheck(BST_UNCHECKED);
		m_buttonTranslation.EnableWindow(FALSE);
		m_buttonRomanization.SetCheck(BST_UNCHECKED);
		m_buttonRomanization.EnableWindow(FALSE);
	}
}

void CMFCMusicPlayerDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	DestroyMediaPlayer();
	CDialogEx::OnCancel();
}

void CMFCMusicPlayerDlg::OnSize(UINT nType, int cx, int cy)
{
	if (nType == SIZE_MAXIMIZED)
    {
        ShowWindow(SW_RESTORE);
        return;
    }
	CDialogEx::OnSize(nType, cx, cy);
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
					CEvent doneEvent;
					if (WaitForSingleObject(doneEvent, 5) == WAIT_TIMEOUT)
						music_player->Start();
				}
				fBasePlayTime = fCurSelectedTime;
			}
			break;
		default:
			return;
		}
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMFCMusicPlayerDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	if (GetDlgItem(IDC_SCROLLBARLRCVERTICAL) == pScrollBar && music_player && music_player->IsInitialized()
		&& lrc_manager_wnd.IsValid())
	{

		if (nSBCode == SB_THUMBTRACK) {
			ATLTRACE("info: lrc vertical slider is being dragged\n");
			bIsAdjustingLrcVertical = true;
		}
		if (nSBCode == SB_ENDSCROLL) {
			ATLTRACE("info: lrc vertical slider drag ended\n");
			SetTimer(1919810, 10, nullptr);
			goto exit;
		}
		bIsMusicPlaying = music_player->IsPlaying();

		int iSliderPos = nPos;
		int iCurrentLrcNodeIndex = lrc_manager_wnd.GetCurrentLrcNodeIndex();
		int iLrcNodeCount = lrc_manager_wnd.GetLrcNodeCount();
		int iTargetLrcNodeIndex = static_cast<int>(static_cast<float>(iSliderPos) * static_cast<float>(iLrcNodeCount) / 1000.0f);
		if (iTargetLrcNodeIndex >= iLrcNodeCount) 
			iTargetLrcNodeIndex = iLrcNodeCount - 1;
		if (iCurrentLrcNodeIndex != iTargetLrcNodeIndex) {
			int iTimeStampTarget = lrc_manager_wnd.GetLrcNodeTimeStamp(iTargetLrcNodeIndex);
			float fTarget = static_cast<float>(iTimeStampTarget) / 1000.f;
			lrc_manager_wnd.PostMessage(WM_PLAYER_TIME_CHANGE, *reinterpret_cast<UINT*>(&fTarget));
		}

		if (nSBCode == SB_THUMBPOSITION) {
			if (music_player && music_player->IsInitialized()) {
				int iTimeStampTarget = lrc_manager_wnd.GetLrcNodeTimeStamp(iTargetLrcNodeIndex);
				float fTarget = static_cast<float>(iTimeStampTarget) / 1000.f;
				bool is_music_playing = bIsMusicPlaying;
				if (fTarget > music_player->GetMusicTimeLength()) {
					music_player->Stop();
					goto exit;
				}
				fBasePlayTime = fTarget;
				int iCurVerticalPos = static_cast<int>(static_cast<float>(iTargetLrcNodeIndex) * 1000.0f / static_cast<float>(iLrcNodeCount));
				m_scrollBarLrcVertical.SetScrollPos(iCurVerticalPos, TRUE);
				music_player->SeekToPosition(fTarget, true);
				if (is_music_playing) {
					ATLTRACE("info: music is playing, resume from seek point\n");
					CEvent doneEvent;
					if (WaitForSingleObject(doneEvent, 5) == WAIT_TIMEOUT) {
						music_player->Start();
					}
				}
			}
		}

	}
exit:
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CMFCMusicPlayerDlg::OnMenuAbout() {
	CAboutDlg dlg;
	dlg.DoModal();
}

[[noreturn]]
void CMFCMusicPlayerDlg::OnMenuExit() {
	ExitProcess(0);
}

void CMFCMusicPlayerDlg::OnMenuOpenCustomLrc() {
	if (music_player && music_player->IsInitialized()) {
		CFileDialog dlg(TRUE, _T("lrc"), nullptr, OFN_FILEMUSTEXIST, _T("Lyric Files (*.lrc)|*.lrc||"));
		if (dlg.DoModal() == IDOK) {
			CString path = dlg.GetPathName();
			LoadLyric(path);
		}
	}
}

void CMFCMusicPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1919810){
		KillTimer(1919810);
		bIsAdjustingLrcVertical = false;
		ATLTRACE(_T("info: 114514\n"));
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CMFCMusicPlayerDlg::OnMenuSettingPlayingTextFont() {
	ModifyPlayingText(false);
}

void CMFCMusicPlayerDlg::ModifyPlayingText(bool is_translation) {
	CString font_name = lrc_manager_wnd.GetTextFont(is_translation);
	float font_size = lrc_manager_wnd.GetTextSize(is_translation) / 3.f * 4 * GetSystemDpiScale();
	bool bold = lrc_manager_wnd.IsTextBold(is_translation), italic = lrc_manager_wnd.IsTextItalic(is_translation);
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	_tcscpy_s(lf.lfFaceName, font_name.GetString());
	lf.lfHeight = static_cast<LONG>(font_size);
	lf.lfItalic = italic;
	lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
	CFontDialog dlg;
	dlg.m_cf.lpLogFont = &lf;
	dlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
	if (dlg.DoModal() == IDOK) {
		font_name = lrc_manager_wnd.GetDirectWriteFontName(&lf);
		if (BOOL result = lrc_manager_wnd.IsFontNameValid(font_name); !result) {
			ATLTRACE(_T("info: invalid font name %s, use default font\n"), font_name.GetString());
			font_name = _T("Microsoft YaHei UI");
		}
		font_size = static_cast<float>(dlg.GetSize()) / 10.f;
		bold = dlg.IsBold();
		italic = dlg.IsItalic();
		lrc_manager_wnd.ModifyTextFont(is_translation, font_name);
		lrc_manager_wnd.ModifyTextSize(is_translation, font_size);
		lrc_manager_wnd.ModifyTextBold(is_translation, bold);
		lrc_manager_wnd.ModifyTextItalic(is_translation, italic);
		lrc_manager_wnd.ReInitializeDirect2D();
	}
	if (is_translation) {
		settings_manager.SetLyricAuxFontName(font_name);
		settings_manager.SetLyricAuxFontSize(static_cast<int>(font_size));
		settings_manager.SetLyricAuxFontBold(bold);
		settings_manager.SetLyricAuxFontItalic(italic);
	}
	else {
		settings_manager.SetLyricFontName(font_name);
		settings_manager.SetLyricFontSize(static_cast<int>(font_size));
		settings_manager.SetLyricFontBold(bold);
		settings_manager.SetLyricFontItalic(italic);
	}
	settings_manager.SaveIni();
}

void CMFCMusicPlayerDlg::OnMenuSettingTranslationTextFont()
{
	// TODO: 在此添加命令处理程序代码
	ModifyPlayingText(true);
}

void CMFCMusicPlayerDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT fileCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	CString openFile, ext;

	for (UINT i = 0; i < fileCount; i++)
	{
		TCHAR filePath[MAX_PATH] = {0};
		DragQueryFile(hDropInfo, i, filePath, MAX_PATH);
		openFile = filePath;
		ext = openFile.Mid(openFile.ReverseFind(_T('.')) + 1);
	}
	DragFinish(hDropInfo);
	OpenMusic(openFile, ext);
	CDialogEx::OnDropFiles(hDropInfo);
}

void CMFCMusicPlayerDlg::ModifyTextColor(bool is_playing) {
	auto d2dColorToRGB = [](D2D1_COLOR_F color) {
		return RGB(static_cast<BYTE>(color.r * 255.f), static_cast<BYTE>(color.g * 255.f), static_cast<BYTE>(color.b * 255.f));
	};
	auto rgbToD2DColor = [](COLORREF color) {
		return D2D1::ColorF(GetRValue(color) / 255.f, GetGValue(color) / 255.f, GetBValue(color) / 255.f, 1.f);
	};
	D2D1::ColorF text_color = lrc_manager_wnd.GetTextColor(is_playing);
	COLORREF text_color_ref = d2dColorToRGB(text_color);
	CColorDialog dlg(text_color_ref, CC_FULLOPEN | CC_RGBINIT);
	if (dlg.DoModal() == IDOK) {
		COLORREF color = dlg.GetColor();
		D2D1::ColorF text_color_f = rgbToD2DColor(color);
		lrc_manager_wnd.ModifyTextColor(is_playing, text_color_f);
		if (is_playing) {
			settings_manager.SetLyricFontColor(color);
		} else
		{
			settings_manager.SetLyricFontColorTranslation(color);
		}
	}
	settings_manager.SaveIni();
}

void CMFCMusicPlayerDlg::OnMenuSettingPlayedTextColor() {
	ModifyTextColor(true);
}

void CMFCMusicPlayerDlg::OnMenuSettingUnplayedTextColor() {
	ModifyTextColor(false);
}

void CMFCMusicPlayerDlg::OnClickedButtonSingleLoop()
{
	bSingleLoop = m_buttonSingleLoop.GetCheck();
}
