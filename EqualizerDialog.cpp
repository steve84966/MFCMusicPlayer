// EqualizerDialog.cpp: 实现文件
//

#include "pch.h"
#include "MFCMusicPlayer.h"
#include "afxdialogex.h"
#include "EqualizerDialog.h"
#include "MFCMusicPlayerDlg.h"

IMPLEMENT_DYNAMIC(InputDlg, CDialogEx)

BEGIN_MESSAGE_MAP(InputDlg, CDialogEx)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(EqualizerPresetDialog, CDialogEx)

constexpr int eq_preset[][10] = {
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // default
	{+6, +4, -5, +2, +3, +4, +4, +5, +5, +6}, // bass
	{+6, +4,  0, -2, -6, +1, +4, +6, +7, +9}, // rock
	{+4,  0, +1, +2, +3, +4, +5, +4, +3, +3}, // jazz
};

EqualizerPresetDialog::EqualizerPresetDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS_CHILDPAGE_EQUALIZER_PRESET, pParent),
	  preset_id(0)
{

}

EqualizerPresetDialog::~EqualizerPresetDialog() = default;

void EqualizerPresetDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PRESET, m_comboPreset);
}


void EqualizerPresetDialog::OnCbnSelChangeComboPreset()
{
	int index = m_comboPreset.GetCurSel();
	if (index == CB_ERR) return;
	if (index >= 0 && index < 4)
	{
		CSimpleArray<int> eq_bands;
		for (int i = 0; i < 10; ++i)
		{
			eq_bands.Add(eq_preset[index][i]);
		}
		auto dlg = reinterpret_cast<EqualizerDialog*>(GetParent());
		dlg->SetPreset(index);
		dlg->UpdateEqualizerUI(eq_bands);
		dlg->SetEqualizerBandByUI();
	}
}

BOOL EqualizerPresetDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_comboPreset.AddString(_T("默认"));
	m_comboPreset.AddString(_T("完美低音"));
	m_comboPreset.AddString(_T("极致摇滚"));
	m_comboPreset.AddString(_T("最毒人声"));
	m_comboPreset.SetCurSel(preset_id);
	return TRUE;
}

void EqualizerPresetDialog::SetPreset(int id)
{
	if (id > 0 && id < 4)
		preset_id = id;
}

BEGIN_MESSAGE_MAP(EqualizerPresetDialog, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_PRESET, &EqualizerPresetDialog::OnCbnSelChangeComboPreset)
END_MESSAGE_MAP()


// EqualizerDialog 对话框

IMPLEMENT_DYNAMIC(EqualizerDialog, CDialogEx)

EqualizerDialog::EqualizerDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS_CHILDPAGE_EQUALIZER, pParent), m_pParentDlg(nullptr), preset_id(0)
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
	SetEqualizerBandByUI();
}

void EqualizerDialog::OnClickedButtonResetSpectrum()
{
	CSimpleArray<int> eq_bands;
	for (int j = 0; j < 10; ++j)
	{
		m_sliderGain[j].SetPos(120);
		eq_bands.Add(0);
	}
	preset_id = 0;
	m_pParentDlg->UpdateEqualizer(eq_bands);
}

void EqualizerDialog::OnClickedButtonPresetSpectrum()
{
	EqualizerPresetDialog dlg(this);
	dlg.SetPreset(preset_id);
	dlg.DoModal();
}

void EqualizerDialog::OnClickedButtonInputSpectrum()
{
	CString equalizer_data{};
	for (int j = 0; j < 10; ++j)
	{
		const auto val = -static_cast<float>(m_sliderGain[j].GetPos() - 120) / 10.0f;
		equalizer_data.AppendFormat(_T("%d,"), static_cast<int>(val));
	}
	equalizer_data.TrimRight(_T(","));
	InputDlg dlg(this);
	dlg.m_strInput = equalizer_data;
	if (dlg.DoModal() == IDOK)
	{
		CSimpleArray<int> eq_bands;
		equalizer_data = dlg.m_strInput;
		ATLTRACE(_T("info: input equalizer = %s\n"), equalizer_data.GetString());

		int pos = 0;
		CString token = equalizer_data.Tokenize(_T(",; \t"), pos);

		while (!token.IsEmpty())
		{
			token.Trim();
			if (!token.IsEmpty())
			{
				int token_int = _ttoi(token);
				if (token_int < -12 || token_int > 12)
				{
					AfxMessageBox(_T("请输入-12db到12db之间的整数!"), MB_ICONEXCLAMATION | MB_OK);
					return;
				}
				eq_bands.Add(token_int);
			}
			token = equalizer_data.Tokenize(_T(",; \t"), pos);
		}

		if (eq_bands.GetSize() != 10)
		{
			AfxMessageBox(_T("请输入10个有效数值！"), MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		preset_id = 0;
		UpdateEqualizerUI(eq_bands);
		SetEqualizerBandByUI();
	}
}


void EqualizerDialog::UpdateEqualizerUI(CSimpleArray<int> eq_bands)
{
	for (int i = 0; i < eq_bands.GetSize(); ++i)
	{
		const int pos = static_cast<int>(-10.0f * eq_bands[i] + 120);
		m_sliderGain[i].SetPos(pos);
	}
}

void EqualizerDialog::SetEqualizerBandByUI()
{
	CSimpleArray<int> eq_bands;
	for (int i = IDC_SLIDER_31HZ, j = 0; i <= IDC_SLIDER_16KHZ; ++i, ++j)
	{
		const auto val = -static_cast<float>(m_sliderGain[j].GetPos() - 120) / 10.0f;
		eq_bands.Add(static_cast<int>(val));
	}

	m_pParentDlg->UpdateEqualizer(eq_bands);
}

[[nodiscard]] CSimpleArray<int> EqualizerDialog::GetEqualizerValue()
{
	CSimpleArray<int> eq_bands;
	for (int i = IDC_SLIDER_31HZ, j = 0; i <= IDC_SLIDER_16KHZ; ++i, ++j)
	{
		const auto val = -static_cast<float>(m_sliderGain[j].GetPos() - 120) / 10.0f;
		eq_bands.Add(static_cast<int>(val));
	}
	return eq_bands;
}

BEGIN_MESSAGE_MAP(EqualizerDialog, CDialogEx)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_BUTTONRESETSPECTRUM, &EqualizerDialog::OnClickedButtonResetSpectrum)
	ON_BN_CLICKED(IDC_BUTTONPRESETSPECTRUM, &EqualizerDialog::OnClickedButtonPresetSpectrum)
	ON_BN_CLICKED(IDC_BUTTONINPUTSPECTRUM, &EqualizerDialog::OnClickedButtonInputSpectrum)
END_MESSAGE_MAP()


// EqualizerDialog 消息处理程序
