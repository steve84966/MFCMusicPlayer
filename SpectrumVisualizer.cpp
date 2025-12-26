#include "pch.h"
#include "SpectrumVisualizer.h"

BEGIN_MESSAGE_MAP(SpectrumVisualizer, CDialogEx)
END_MESSAGE_MAP()

void SpectrumVisualizer::DoDataExchange(CDataExchange *pDX) {
    CDialogEx::DoDataExchange(pDX);
}

SpectrumVisualizer::SpectrumVisualizer(CWnd *pParent): CDialogEx(IDD_DIALOGSPECTRUM, pParent) {
}
