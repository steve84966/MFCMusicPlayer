
// MFCMusicPlayerDlg.h: 头文件
//

#pragma once

#include "MusicPlayerSettingsManager.h"
#include "MusicPlayer.h"
#include "LrcManagerWnd.h"

class CProgressSliderCtrl : public CSliderCtrl
{
protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point)
	{
		CRect rcThumb;
		GetThumbRect(&rcThumb);

		if (rcThumb.PtInRect(point))
		{
			CSliderCtrl::OnLButtonDown(nFlags, point);
		}
		else
		{
			// 点击在其他区域，忽略
		}
	}

	DECLARE_MESSAGE_MAP()
};

class CProgressScrollBar : public CScrollBar
{
protected:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
	{
		if (nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION)
			CScrollBar::OnHScroll(nSBCode, nPos, pScrollBar);
	}

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
	{
		if (nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION)
			CScrollBar::OnVScroll(nSBCode, nPos, pScrollBar);
	}

	DECLARE_MESSAGE_MAP()
};


// CMFCMusicPlayerDlg 对话框
class CMFCMusicPlayerDlg : public CDialogEx
{
// 构造
public:
	explicit CMFCMusicPlayerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCMUSICPLAYER_DIALOG };
#endif

	protected:
	void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	MusicPlayer* music_player;

// 生成的消息映射函数
	BOOL OnInitDialog() override;
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
void OpenMusic(const CString& file_path, const CString& ext);
bool bIsMusicPlaying = false, bIsMusicPlayingStateRecorded = false;
	float fBasePlayTime = -1.f;
	bool bIsAdjustingLrcVertical = false;
	MusicPlayerSettingsManager settings_manager;
	DECLARE_MESSAGE_MAP()
public:
	static std::initializer_list<CString> music_ext_list;
	afx_msg void OnClickedButtonOpen();
	afx_msg void OnClickedButtonPlay();
	afx_msg void OnClickedButtonPause();
	afx_msg void OnClickedButtonTranslation();
	afx_msg void OnClickedButtonRomanization();
	afx_msg LRESULT OnPlayerFileInit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPlayerTimeChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPlayerPause(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPlayerStop(WPARAM wParam, LPARAM lParam);
	LRESULT OnAlbumArtInit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnCancel() override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	void LoadLyric(const CString& file_path);
	void DestroyMediaPlayer();
	CStatic m_labelTime;
	afx_msg void OnClickedButtonStop();
	CStatic m_labelAlbumArt;
	CProgressSliderCtrl m_sliderProgress;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);

	CSliderCtrl m_sliderVolumeCtrl;
	CLrcManagerWnd lrc_manager_wnd;
	CButton m_buttonTranslation;
	CButton m_buttonRomanization;
	CProgressScrollBar m_scrollBarLrcVertical;
	afx_msg void OnMenuAbout();
	[[noreturn]] afx_msg void OnMenuExit();
	afx_msg void OnMenuOpenCustomLrc();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMenuSettingPlayingTextFont();
	void ModifyPlayingText(bool is_translation);
	afx_msg void OnMenuSettingTranslationTextFont();

	void ModifyTextColor(bool is_playing);
	afx_msg void OnMenuSettingPlayedTextColor();
	afx_msg void OnMenuSettingUnplayedTextColor();
};
