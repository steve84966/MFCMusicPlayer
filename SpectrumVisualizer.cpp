#include "pch.h"
#include "SpectrumVisualizer.h"
#include <fftw3.h>

BEGIN_MESSAGE_MAP(SpectrumVisualizer, CDialogEx)
END_MESSAGE_MAP()

void SpectrumVisualizer::DoDataExchange(CDataExchange *pDX) {
    CDialogEx::DoDataExchange(pDX);
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
    const size_t fft_size = input.size();
    output.resize(fft_size);
    for (size_t i = 0; i < fft_size; ++i) {
        // hann窗系数
        const double w = 0.5 * (1.0 - cos(2.0 * M_PI * i / (fft_size - 1)));
        // 信号居中
        output[i] = (static_cast<double>(input[i]) - 128.0) * w;
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
        for (size_t i = 0; i < fft_size / 2; ++i) {
            double real = out[i][0];
            double imag = out[i][1];
            fft_result[i] = static_cast<float>(sqrt(real * real + imag * imag) / fft_size);
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

    // 从环形缓冲区提取数据
    std::vector raw_samples(spectrum_data_ring_buffer.begin(),
                            spectrum_data_ring_buffer.end());

    // 加窗
    std::vector<double> windowed;
    ApplyWindow(raw_samples, windowed);

    // FFT
    std::vector<float> fft_result;
    DoFFT(windowed, fft_result);

    if (fft_result.empty())
        return;

    // 44100hz, 16 segments
    constexpr float sample_rate = 44100.0f;
    constexpr size_t segment_num = 16;
    size_t fft_size = 1;
    while (fft_size < raw_samples.size()) fft_size <<= 1;

    auto boundaries = GenBoundaries(sample_rate, fft_size, segment_num);

    std::vector<float> segments;
    MapFreqToSegments(fft_result, segments, boundaries);

    CString spectrum_str = _T("info: Spectrum: [");
    for (size_t i = 0; i < segments.size(); ++i) {
        CString val;
        val.Format(_T("%.2f"), segments[i]);
        spectrum_str += val;
        if (i < segments.size() - 1)
            spectrum_str += _T(", ");
    }
    spectrum_str += _T("]\n");
    ATLTRACE(spectrum_str);
}