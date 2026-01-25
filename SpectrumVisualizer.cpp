#include "pch.h"
#include "SpectrumVisualizer.h"
#include <fftw3.h>

BEGIN_MESSAGE_MAP(SpectrumVisualizer, CDialogEx)
    ON_WM_PAINT()
    ON_WM_MOVE()
    ON_WM_TIMER()
END_MESSAGE_MAP()

void SpectrumVisualizer::DoDataExchange(CDataExchange *pDX) {
    CDialogEx::DoDataExchange(pDX);
}

int SpectrumVisualizer::OnInitDialog()
{
    SetTimer(114515, 20, nullptr);
    spectrum_max_data.resize(32);
    std::ranges::fill(spectrum_max_data, -128.0f);
    return CDialogEx::OnInitDialog();
}

SpectrumVisualizer::SpectrumVisualizer(CWnd *pParent): CDialogEx(IDD_DIALOGSPECTRUM, pParent) {
    ATLTRACE("info: SpectrumVisualizer inited\n");
}

void SpectrumVisualizer::AddSamplesToRingBuffer(uint8_t* samples, int sample_size)
{
    if (samples == nullptr || sample_size <= 0)
        return;

    for (int i = 0; i < sample_size; ++i)
    {
        spectrum_data_ring_buffer.push_back(samples[i]);
    }

    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (spectrum_data_ring_buffer.size() > RING_BUFFER_MAX_SIZE)
    {
        spectrum_data_ring_buffer.pop_front();
    }
}

int SpectrumVisualizer::GetRingBufferSize() const
{
    return static_cast<int>(spectrum_data_ring_buffer.size());
}

void SpectrumVisualizer::ApplyWindow(const std::vector<uint8_t>& input, std::vector<double>& output)
{
    const size_t bytes_per_frame = 4;  // 2 channels * 2 bytes
    const size_t frame_count = input.size() / bytes_per_frame;
    output.resize(frame_count);

    for (size_t i = 0; i < frame_count; ++i) {
        auto left = static_cast<int16_t>(input[i * 4] | (input[i * 4 + 1] << 8));
        auto right = static_cast<int16_t>(input[i * 4 + 2] | (input[i * 4 + 3] << 8));
        // mix 2 channels
        double sample = (static_cast<double>(left) + static_cast<double>(right)) / 2.0;
        // hamming window
        const double w = 0.53836 * (1.0 - cos(2.0 * M_PI * i / (frame_count - 1)));
        // normalize
        output[i] = (sample / 32768.0) * w;
    }
}

void SpectrumVisualizer::DoFFT(const std::vector<double>& windowed_data, std::vector<float>& fft_result)
{
    size_t n = windowed_data.size();
    if (n == 0) return;

    size_t fft_size = 1;
    while (fft_size < n) fft_size <<= 1;
    double* in = fftw_alloc_real(fft_size);
    fftw_complex* out = fftw_alloc_complex(fft_size / 2 + 1);
    if (!in || !out) {
        if (in) fftw_free(in);
        if (out) fftw_free(out);
        return;
    }

    // 复制加窗后的数据
    for (size_t i = 0; i < n; ++i) {
        in[i] = windowed_data[i];
    }
    // 零填充
    for (size_t i = n; i < fft_size; ++i) {
        in[i] = 0.0;
    }

    fftw_plan plan = fftw_plan_dft_r2c_1d(static_cast<int>(fft_size), in, out, FFTW_ESTIMATE);

    if (plan) {
        fftw_execute(plan);

        // 幅度谱
        fft_result.resize(fft_size / 2);
        fft_result.resize(fft_size / 2);
        for (size_t i = 0; i < fft_size / 2; ++i) {
            double real = out[i][0];
            double imag = out[i][1];
            fft_result[i] = static_cast<float>(sqrt(real * real + imag * imag));
        }

        fftw_destroy_plan(plan);
    }

    fftw_free(in);
    fftw_free(out);
}

std::vector<size_t> SpectrumVisualizer::GenBoundaries(float sample_rate, size_t fft_size, size_t segment_num, float f_lo, float f_hi)
{
    std::vector<size_t> boundaries(segment_num + 1);
    float delta_f = sample_rate / fft_size;
    size_t max_bin = fft_size / 2; // 只用正频率部分

    for (size_t i = 0; i <= segment_num; ++i) {
        float fraction = static_cast<float>(i) / segment_num;
        float freq = f_lo * pow(f_hi / f_lo, fraction);        // 对数插值
        auto idx = static_cast<size_t>(freq / delta_f);
        if (idx > max_bin - 1) idx = max_bin - 1;
        boundaries[i] = idx;
    }
    // 去重保证范围非空（即每个段至少1bin）
    for (size_t i = 1; i < boundaries.size(); ++i)
        if (boundaries[i] <= boundaries[i-1]) boundaries[i] = boundaries[i-1]+1;
    if (boundaries.back() > max_bin) boundaries.back() = max_bin;
    return boundaries;
}

void SpectrumVisualizer::MapFreqToSegments(
    std::vector<float>& fft_result,
    std::vector<float>& segments,
    const std::vector<size_t>& bandBounds)
{
    size_t numSegments = bandBounds.size() - 1;
    segments.resize(numSegments);
    for (size_t i = 0; i < numSegments; ++i) {
        float maxVal = 0.0f;
        for (size_t j = bandBounds[i]; j < bandBounds[i+1]; ++j) {
            if (j >= fft_result.size()) break;
            if (fft_result[j] > maxVal) maxVal = fft_result[j];
        }
        segments[i] = maxVal;
    }
}

void SpectrumVisualizer::UpdateSpectrum()
{
    // 检查缓冲区是否有足够数据
    if (spectrum_data_ring_buffer.size() < RING_BUFFER_MAX_SIZE)
        return;

    // drain data
    std::vector raw_samples(spectrum_data_ring_buffer.begin(),
                            spectrum_data_ring_buffer.end());

    // windowing
    std::vector<double> windowed;
    ApplyWindow(raw_samples, windowed);
    if (windowed.empty())
        return;

    // FFT
    std::vector<float> fft_result;
    DoFFT(windowed, fft_result);

    if (fft_result.empty())
        return;

    // 44100hz, 32 segments
    constexpr float sample_rate = 44100.0f;
    constexpr size_t segment_num = 32;
    size_t fft_size = 1;
    while (fft_size < windowed.size()) fft_size <<= 1;

    auto boundaries = GenBoundaries(sample_rate, fft_size, segment_num);

    spectrum_data.clear();
    MapFreqToSegments(fft_result, spectrum_data, boundaries);

    for (size_t i=0; i < spectrum_data.size(); ++i) {
        float& val = spectrum_data[i];
        // transition db
        float db = 20.0f * log10f(val + 1e-6f);
        constexpr float db_min = 10.0f;   // supress noise
        constexpr float db_max = 45.0f;   // full
        val = (db - db_min) / (db_max - db_min);
        if (val < 0.0f) val = 0.0f;
        if (val > 1.0f) val = 1.0f;

        // high freq attenuation
        constexpr size_t high_freq_start = segment_num * 2 / 3;
        if (i >= high_freq_start) {
            float attenuation = 1.0f - 0.4f * static_cast<float>(i - high_freq_start) / (segment_num - high_freq_start);
            val *= attenuation;
        }
    }

    // time-domain smoothing
    if (spectrum_smooth_data.size() != spectrum_data.size()) {
        spectrum_smooth_data.resize(spectrum_data.size(), 0.0f);
    }

    for (size_t i = 0; i < spectrum_data.size(); ++i) {
        float smooth_factor = 0.75f;
        spectrum_smooth_data[i] = smooth_factor * spectrum_smooth_data[i]
                                 + (1.0f - smooth_factor) * spectrum_data[i];
    }

    spectrum_data = spectrum_smooth_data;

    /*
    CString spectrum_str = _T("info: Spectrum: [");
    for (size_t i = 0; i < spectrum_data.size(); ++i) {
        CString val;
        val.Format(_T("%.2f"), spectrum_data[i]);
        spectrum_str += val;
        if (i < spectrum_data.size() - 1)
            spectrum_str += _T(", ");
    }
    spectrum_str += _T("]\n");
    ATLTRACE(spectrum_str);
    */

    // Call gdi+ to initialize spectrum
}

void SpectrumVisualizer::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);
    dc.FillSolidRect(0, 0, rect.Width(), rect.Height(), RGB(0, 0, 0));

    constexpr float spectrum_seg_width = 18.0f;
    constexpr float spectrum_seg_distance = 2.0f;
    const int spectrum_seg_count = static_cast<int>(spectrum_data.size());
    const int spectrum_seg_start =
       (rect.Width() - static_cast<int>(spectrum_seg_width) * spectrum_seg_count - static_cast<int>(spectrum_seg_distance) * (spectrum_seg_count - 1)) / 2;

    for (int i = 0; i < spectrum_seg_count; ++i) {
        const float spectrum_seg_height = spectrum_data[i] * static_cast<float>(rect.Height());
        const float spectrum_seg_height_start = static_cast<float>(rect.Height()) - spectrum_seg_height;
        const float spectrum_seg_x_start = static_cast<float>(spectrum_seg_start)
            + static_cast<float>(i) * spectrum_seg_width
            + static_cast<float>(i - 1) * spectrum_seg_distance;
        dc.FillSolidRect(
            std::floor(spectrum_seg_x_start),
            std::floor(spectrum_seg_height_start),
            std::floor(spectrum_seg_width),
            std::floor(spectrum_seg_height),
            RGB(0, 255, 0));
        if (spectrum_seg_height > spectrum_max_data[i])
            spectrum_max_data[i] = spectrum_seg_height;
        else
            spectrum_max_data[i] -= 2.0f;
        if (spectrum_max_data[i] < 0.0f)
                spectrum_max_data[i] = 0.0f;

        dc.FillSolidRect(
        std::floor(spectrum_seg_x_start),
        std::floor(static_cast<float>(rect.Height()) - spectrum_max_data[i]),
        std::floor(spectrum_seg_width), 2,
            RGB(255, 255, 255));
    }
}

void SpectrumVisualizer::ResetSpectrum()
{
    spectrum_data_ring_buffer.clear();
    spectrum_data.clear();
    spectrum_data.resize(32);
    spectrum_max_data.resize(32);
    std::ranges::fill(spectrum_max_data, -128.0f);
    Invalidate(FALSE);
}

void SpectrumVisualizer::OnMove(int cx, int cy) {
    if (this->m_pParentWnd != nullptr)
    {
        CRect thisRect;
        this->GetWindowRect(&thisRect);
        CRect parentDlgRect;
        this->m_pParentWnd->GetWindowRect(&parentDlgRect);
        m_pParentWnd->MoveWindow(thisRect.left - parentDlgRect.Width(), thisRect.top, parentDlgRect.Width(), parentDlgRect.Height());
    }
    CDialogEx::OnMove(cx, cy);
}

void SpectrumVisualizer::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 114515)
    {
		UpdateSpectrum();
        Invalidate(FALSE);
    }
}