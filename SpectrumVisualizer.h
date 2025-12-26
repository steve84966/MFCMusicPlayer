#pragma once
#include "pch.h"
#include "framework.h"
#include "resource.h"

struct AudioFrameData {
    std::vector<uint8_t> data;
    int samples;
};

class SpectrumVisualizer
    : public CDialogEx
{
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOGSPECTRUM };
#endif
    DECLARE_MESSAGE_MAP()

    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV 支持

public:
    SpectrumVisualizer(CWnd* pParent = nullptr);

protected:
    uint8_t* spectrum_data_ring_buffer;
    int spectrum_data_ring_buffer_size;
};

