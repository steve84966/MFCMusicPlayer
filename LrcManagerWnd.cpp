#include "pch.h"
#include "LrcManagerWnd.h"

#pragma warning (disable: 4244) // only disable in this file. conversions are so much i cannot static_cast everyone.
// #include "Resource.h"

LRESULT CLrcManagerWnd::OnPlayerTimeChange(WPARAM wParam, LPARAM lParam)
{ // NOLINT(*-convert-member-functions-to-static) 服了clang-tidy又xjb乱报
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    UINT32 raw = static_cast<UINT32>(wParam); // NOLINT(*-use-auto)
    float time = *reinterpret_cast<float*>(&raw);

    int time_ms = static_cast<int>(time * 1000.0f);
    if (lrc_controller.valid())
    {
        lrc_controller.set_time_stamp(time_ms);
        this->Invalidate(FALSE);
    }
    return LRESULT();
}

IMPLEMENT_DYNAMIC(CLrcManagerWnd, CWnd)
BEGIN_MESSAGE_MAP(CLrcManagerWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_MESSAGE(WM_PLAYER_TIME_CHANGE, &CLrcManagerWnd::OnPlayerTimeChange)
END_MESSAGE_MAP()

CLrcManagerWnd::CLrcManagerWnd() :
    d2d1_factory(nullptr), render_target(nullptr),
    brush_unplay_text(nullptr), brush_played_text(nullptr),
    write_factory(nullptr), text_format(nullptr), text_format_translation(nullptr)
{
    COLORREF cr = ::GetSysColor(COLOR_BTNFACE);
    FLOAT r = GetRValue(cr) / 255.0f;
    FLOAT g = GetGValue(cr) / 255.0f;
    FLOAT b = GetBValue(cr) / 255.0f;

    default_dialog_color = D2D1::ColorF(r, g, b);

    UNREFERENCED_PARAMETER(
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d1_factory)
    );

    UNREFERENCED_PARAMETER(
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&write_factory))
    );
    InitDirectWrite();
}

CLrcManagerWnd::~CLrcManagerWnd()
{
    DiscardDeviceResources();
    if (text_format) text_format->Release();
    if (text_format_translation) text_format_translation->Release();
    if (write_factory) write_factory->Release();
    if (d2d1_factory) d2d1_factory->Release();
}

int CLrcManagerWnd::InitDirectWrite()
{
    if (write_factory)
    {
        UNREFERENCED_PARAMETER(
            write_factory->CreateTextFormat(
                text_customization.font_name, // TODO: customizable text format
                nullptr,
                text_customization.is_bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                text_customization.is_italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                text_customization.font_size * GetSystemDpiScale(),
                _T("zh-CN"),
                &text_format)
        );
        UNREFERENCED_PARAMETER(
            write_factory->CreateTextFormat(
                text_translation_customization.font_name, // TODO: customizable text format
                nullptr,
                text_translation_customization.is_bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                text_translation_customization.is_italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                text_translation_customization.font_size * GetSystemDpiScale(),
                _T("zh-CN"),
                &text_format_translation)
        );
        UNREFERENCED_PARAMETER(
            text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER)
        );
        UNREFERENCED_PARAMETER(
            text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)
        );
        UNREFERENCED_PARAMETER(
            text_format_translation->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER)
        );
        UNREFERENCED_PARAMETER(
            text_format_translation->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
        );
    }
    ATLTRACE("info: Lrc Direct2D surface created.\n");
    return 0;
}

int CLrcManagerWnd::InitLrcControllerWithFile(const CString& file_path, float song_duration)
{
    lrc_controller.set_song_duration(song_duration);
    lrc_controller.parse_lrc_file(file_path);
    Invalidate(FALSE);
    return lrc_controller.valid();
}

int CLrcManagerWnd::InitLrcControllerWithStream(const CString& stream, float song_duration)
{
    lrc_controller.set_song_duration(song_duration);
    CStringA input;
    LPCTSTR stream_str = stream.GetString();
    int input_size = WideCharToMultiByte(CP_UTF8, 0, stream_str, -1, nullptr, 0, nullptr, nullptr);
    LPSTR buffer = input.GetBufferSetLength(input_size);
    WideCharToMultiByte(CP_UTF8, 0, stream_str, -1, buffer, input_size, nullptr, nullptr);
    input.ReleaseBuffer();
    CFile* file = new CMemFile(reinterpret_cast<BYTE*>(const_cast<CHAR*>(input.GetString())), input_size);
    lrc_controller.parse_lrc_file_stream(file);
    delete file;
    Invalidate(FALSE);
    return lrc_controller.valid();
}

void CLrcManagerWnd::DestroyLrcController()
{
    lrc_controller.clear_lrc_nodes();
    lrc_controller.reset_auxiliary_info_enabled();
}

void CLrcManagerWnd::UpdateLyric()
{
    CRect rc;
    GetClientRect(&rc);
    if (lrc_controller.valid() && render_target && brush_unplay_text && brush_played_text && text_format)
    {
        CString lyric_main_text;
        lrc_controller.get_current_lrc_line_at(lrc_controller.get_current_lrc_line_aux_index(LrcAuxiliaryInfo::Lyric),
                                               lyric_main_text);

        float lyric_main_metrics_width, lyric_main_metrics_height;
        MeasureTextMetrics(lyric_main_text, static_cast<float>(rc.right - rc.left), &lyric_main_metrics_width,
                           &lyric_main_metrics_height);

        float center_y = (static_cast<float>(rc.Height()) - lyric_main_metrics_height) / 2 - 20;

        D2D1_RECT_F center_lyric_layout = D2D1::RectF(rc.left, center_y, rc.right,
                                                      center_y + lyric_main_metrics_height);

        bool percentage_enabled = lrc_controller.is_percentage_enabled(lrc_controller.get_current_lrc_node_index());
        if (percentage_enabled)
        {
            float percentage = lrc_controller.get_lrc_percentage(lrc_controller.get_current_lrc_node_index());

            render_target->DrawText(
                lyric_main_text.GetString(),
                lyric_main_text.GetLength(),
                text_format,
                &center_lyric_layout,
                brush_unplay_text);

            // text_layout: line info
            Microsoft::WRL::ComPtr<IDWriteTextLayout> text_layout;
            write_factory->CreateTextLayout(
                lyric_main_text.GetString(),
                lyric_main_text.GetLength(),
                text_format,
                static_cast<float>(rc.right - rc.left),
                FLT_MAX,
                &text_layout);

            if (text_layout)
            {
                text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                UINT32 line_count = 0;
                text_layout->GetLineMetrics(nullptr, 0, &line_count);

                if (line_count <= 1)
                {
                    float played_width = lyric_main_metrics_width * percentage;
                    float text_left = (rc.right - rc.left - lyric_main_metrics_width) / 2.0f;

                    D2D1_RECT_F clip_rect = D2D1::RectF(
                        text_left,
                        center_y,
                        text_left + played_width,
                        center_y + lyric_main_metrics_height
                    );

                    render_target->PushAxisAlignedClip(clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    render_target->DrawText(
                        lyric_main_text.GetString(),
                        lyric_main_text.GetLength(),
                        text_format,
                        &center_lyric_layout,
                        brush_played_text);
                    render_target->PopAxisAlignedClip();
                }
                else
                {
                    // get all line metrics
                    std::vector<DWRITE_LINE_METRICS> line_metrics(line_count);
                    text_layout->GetLineMetrics(line_metrics.data(), line_count, &line_count);

                    // calc sum of width
                    float total_width = 0.0f;
                    std::vector<float> line_widths(line_count);
                    UINT32 char_pos = 0;

                    for (UINT32 i = 0; i < line_count; ++i)
                    {
                        UINT32 visible_length = line_metrics[i].length - line_metrics[i].trailingWhitespaceLength;
                        if (visible_length > 0)
                        {
                            DWRITE_HIT_TEST_METRICS hit{};
                            float x, y;
                            // get end font pos
                            UNREFERENCED_PARAMETER(
                                text_layout->HitTestTextPosition(char_pos + visible_length - 1, TRUE, &x, &y, &hit)
                            );
                            DWRITE_HIT_TEST_METRICS hit_start{};
                            float x_start, y_start;
                            // get start font pos
                            UNREFERENCED_PARAMETER(
                            text_layout->HitTestTextPosition(char_pos, FALSE, &x_start, &y_start, &hit_start)
                            );
                            line_widths[i] = hit.left + hit.width - hit_start.left;
                        }
                        else
                        {
                            line_widths[i] = 0.0f;
                        }
                        total_width += line_widths[i];
                        char_pos += line_metrics[i].length;
                    }

                    float played_total_width = total_width * percentage;
                    float accumulated_width = 0.0f;
                    float current_y = center_y;
                    char_pos = 0;

                    for (UINT32 i = 0; i < line_count; ++i)
                    {
                        float line_height = line_metrics[i].height;

                        if (accumulated_width + line_widths[i] <= played_total_width)
                        {
                            D2D1_RECT_F line_clip = D2D1::RectF(
                                static_cast<float>(rc.left),
                                current_y,
                                static_cast<float>(rc.right),
                                current_y + line_height
                            );

                            render_target->PushAxisAlignedClip(line_clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                            render_target->DrawText(
                                lyric_main_text.GetString(),
                                lyric_main_text.GetLength(),
                                text_format,
                                &center_lyric_layout,
                                brush_played_text);
                            render_target->PopAxisAlignedClip();

                            accumulated_width += line_widths[i];
                        }
                        else if (accumulated_width < played_total_width)
                        {
                            float remaining = played_total_width - accumulated_width;

                            DWRITE_HIT_TEST_METRICS hit_start{};
                            float x_start, y_start;
                            UNREFERENCED_PARAMETER(
                                text_layout->HitTestTextPosition(char_pos, FALSE, &x_start, &y_start, &hit_start)
                            );
                            float line_start_x = hit_start.left;

                            D2D1_RECT_F line_clip = D2D1::RectF(
                                static_cast<float>(rc.left),
                                current_y,
                                static_cast<float>(rc.left) + line_start_x + remaining,
                                current_y + line_height
                            );

                            render_target->PushAxisAlignedClip(line_clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                            render_target->DrawText(
                                lyric_main_text.GetString(),
                                lyric_main_text.GetLength(),
                                text_format,
                                &center_lyric_layout,
                                brush_played_text);
                            render_target->PopAxisAlignedClip();

                            break;
                        }

                        current_y += line_height;
                        char_pos += line_metrics[i].length;
                    }
                }
            }
        }
        else
        {
            render_target->DrawText(
                lyric_main_text.GetString(),
                lyric_main_text.GetLength(),
                text_format,
                &center_lyric_layout,
                brush_played_text);
        }

        float lyric_aux_metrics_width, lyric_aux_metrics_height;
        if ((IsTranslationEnabled() && lrc_controller.is_auxiliary_info_enabled(LrcAuxiliaryInfo::Translation))
            || (IsRomanizationEnabled() && lrc_controller.is_auxiliary_info_enabled(LrcAuxiliaryInfo::Romanization)))
        {
            LrcAuxiliaryInfo lrc_aux_info = LrcAuxiliaryInfo::Ignored;
            if (IsTranslationEnabled() && lrc_controller.get_current_lrc_line_aux_index(LrcAuxiliaryInfo::Translation)
                != -1)
                lrc_aux_info = LrcAuxiliaryInfo::Translation;
            else if (IsRomanizationEnabled() && lrc_controller.get_current_lrc_line_aux_index(
                LrcAuxiliaryInfo::Romanization) != -1)
                lrc_aux_info = LrcAuxiliaryInfo::Romanization;
            if (lrc_aux_info != LrcAuxiliaryInfo::Ignored)
            {
                int lrc_aux_index = -1;
                UNREFERENCED_PARAMETER(lrc_aux_index);
                CString lrc_aux_text;
                switch (lrc_aux_info)
                {
                case LrcAuxiliaryInfo::Translation: lrc_aux_index = lrc_controller.get_current_lrc_line_aux_index(
                        LrcAuxiliaryInfo::Translation);
                    break;
                case LrcAuxiliaryInfo::Romanization: lrc_aux_index = lrc_controller.get_current_lrc_line_aux_index(
                        LrcAuxiliaryInfo::Romanization);
                    break;
                default: assert(false);
                }

                lrc_controller.get_current_lrc_line_at(lrc_aux_index, lrc_aux_text);
                MeasureTextMetrics(lrc_aux_text, rc.right - rc.left, &lyric_aux_metrics_width,
                                   &lyric_aux_metrics_height, LrcAuxiliaryInfo::Translation);

                D2D1_RECT_F translation_layout = D2D1::RectF(rc.left, center_y + lyric_main_metrics_height, rc.right,
                                                             center_y + lyric_main_metrics_height +
                                                             lyric_aux_metrics_height);
                render_target->DrawText(
                    lrc_aux_text.GetString(),
                    lrc_aux_text.GetLength(),
                    text_format_translation,
                    &translation_layout,
                    percentage_enabled ? brush_unplay_text : brush_played_text);
            }
        }

        float lyric_prev_metrics_width, lyric_prev_metrics_height;
        float lyric_next_metrics_width, lyric_next_metrics_height;
        int lrc_prev_node_index = lrc_controller.get_current_lrc_node_index() - 1;
        int lrc_next_node_index = lrc_controller.get_current_lrc_node_index() + 1;

        if (lrc_next_node_index < lrc_controller.get_lrc_node_count())
        {
            CString lyric_next_text;
            lrc_controller.get_lrc_line_at(lrc_next_node_index,
                                           lrc_controller.get_lrc_line_aux_index(
                                               lrc_next_node_index, LrcAuxiliaryInfo::Lyric),
                                           lyric_next_text);
            MeasureTextMetrics(lyric_next_text, rc.right - rc.left, &lyric_next_metrics_width,
                               &lyric_next_metrics_height);
            D2D1_RECT_F next_layout = D2D1::RectF(rc.left, center_y + lyric_main_metrics_height + 30, rc.right,
                                                  center_y + lyric_main_metrics_height + lyric_next_metrics_height +
                                                  30);
            if ((IsTranslationEnabled() && lrc_controller.get_current_lrc_line_aux_index(LrcAuxiliaryInfo::Translation) != -1)
                || IsRomanizationEnabled() && lrc_controller.get_current_lrc_line_aux_index(LrcAuxiliaryInfo::Romanization) != -1)
            {
                next_layout.top += lyric_aux_metrics_height - 20;
                next_layout.bottom += lyric_aux_metrics_height - 20;
            }
            render_target->DrawText(
                lyric_next_text.GetString(),
                lyric_next_text.GetLength(),
                text_format,
                &next_layout,
                brush_unplay_text
            );
        }
        // prev&next
        float aux_prev_metrics_width, aux_prev_metrics_height;
        if (lrc_prev_node_index >= 0)
        {
            LrcAuxiliaryInfo prev_aux_info = LrcAuxiliaryInfo::Ignored;
            if (IsTranslationEnabled() && lrc_controller.get_lrc_line_aux_index(lrc_prev_node_index, LrcAuxiliaryInfo::Translation) != -1)
                prev_aux_info = LrcAuxiliaryInfo::Translation;
            else if (IsRomanizationEnabled() && lrc_controller.get_lrc_line_aux_index(lrc_prev_node_index, LrcAuxiliaryInfo::Romanization) != -1)
                prev_aux_info = LrcAuxiliaryInfo::Romanization;
            if (prev_aux_info != LrcAuxiliaryInfo::Ignored) {
                int lrc_aux_prev_index = -1;
                UNREFERENCED_PARAMETER(lrc_aux_prev_index);
                switch (prev_aux_info) {
                    case LrcAuxiliaryInfo::Translation: lrc_aux_prev_index = lrc_controller.get_lrc_line_aux_index(lrc_prev_node_index, LrcAuxiliaryInfo::Translation); break;
                    case LrcAuxiliaryInfo::Romanization: lrc_aux_prev_index = lrc_controller.get_lrc_line_aux_index(lrc_prev_node_index, LrcAuxiliaryInfo::Romanization); break;
                    default:  assert(false);
                }
                CString translation_prev_text;
                lrc_controller.get_lrc_line_at(lrc_prev_node_index, lrc_aux_prev_index, translation_prev_text);
                MeasureTextMetrics(translation_prev_text, rc.right - rc.left, &aux_prev_metrics_width,
                                   &aux_prev_metrics_height, LrcAuxiliaryInfo::Translation);
                D2D1_RECT_F translation_prev_layout_rect = D2D1::RectF(
                    rc.left, center_y - aux_prev_metrics_height - 10, rc.right, center_y - 10);

                render_target->DrawText(
                    translation_prev_text.GetString(),
                    translation_prev_text.GetLength(),
                    text_format_translation,
                    &translation_prev_layout_rect,
                    brush_unplay_text
                );
            }

        }

        if (lrc_prev_node_index >= 0)
        {
            CString lyric_prev_text;
            lrc_controller.get_lrc_line_at(lrc_prev_node_index,
                                           lrc_controller.get_lrc_line_aux_index(
                                               lrc_prev_node_index, LrcAuxiliaryInfo::Lyric),
                                           lyric_prev_text);
            MeasureTextMetrics(lyric_prev_text, rc.right - rc.left, &lyric_prev_metrics_width,
                               &lyric_prev_metrics_height);
            D2D1_RECT_F prev_layout = D2D1::RectF(rc.left, center_y - lyric_prev_metrics_height - 30, rc.right,
                                                  center_y - 30);
            if ((IsTranslationEnabled() && lrc_controller.get_lrc_line_aux_index(lrc_prev_node_index, LrcAuxiliaryInfo::Translation) != -1)
                || (IsRomanizationEnabled() && lrc_controller.get_lrc_line_aux_index(lrc_prev_node_index, LrcAuxiliaryInfo::Romanization) != -1))
            {
                prev_layout.top += 20 - aux_prev_metrics_height;
                prev_layout.bottom += 20 - aux_prev_metrics_height;
            }
            render_target->DrawText(
                lyric_prev_text.GetString(),
                lyric_prev_text.GetLength(),
                text_format,
                &prev_layout,
                brush_unplay_text);
        }

        if (lrc_next_node_index < lrc_controller.get_lrc_node_count())
        {
            LrcAuxiliaryInfo lrc_aux_next_info = LrcAuxiliaryInfo::Ignored;
            if (IsTranslationEnabled() && lrc_controller.get_lrc_line_aux_index(lrc_next_node_index, LrcAuxiliaryInfo::Translation) != -1)
                lrc_aux_next_info = LrcAuxiliaryInfo::Translation;
            else if (IsRomanizationEnabled() && lrc_controller.get_lrc_line_aux_index(lrc_next_node_index, LrcAuxiliaryInfo::Romanization) != -1)
                lrc_aux_next_info = LrcAuxiliaryInfo::Romanization;
            if (lrc_aux_next_info != LrcAuxiliaryInfo::Ignored) {
                int lrc_aux_next_index = lrc_controller.get_lrc_line_aux_index(
                lrc_next_node_index, lrc_aux_next_info);

                CString translation_next_text;
                lrc_controller.get_lrc_line_at(lrc_next_node_index, lrc_aux_next_index, translation_next_text);
                float translation_next_metrics_width, translation_next_metrics_height;
                MeasureTextMetrics(translation_next_text, rc.right - rc.left, &translation_next_metrics_width,
                                   &translation_next_metrics_height, LrcAuxiliaryInfo::Translation);
                D2D1_RECT_F translation_next_layout_rect = D2D1::RectF(
                    rc.left,
                    center_y + lyric_main_metrics_height + lyric_next_metrics_height + 30, rc.right,
                    center_y + lyric_main_metrics_height + translation_next_metrics_height + lyric_next_metrics_height + 30);
                if ((IsTranslationEnabled() && lrc_controller.get_current_lrc_line_aux_index(LrcAuxiliaryInfo::Translation) != -1)
                    || (IsRomanizationEnabled() && lrc_controller.get_current_lrc_line_aux_index(LrcAuxiliaryInfo::Romanization) != -1)) {
                    translation_next_layout_rect.top -= 20 - lyric_aux_metrics_height;
                    translation_next_layout_rect.bottom -= 20 - lyric_aux_metrics_height;
                }
                render_target->DrawText(
                    translation_next_text.GetString(),
                    translation_next_text.GetLength(),
                    text_format_translation,
                    &translation_next_layout_rect,
                    brush_unplay_text
                );
            }

        }
    }
    else
    {
        D2D1_RECT_F layoutRect = D2D1::RectF(
            (FLOAT)rc.left, (FLOAT)rc.top,
            (FLOAT)rc.right, (FLOAT)rc.bottom);

        render_target->DrawText(
            _T("暂无歌词"),
            4,
            text_format,
            &layoutRect,
            brush_unplay_text);
    }
}

void CLrcManagerWnd::MeasureTextMetrics(const CString& str, float max_width, float* width_out, float* height_out,
                                        LrcAuxiliaryInfo aux_info)
{
    Microsoft::WRL::ComPtr<IDWriteTextLayout> layout_ptr;
    IDWriteTextFormat* text_format_1 =
        aux_info == LrcAuxiliaryInfo::Lyric ? text_format : text_format_translation;
    if (FAILED(write_factory->CreateTextLayout(str, str.GetLength(), text_format_1, max_width, FLT_MAX, &layout_ptr)))
    {
        ATLTRACE(_T("err: CreateTextLayout failed!\n"));
        return;
    }
    DWRITE_TEXT_METRICS metrics{};
    if (FAILED(layout_ptr->GetMetrics(&metrics)))
    {
        ATLTRACE(_T("err: GetMetrics failed!\n"));
        return;
    }
    *width_out = metrics.widthIncludingTrailingWhitespace;
    *height_out = metrics.height;
}

void CLrcManagerWnd::OnPaint()
{
    CPaintDC dc(this);
    assert(d2d1_factory != nullptr);
    CreateDeviceResources();

    render_target->BeginDraw();
    render_target->Clear(default_dialog_color);

    UpdateLyric();

    if (HRESULT hr = render_target->EndDraw(); hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
    }
}

void CLrcManagerWnd::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (render_target)
    {
        UNREFERENCED_PARAMETER(render_target->Resize(D2D1::SizeU(cx, cy)));
    }
}

void CLrcManagerWnd::ModifyTextColor(bool is_playing, D2D1::ColorF color) {
    if (is_playing) {
        text_played_color = color;
        brush_played_text->SetColor(color);
    } else {
        text_unplayed_color = color;
        brush_unplay_text->SetColor(color);
    }
    Invalidate();
}

void CLrcManagerWnd::ModifyTextFont(bool is_translation, CString font_name) {
    LrcTextCustomization& customization = is_translation ? text_translation_customization : text_customization;
    customization.font_name = font_name;
}

void CLrcManagerWnd::ModifyTextSize(bool is_translation, float font_size) {
    LrcTextCustomization& customization = is_translation ? text_translation_customization : text_customization;
    customization.font_size = font_size;
}

void CLrcManagerWnd::ModifyTextBold(bool is_translation, bool is_bold) {
    LrcTextCustomization& customization = is_translation ? text_translation_customization : text_customization;
    customization.is_bold = is_bold;
}

void CLrcManagerWnd::ModifyTextItalic(bool is_translation, bool is_italic) {
    LrcTextCustomization& customization = is_translation ? text_translation_customization : text_customization;
    customization.is_italic = is_italic;
}

void CLrcManagerWnd::LoadSettingsFromManager(const MusicPlayerSettingsManager &settings_manager) {
    ATLTRACE(_T("info: resume settings from ini\n"));
    ATLTRACE(_T("info: lyric font: %s, size: %d\n"), settings_manager.GetLyricFontName().GetString(), settings_manager.GetLyricFontSize());
    ATLTRACE(_T("info: lyric aux font: %s, size: %d\n"), settings_manager.GetLyricAuxFontName().GetString(), settings_manager.GetLyricAuxFontSize());
    ATLTRACE(_T("info: lyric font bold: %d\n"), settings_manager.IsLyricFontBold());
    ATLTRACE(_T("info: lyric font italic: %d\n"), settings_manager.IsLyricFontItalic());
    ATLTRACE(_T("info: lyric font color: %d\n"), settings_manager.GetLyricFontColor());
    text_customization.font_name = settings_manager.GetLyricFontName();
    text_customization.font_size = settings_manager.GetLyricFontSize();
    text_translation_customization.font_name = settings_manager.GetLyricAuxFontName();
    text_translation_customization.font_size = settings_manager.GetLyricAuxFontSize();
    text_customization.is_bold = settings_manager.IsLyricFontBold();
    text_customization.is_italic = settings_manager.IsLyricFontItalic();
    auto colorrefToD2DColor = [](COLORREF color) {
        return D2D1::ColorF(
            GetRValue(color) / 255.f,
            GetGValue(color) / 255.f,
            GetBValue(color) / 255.f,
            1.f);
    };
    text_played_color = colorrefToD2DColor(settings_manager.GetLyricFontColor());
    text_unplayed_color = colorrefToD2DColor(settings_manager.GetLyricFontColorTranslation());

    ReInitializeDirect2D();
}


void CLrcManagerWnd::CreateDeviceResources()
{
    if (!render_target)
    {
        CRect rc;
        GetClientRect(&rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.Width(), rc.Height());
        // static_cast<int>(static_cast<float>(rc.Width()) * GetSystemDpiScale()),
        // static_cast<int>(static_cast<float>(rc.Height()) * GetSystemDpiScale()));

        UNREFERENCED_PARAMETER(d2d1_factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hWnd, size),
            &render_target));

        if (render_target)
        {
            render_target->SetDpi(96.0f, 96.0f); // process is dpi aware, do not use dip scale
            UNREFERENCED_PARAMETER(
                render_target->CreateSolidColorBrush(text_unplayed_color, &brush_unplay_text));
            UNREFERENCED_PARAMETER(
                render_target->CreateSolidColorBrush(text_played_color, &brush_played_text));
        }
        else
        {
            AfxMessageBox(_T("err: direct2d initialization error!"), MB_ICONERROR);
        }
    }
}

void CLrcManagerWnd::DiscardDirectWrite() {

    if (brush_unplay_text)
    {
        brush_unplay_text->Release();
        brush_unplay_text = nullptr;
    }
    if (brush_played_text)
    {
        brush_played_text->Release();
        brush_played_text = nullptr;
    }
}

void CLrcManagerWnd::ReInitializeDirect2D() {
    DiscardDeviceResources();
    DiscardDirectWrite();
    CreateDeviceResources();
    InitDirectWrite();
    Invalidate();
}

bool CLrcManagerWnd::IsFontNameValid(const CString &font_name) {
    if (font_name.IsEmpty()) return false;
    if (write_factory == nullptr) return false;

    CComPtr<IDWriteFontCollection> font_collection;
    write_factory->GetSystemFontCollection(&font_collection);

    BOOL exists = FALSE;
    UINT32 index;
    font_collection->FindFamilyName(font_name.GetString(), &index, &exists);
    return exists;
}

CString CLrcManagerWnd::GetDirectWriteFontName(LOGFONT* logfont) {
    CComPtr<IDWriteGdiInterop> gdi_interop;
    if (FAILED(write_factory->GetGdiInterop(&gdi_interop)))
        return {};
    CComPtr<IDWriteFont> font;
    if (FAILED(gdi_interop->CreateFontFromLOGFONT(logfont, &font)))
        return {};
    CComPtr<IDWriteFontFamily> family;
    if (FAILED(font->GetFontFamily(&family)))
        return {};

    CComPtr<IDWriteLocalizedStrings> family_names;
    if (FAILED(family->GetFamilyNames(&family_names)))
        return {};

    UINT32 index = 0;
    BOOL exists = FALSE;
    if (FAILED(family_names->FindLocaleName(L"zh-cn", &index, &exists)))
        exists = FALSE;
    if (!exists) index = 0; // fallback

    WCHAR name[LF_FACESIZE];
    UINT32 length = 0;
    if (FAILED(family_names->GetStringLength(index, &length)))
        return {};
    if (FAILED(family_names->GetString(index, name, LF_FACESIZE)))
        return {};
    return name;
}

void CLrcManagerWnd::DiscardDeviceResources()
{
    if (render_target)
    {
        render_target->Release();
        render_target = nullptr;
    }
}

CSimpleArray<CString> SplitLrcForProgressMultiNode2(const CSimpleArray<CString>& texts)
{
    CSimpleArray<CString> strs;
    for (int i = 0; i < texts.GetSize(); ++i)
    {
        const auto& text = texts[i];
        auto new_text = CString();
        bool is_pressed = false;
        for (int j = 0; j < text.GetLength(); ++j)
        {
            if (text[j] == '[' || text[j] == '<')
            {
                is_pressed = true;
                continue;
            }
            if (text[j] == ']' || text[j] == '>')
            {
                is_pressed = false;
            }
            else
            {
                if (is_pressed) continue;
                new_text.AppendChar(text[j]);
            }
        }
        strs.Add(new_text);
    }
    return strs;
}


LrcMultiNode::LrcMultiNode(int t, const CSimpleArray<CString>& texts) :
    LrcAbstractNode(t), str_count(texts.GetSize()), lrc_texts(texts)
{
    for (int i = 0; i < str_count; ++i)
    {
        lang_types.Add(LrcLanguageHelper::detect_language_type(texts[i]));
        aux_infos.Add(LrcAuxiliaryInfo::Lyric);
    }
    int jp_index = lang_types.Find(LrcLanguageHelper::LanguageType::jp), 
        kr_index = lang_types.Find(LrcLanguageHelper::LanguageType::kr),
        eng_index = lang_types.Find(LrcLanguageHelper::LanguageType::en),
        zh_index = lang_types.Find(LrcLanguageHelper::LanguageType::zh);
    if (jp_index != -1)
    {
        aux_infos[jp_index] = LrcAuxiliaryInfo::Lyric;
        int jp_index_2;
        for (jp_index_2 = jp_index + 1; jp_index_2 < str_count; ++jp_index_2) {
            if (lang_types[jp_index_2] == LrcLanguageHelper::LanguageType::jp) {
                break;
            }
        }
        if (jp_index_2 != str_count) {
            // 一般是罗马音置前导致的误判，设置第二个为歌词
            aux_infos[jp_index_2] = LrcAuxiliaryInfo::Lyric;
            aux_infos[jp_index] = LrcAuxiliaryInfo::Ignored;
        }
        if (kr_index != -1)
        {
            ATLTRACE(_T("warn: jp & kr mix, ignoring kr line\n"));
            aux_infos[kr_index] = LrcAuxiliaryInfo::Ignored;
        }
    }
    if (jp_index == -1 && kr_index != -1)
    {
        aux_infos[kr_index] = LrcAuxiliaryInfo::Lyric;
    }


    if (zh_index != -1)
    {
        for (int i = zh_index + 1; i < str_count; ++i)
        {
            if (lang_types[i] == LrcLanguageHelper::LanguageType::zh)
            {
                aux_infos[i] = LrcAuxiliaryInfo::Translation; // 无法判断中文和日文，假定后出现的为翻译
                lang_types[zh_index] = LrcLanguageHelper::LanguageType::jp;
				aux_infos[zh_index] = LrcAuxiliaryInfo::Lyric;
				jp_index = zh_index;
				zh_index = i;
                break;
            }
        }
        if (jp_index != -1 || kr_index != -1 || eng_index != -1 && lang_types[zh_index] == LrcLanguageHelper::LanguageType::zh)
        {
//            ATLTRACE(_T("info: translation hit, line %s\n"), texts[zh_index].GetString());
            aux_infos[zh_index] = LrcAuxiliaryInfo::Translation;
        }
        else
        {
            aux_infos[zh_index] = LrcAuxiliaryInfo::Lyric;
        }
    }

    if (eng_index != -1)
    {
        float eng_prob, romaji_prob;
        LrcLanguageHelper::detect_eng_vs_jpn_romaji_prob(texts[eng_index], &eng_prob, &romaji_prob);
        if (eng_prob > romaji_prob)
        {
            aux_infos[eng_index] = LrcAuxiliaryInfo::Lyric;
        }
        else if (eng_prob < romaji_prob && (jp_index != -1 || kr_index != -1))
        {
//            ATLTRACE(_T("info: romanization hit, line %s\n"), texts[eng_index].GetString());
            aux_infos[eng_index] = LrcAuxiliaryInfo::Romanization;
        }
        else if (jp_index != -1 && kr_index != -1)
        {
            ATLTRACE(_T("warn: unknown romaji, ignoring eng line: %s\n"), texts[eng_index].GetString());
            aux_infos[eng_index] = LrcAuxiliaryInfo::Ignored;
        }
    }
}

void LrcLanguageHelper::detect_eng_vs_jpn_romaji_prob(const CString& input, float* eng_prob, float* jpn_romaji_prob)
{
    CString lower = input;
    lower.MakeLower();
    CStringA str{CT2A(lower)};
    std::initializer_list<CStringA> romaji_syllables = {
        // 五十音图（去掉单元音，因为也同时包含在英语中）
        "ka", "ki", "ku", "ke", "ko", "sa", "shi", "su", "se", "so",
        "ta", "chi", "tsu", "te", "to", "na", "ni", "nu", "ne", "no",
        "ha", "hi", "fu", "he", "ho", "ma", "mi", "mu", "me", "mo",
        "ya", "yu", "yo", "ra", "ri", "ru", "re", "ro", "wa", "wo", "n"
    };
    std::initializer_list<CStringA> english_clusters = {
        // 常见英语辅音组合
        "th", "sh", "ph", "bl", "cl", "str", "spr", "pr", "tr", "dr"
    };

    int romaji_score = 0;
    int english_score = 0;
    float romaji_prob_out, eng_prob_out;

    for (auto& syl : romaji_syllables)
        if (str.Find(syl) != -1) romaji_score++;
    for (auto& cl : english_clusters)
        if (str.Find(cl) != -1) english_score++;

    int vowels = 0, consonants = 0;
    for (int i = 0; i < str.GetLength(); i++)
    {
        if (unsigned char c = str[i]; isalpha(c))
        {
            if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
                vowels++;
            else
                consonants++;
        }
    }
    double vowel_ratio = vowels + consonants > 0 ? (double)vowels / (vowels + consonants) : 0.0;
    if (vowel_ratio > 0.45) romaji_score++;
    else english_score++;

    if (auto total = static_cast<float>(romaji_score + english_score); total == 0)
    { // NOLINT(*-use-auto)
        eng_prob_out = 0.5f;
        romaji_prob_out = 0.5f;
    }
    else
    {
        eng_prob_out = (float)english_score / total;
        romaji_prob_out = (float)romaji_score / total;
    }
    if (fabs(eng_prob_out - romaji_prob_out) < 1e-6) 
    {
        eng_prob_out = 0.f;
        romaji_prob_out = 1.f;
    }
    *eng_prob = eng_prob_out;
    *jpn_romaji_prob = romaji_prob_out;
}

LrcLanguageHelper::LanguageType
LrcLanguageHelper::detect_language_type(const CString& input, float* probability)
{
    int zh = 0, jp = 0, kr = 0, en = 0;
    for (int i = 0; i < input.GetLength(); ++i)
    {
        if (wchar_t ch = input[i]; ch >= 0x4E00 && ch <= 0x9FFF) // zh character
            zh++;
        else if ((ch >= 0x3040 && ch <= 0x309F) || // hiragana
            (ch >= 0x30A0 && ch <= 0x30FF)) // katakana
            jp++;
        else if (ch >= 0xAC00 && ch <= 0xD7AF) // korean
            kr++;
        else if (ch <= 0x007F) // ANSI character, english
            en++;
    }

    float length = static_cast<float>(zh + jp + kr + en); // NOLINT(*-use-auto)
    if (length == 0) return LanguageType::others;

    float zh_score = static_cast<float>(zh); // NOLINT(*-use-auto)
    float jp_score = static_cast<float>(jp) * 2.f + static_cast<float>(zh) * 0.5f; // 日语中包含部分汉字，对假名进行加权
    float kr_score = static_cast<float>(kr); // NOLINT(*-use-auto)
    float en_score = static_cast<float>(en); // NOLINT(*-use-auto)

    auto write_prob = [&input, probability](const CString& out_type, float out_prob)
    {
        if (probability) *probability = out_prob;
//        ATLTRACE(_T("info: line %s, type = %s, max prob = %f\n"),
//                 input.GetString(), out_type.GetString(), out_prob);
    };

    if (zh > 0 && jp > 0)
    {
        write_prob(_T("jp"), jp_score / length);
        return LanguageType::jp;
    }

    if (zh > 0 && en > 0)
    {
        write_prob(_T("zh"), zh_score / length);
        return LanguageType::zh;
    }

	if (kr > 0 && en > 0)
    {
        write_prob(_T("kr"), kr_score / length);
        return LanguageType::kr;
    }

    if (jp > 0 && en > 0) {
        write_prob(_T("jp"), jp_score / length);
        return LanguageType::jp;
    }

    if (zh_score > jp_score && zh_score > en_score && zh_score > kr_score)
    {
        write_prob(_T("zh"), zh_score / length);
        return LanguageType::zh;
    }
    if (jp_score > zh_score && jp_score > en_score && jp_score > kr_score)
    {
        write_prob(_T("jp"), jp_score / length);
        return LanguageType::jp;
    }
    if (en_score > jp_score && en_score > kr_score && en_score > zh_score)
    {
        write_prob(_T("en"), en_score / length);
        return LanguageType::en;
    }
    if (kr_score > zh_score && kr_score > en_score && kr_score > jp_score)
    {
        write_prob(_T("kr"), kr_score / length);
        return LanguageType::kr;
    }
    return LanguageType::others;
}

LrcProgressNode::LrcProgressNode(int t, const CString& text_with_node)
    : LrcAbstractNode(t), node_count(0), end_time_ms(0)
{
    CString text = text_with_node;
    int find_right_brace_info;
    const TCHAR right_brace_type = text[text.GetLength() - 1];
    TCHAR left_brace_type;
    switch (right_brace_type)
    {
        case _T(']'): left_brace_type = _T('['); break;
        case _T('>'): left_brace_type = _T('<'); break;
        default: return;
    }
    do
    {
        find_right_brace_info = text.Find(right_brace_type);
        CString node = text.Left(find_right_brace_info + 1);
        if (node.IsEmpty())
            continue;
        auto node_controller_start_index = node.Find(left_brace_type);
        if (node_controller_start_index == -1)
        {
            ATLTRACE(_T("warn: invalid progress node: %s\n"), node.GetString());
            break;
        }
        auto time_stamp = node.Mid(node_controller_start_index + 1);
        time_stamp.Remove(right_brace_type);
        auto lyric_text = node.Left(node_controller_start_index);
        int minutes = _ttoi(time_stamp.Left(2));
        int seconds = _ttoi(time_stamp.Mid(3, 2));
        CString milliseconds_str = time_stamp.Mid(6);
        int milliseconds = _ttoi(milliseconds_str);
        if (milliseconds_str.GetLength() < 3)
        {
            auto multiples = 3 - milliseconds_str.GetLength();
            milliseconds *= std::floor(pow(10, multiples));
        }
        int total_ms = minutes * 60000 + seconds * 1000 + milliseconds;
        nodes.Add({
            .time_ms = total_ms,
            .node_text = lyric_text
        });
        node_count++;
        text = text.Mid(find_right_brace_info + 1);
    } while (find_right_brace_info != -1);
}

float LrcProgressNode::get_lrc_percentage(float current_timestamp) const
{
    auto base = nodes.GetSize();
    if (base == 0) return 1.0f;

    const int timestamp_in_ms = static_cast<int>(current_timestamp * 1000);
    if (timestamp_in_ms >= nodes[base - 1].time_ms)
    {
        return 1.0f;
    }
    if (timestamp_in_ms < time_ms)
    {
        return 0.0f;
    }

    int index;
    float percentage_in_node = 0.0f, distance = 0.0f, percentage = 0.0f;
    for (index = 0; index < base; ++index)
    {
        if (timestamp_in_ms < nodes[index].time_ms) break;
    }

    if (index != 0)
    {
        distance = static_cast<float>(nodes[index].time_ms - nodes[index - 1].time_ms);
        percentage_in_node = static_cast<float>(timestamp_in_ms - nodes[index - 1].time_ms) / distance;
    }
    else
    {
        // time_ms->start
        distance = static_cast<float>(nodes[0].time_ms - time_ms);
        if (distance > 0)
            percentage_in_node = static_cast<float>(timestamp_in_ms - time_ms) / distance;
        else
            percentage_in_node = 1.0f;
    }

    percentage = static_cast<float>(index) / base + (percentage_in_node / base);
    return percentage;
}

LrcProgressMultiNode::LrcProgressMultiNode
    (int t, const CString& str_1, const CSimpleArray<CString>& str_arr_2):
    LrcAbstractNode(t),
    LrcProgressNode(t, str_1),
    LrcMultiNode(t, SplitLrcForProgressMultiNode2(str_arr_2)) { }

LrcFileController::~LrcFileController()
{
    clear_lrc_nodes();
}

void LrcFileController::parse_lrc_file(const CString& file_path)
{
    clear_lrc_nodes();
    if (CFileStatus file_status;
        file_path.IsEmpty()
        || file_path.Find(_T(".lrc")) == -1
        || !CFile::GetStatus(file_path, file_status))
        return;
    try
    {
        CFile file(file_path, CFile::modeRead | CFile::typeBinary);
        parse_lrc_file_stream(&file);
    }
    catch (const CFileException*& ex)
    {
        CString err_msg;
        LPTSTR err_msg_buf = err_msg.GetBufferSetLength(1024);
        ex->GetErrorMessage(err_msg_buf, 1024);
        err_msg.ReleaseBuffer();
        ATLTRACE(_T("err: err in file open:%s\n"), err_msg.GetString());
    } // nothing happened, LrcFileController remains invalid
}

void LrcFileController::parse_lrc_file_stream(CFile* file_stream)
{
    // 目前仅支持逐行lrc解析
    if (file_stream == nullptr)
    {
        return;
    }
    clear_lrc_nodes();
    CStringA file_content_a;
    const int buf_size = 4096;
    char buffer[buf_size];
    UINT bytes_read = 0;
    do
    {
        bytes_read = file_stream->Read(buffer, buf_size - 1);
        buffer[bytes_read] = '\0';
        file_content_a += buffer;
    }
    while (bytes_read > 0);

    // 转换为宽字符
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, file_content_a, -1, nullptr, 0);
    CString file_content_w;
    MultiByteToWideChar(CP_UTF8, 0, file_content_a, -1, file_content_w.GetBuffer(wide_len), wide_len);
    file_content_w.ReleaseBuffer();
    // 逐行解析
    int start = 0, flag_decoding_metadata = 1;
    std::stack<CString> lyrics_in_ms;
    int recorded_ms = 0;
    bool is_lrc_end = false;

    auto pump_stack = [&](bool is_lrc_ended)
    {
        CSimpleArray<CString> lrc_texts;
        while (!lyrics_in_ms.empty())
        {
            lrc_texts.Add(lyrics_in_ms.top());
            lyrics_in_ms.pop();
        }
        if (lrc_texts.GetSize() > 1)
            std::reverse(lrc_texts.GetData(), lrc_texts.GetData() + lrc_texts.GetSize());
        if (lrc_texts.GetSize() == 0)
            return;
        if (LrcAbstractNode* node = LrcNodeFactory::CreateLrcNode(recorded_ms, lrc_texts))
        {
            if (!lrc_nodes.IsEmpty())
            {
                lrc_nodes[lrc_nodes.GetCount() - 1]->set_lrc_end_timestamp(recorded_ms);
            }
            if (is_lrc_ended)
            {
                node->set_lrc_end_timestamp(std::floor(this->song_duration_sec * 1000));
            }
            lrc_nodes.Add(node);
            if (node->is_translation_enabled())
                this->set_auxiliary_info_enabled(LrcAuxiliaryInfo::Translation);
            if (node->is_romanization_enabled())
                this->set_auxiliary_info_enabled(LrcAuxiliaryInfo::Romanization);
        }
        else
        {
            AfxMessageBox(_T("err: create lrc node failed, aborting!"), MB_ICONERROR);
        }
    };

    while (start < file_content_w.GetLength())
    {
        int end = file_content_w.Find('\n', start);
        if (end == -1)
        {
            end = file_content_w.GetLength();
            is_lrc_end = true;
        }
        CString line = file_content_w.Mid(start, end - start).Trim();
        if (line.IsEmpty())
        {
            start = end + 1;
            continue;
        }
        if (flag_decoding_metadata)
        {
            // 走metadata解析，不遵守标准lrc解码
            switch (get_metadata_type(line))
            {
            case LrcMetadataType::Artist:
                metadata.artist = get_metadata_value(line);
                break;
            case LrcMetadataType::Album:
                metadata.album = get_metadata_value(line);
                break;
            case LrcMetadataType::Title:
                metadata.title = get_metadata_value(line);
                break;
            case LrcMetadataType::By:
                metadata.by = get_metadata_value(line);
                break;
            case LrcMetadataType::Offset:
                lrc_offset_ms = _ttoi(get_metadata_value(line));
                break;
            case LrcMetadataType::Author:
                metadata.author = get_metadata_value(line);
                break;
            case LrcMetadataType::Ignored:
                break;
            case LrcMetadataType::Error: default:
                flag_decoding_metadata = 0;
                break;
            }
            if (flag_decoding_metadata)
            {
                start = end + 1;
                continue;
            }
        }
        // 解析时间tag
        if (line.GetLength() < 10)
        {
            AfxMessageBox(_T("err: invalid lrc line, aborting!"), MB_ICONERROR);
            // clear stack
            while (!lyrics_in_ms.empty())
            {
                delete lyrics_in_ms.top();
                lyrics_in_ms.pop();
            }
            clear_lrc_nodes();
            return;
        }
        int time_tag_end_index = line.Find(']');
        if (time_tag_end_index == -1 || line[0] != '[' || line[3] != ':' || line[6] != '.')
        {
            AfxMessageBox(_T("err: invalid lrc time tag, aborting!"), MB_ICONERROR);
            while (!lyrics_in_ms.empty())
            {
                delete lyrics_in_ms.top();
                lyrics_in_ms.pop();
            }
            clear_lrc_nodes();
            return;
        }
        int minutes = _ttoi(line.Mid(1, 2));
        int seconds = _ttoi(line.Mid(4, 2));
        CString milliseconds_str = line.Mid(7, time_tag_end_index - 7);
        int milliseconds = _ttoi(milliseconds_str);
        if (milliseconds_str.GetLength() < 3)
        {
            auto multiples = 3 - milliseconds_str.GetLength();
            milliseconds *= std::floor(pow(10, multiples));
        }

        switch (int total_ms = minutes * 60000 + seconds * 1000 + milliseconds + lrc_offset_ms; WAY3RES(
            total_ms <=> recorded_ms))
        {
        case ThreeWayCompareResult::Less: // 歌词时间戳一定有序
            AfxMessageBox(_T("err: invalid time stamp order!"), MB_ICONERROR);
            while (!lyrics_in_ms.empty())
            {
                delete lyrics_in_ms.top();
                lyrics_in_ms.pop();
            }
            clear_lrc_nodes();
            break;
        case ThreeWayCompareResult::Greater:
            // 先处理之前的歌词
            if (total_ms < 0) total_ms = 0;
            if (!lyrics_in_ms.empty())
                pump_stack(is_lrc_end);
            recorded_ms = total_ms;
            break;
        default:
            break;
        }
        lyrics_in_ms.push(line.Mid(time_tag_end_index + 1).Trim());
        start = end + 1;
    }
    pump_stack(is_lrc_end);
    cur_lrc_node_index = 0;
}

void LrcFileController::clear_lrc_nodes()
{
    for (size_t i = 0; i < lrc_nodes.GetCount(); i++)
    {
        delete lrc_nodes[i];
    }
    lrc_nodes.RemoveAll();
}

void LrcFileController::set_time_stamp(int time_stamp_ms_in)
{
    if (time_stamp_ms_in < time_stamp_ms)
    {
        // reverse query, set index to zero
        cur_lrc_node_index = 0;
    }
    bool found = false;
    for (size_t i = cur_lrc_node_index; i < lrc_nodes.GetCount(); i++)
    {
        if (lrc_nodes[i]->get_time_ms() > time_stamp_ms_in)
        {
            cur_lrc_node_index = i == 0 ? 0 : i - 1;
            found = true;
            break;
        }
    }
    if (!found)
    {
        cur_lrc_node_index = lrc_nodes.GetCount() - 1;
    }
    time_stamp_ms = time_stamp_ms_in;
}

void LrcFileController::time_stamp_increase(int ms)
{
    time_stamp_ms += ms;
    set_time_stamp(time_stamp_ms);
}

bool LrcFileController::valid() const
{
    return lrc_nodes.GetCount() > 0 && song_duration_sec >= 0;
}

int LrcFileController::get_current_lrc_lines_count() const
{
    return lrc_nodes[cur_lrc_node_index]->get_lrc_str_count();
}

int LrcFileController::get_current_lrc_line_at(int index, CString& out_str) const
{
    return lrc_nodes[cur_lrc_node_index]->get_lrc_str_at(index, out_str);
}

int LrcFileController::get_lrc_line_at(int lrc_node_index, int index, CString& out_str) const
{
    assert(
        lrc_node_index >= 0 && lrc_node_index < lrc_nodes.GetCount() && index >= 0 && index < lrc_nodes[lrc_node_index]
        ->get_lrc_str_count());
    return lrc_nodes[lrc_node_index]->get_lrc_str_at(index, out_str);
}

int LrcFileController::get_current_lrc_line_aux_index(LrcAuxiliaryInfo info) const
{
    return lrc_nodes[cur_lrc_node_index]->get_auxiliary_info_at(info);
}

int LrcFileController::get_lrc_line_aux_index(int lrc_node_index, LrcAuxiliaryInfo info) const
{
    assert(lrc_node_index >= 0 && lrc_node_index < lrc_nodes.GetCount());
    return lrc_nodes[lrc_node_index]->get_auxiliary_info_at(info);
}

LrcMetadataType LrcFileController::get_metadata_type(const CString& str)
{
    if (str.IsEmpty() || str.GetLength() < 3 || str[0] != '[')
    {
        return LrcMetadataType::Error;
    }
    // 逐字歌词有可能每个单位后都带有时间戳
    if (str.Find(']') != str.GetLength() - 1)
        return LrcMetadataType::Error;
    int metadata_end_index = str.Find(':', 1);
    if (metadata_end_index == -1)
        return LrcMetadataType::Error;

    switch (CString metadata_type_str = str.Left(metadata_end_index).Mid(1);
        cstring_hash_fnv_64bit_int(metadata_type_str))
    {
    case 0x645d220c: return LrcMetadataType::Artist;
    case 0x63d58dce: return LrcMetadataType::Album;
    case 0x0387b4f0: return LrcMetadataType::Title;
    case 0x27a9be4e: return LrcMetadataType::By;
    case 0x4f6518ce: return LrcMetadataType::Offset;
    case 0x642cb63f: return LrcMetadataType::Author;
    default: return LrcMetadataType::Ignored;
    }
}

int LrcFileController::cstring_hash_fnv_64bit_int(const CString& str)
{
    const TCHAR* p = str.GetString();
    const int len = str.GetLength();
    unsigned long long h = 14695981039346656037ull; // fnv offset basis
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(p); // NOLINT(*-use-auto)
    const size_t count = static_cast<size_t>(len) * sizeof(TCHAR);
    for (size_t i = 0; i < count; ++i)
    {
        h ^= bytes[i];
        h *= 1099511628211ull; // fnv prime
    }
    return static_cast<int>(h % 0x7fffffffull); // switch-case requires int
}

CString LrcFileController::get_metadata_value(const CString& str)
{
    int metadata_end_index = str.Find(':', 1);
    return str.Mid(metadata_end_index + 1).Trim(']').Trim();
}
