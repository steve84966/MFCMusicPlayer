#pragma once
#include "afxdialogex.h"
#include "framework.h"

class CMFCMusicPlayerDlg;


// EqualizerDialog 对话框

class EqualizerDialog : public CDialogEx
{
	DECLARE_DYNAMIC(EqualizerDialog)

public:
	EqualizerDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~EqualizerDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_CHILDPAGE_EQUALIZER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	CSliderCtrl m_sliderGain[10];

	BOOL OnInitDialog() override;
	CMFCMusicPlayerDlg *m_pParentDlg;
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClickedButtonResetSpectrum();
public:
	void SetParentDlg(CMFCMusicPlayerDlg* pParentDlg)
	{
		m_pParentDlg = pParentDlg;
	}
	void UpdateEqualizerUI(CSimpleArray<int> eq_bands);
protected:
	DECLARE_MESSAGE_MAP()
};
