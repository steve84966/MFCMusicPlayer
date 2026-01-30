// EqualizerDialog.cpp: 实现文件
//

#include "pch.h"
#include "MFCMusicPlayer.h"
#include "afxdialogex.h"
#include "EqualizerDialog.h"
#include "MFCMusicPlayerDlg.h"


// EqualizerDialog 对话框

IMPLEMENT_DYNAMIC(EqualizerDialog, CDialogEx)

EqualizerDialog::EqualizerDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS_CHILDPAGE_EQUALIZER, pParent)
{

}

EqualizerDialog::~EqualizerDialog() = default;

void EqualizerDialog::DoDataExchange(CDataExchange* pDX)
{
	for (int i = IDC_SLIDER_31HZ, j = 0; i <= IDC_SLIDER_16KHZ; ++i, ++j)
	{
		DDX_Control(pDX, i, m_sliderGain[j]);
	}
	CDialogEx::DoDataExchange(pDX);
}

BOOL EqualizerDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int i = IDC_SLIDER_31HZ, j = 0; i <= IDC_SLIDER_16KHZ; ++i, ++j)
	{
		m_sliderGain[j].SetRange(0, 240);
		m_sliderGain[j].SetPos(120);
	}

	return TRUE; // return TRUE unless you set the focus to a control
}

void EqualizerDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSimpleArray<int> eq_bands;
	for (int i = IDC_SLIDER_31HZ, j = 0; i <= IDC_SLIDER_16KHZ; ++i, ++j)
	{
		const auto val = -static_cast<float>(m_sliderGain[j].GetPos() - 120) / 10.0f;
		eq_bands.Add(static_cast<int>(val));
	}
	CString eq_band_info;
	for (int i = 0; i < eq_bands.GetSize(); ++i)
	{
		eq_band_info.AppendFormat(_T("%d "), eq_bands[i]);
	}
	m_pParentDlg->UpdateEqualizer(eq_bands);
}

void EqualizerDialog::OnClickedButtonResetSpectrum()
{
	CSimpleArray<int> eq_bands;
	for (int j = 0; j < 10; ++j)
	{
		m_sliderGain[j].SetPos(120);
		eq_bands.Add(0);
	}
	m_pParentDlg->UpdateEqualizer(eq_bands);
}


void EqualizerDialog::UpdateEqualizerUI(CSimpleArray<int> eq_bands)
{
	for (int i = 0; i < eq_bands.GetSize(); ++i)
	{
		const int pos = static_cast<int>(-10.0f * eq_bands[i] + 120);
		m_sliderGain[i].SetPos(pos);
	}
}

BEGIN_MESSAGE_MAP(EqualizerDialog, CDialogEx)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_BUTTONRESETSPECTRUM, &EqualizerDialog::OnClickedButtonResetSpectrum)
END_MESSAGE_MAP()


// EqualizerDialog 消息处理程序
