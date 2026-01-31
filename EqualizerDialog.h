#pragma once
#include "afxdialogex.h"
#include "framework.h"

class CMFCMusicPlayerDlg;

class EqualizerPresetDialog : public CDialogEx
{
	DECLARE_DYNAMIC(EqualizerPresetDialog)

public:
	explicit EqualizerPresetDialog(CWnd* pParent = nullptr);
	~EqualizerPresetDialog() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_CHILDPAGE_EQUALIZER_PRESET };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	CComboBox m_comboPreset;
	afx_msg void OnCbnSelChangeComboPreset();

	int preset_id;
	BOOL OnInitDialog() override;
	DECLARE_MESSAGE_MAP()
public:
	void SetPreset(int id);
};

// EqualizerDialog 对话框

class EqualizerDialog : public CDialogEx
{
	DECLARE_DYNAMIC(EqualizerDialog)

public:
	explicit EqualizerDialog(CWnd* pParent = nullptr);   // 标准构造函数
	~EqualizerDialog() override;

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
	afx_msg void OnClickedButtonPresetSpectrum();
	int preset_id;
public:
	void SetParentDlg(CMFCMusicPlayerDlg* pParentDlg)
	{
		m_pParentDlg = pParentDlg;
	}
	void UpdateEqualizerUI(CSimpleArray<int> eq_bands);
	void SetEqualizerBandByUI();
	void SetPreset(const int id) { preset_id = id; }
	[[nodiscard]] int GetPreset() const { return preset_id; }
	[[nodiscard]] CSimpleArray<int> GetEqualizerValue();
protected:
	DECLARE_MESSAGE_MAP()
};
