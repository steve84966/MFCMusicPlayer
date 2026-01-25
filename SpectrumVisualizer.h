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

    static constexpr int RING_BUFFER_MAX_FRAMES = 2560;
    static constexpr int RING_BUFFER_MAX_SIZE = RING_BUFFER_MAX_FRAMES * 4;  // stereo 16-bit
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV 支持

public:
    SpectrumVisualizer(CWnd* pParent = nullptr);
    void AddSamplesToRingBuffer(uint8_t* samples, int sample_size);
    [[nodiscard]] int GetRingBufferSize() const;

    // apply window to ring buffer, convert to vector
    static void ApplyWindow(const std::vector<uint8_t>& input, std::vector<double>& output);
    static void DoFFT(const std::vector<double>& windowed_data, std::vector<float>&);
    static std::vector<size_t> GenBoundaries(float sample_rate, size_t fft_size, size_t segment_num, float f_lo = 20.0f, float f_hi = 20000.0f);
    static void MapFreqToSegments(std::vector<float>&, std::vector<float>&, const std::vector<size_t>&);
    void UpdateSpectrum();
    afx_msg void OnPaint();
    void ResetSpectrum();
    afx_msg void OnMove(int cx, int cy);

protected:
    // ring buffer, fix size=2560 samples
    std::deque<uint8_t> spectrum_data_ring_buffer;
    std::vector<float> spectrum_data;
};

