#include "pch.h"
#include "LrcManagerWnd.h"

LRESULT CLrcManagerWnd::OnPlayerTimeChange(WPARAM wParam, LPARAM lParam) { // NOLINT(*-convert-member-functions-to-static) 服了clang-tidy又xjb乱报
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    UINT32 raw = static_cast<UINT32>(wParam); // NOLINT(*-use-auto)
    float time = *reinterpret_cast<float*>(&raw);

    int time_ms = static_cast<int>(time * 1000.0f);
    lrc_controller.set_time_stamp(time_ms);
	this->Invalidate(FALSE);
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
    write_factory(nullptr), text_format(nullptr)
{
    COLORREF cr = ::GetSysColor(COLOR_BTNFACE);
    FLOAT r = GetRValue(cr) / 255.0f;
    FLOAT g = GetGValue(cr) / 255.0f;
    FLOAT b = GetBValue(cr) / 255.0f;

	default_dialog_color = D2D1::ColorF(r, g, b);

    InitDirect2D();
}

CLrcManagerWnd::~CLrcManagerWnd() { 
    DiscardDeviceResources();
    if (text_format) text_format->Release();
    if (write_factory) write_factory->Release();
    if (d2d1_factory) d2d1_factory->Release(); 
}

int CLrcManagerWnd::InitDirect2D()
{
    UNREFERENCED_PARAMETER(
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d1_factory)
        );

    UNREFERENCED_PARAMETER(
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&write_factory))
        );

    if (write_factory)
    {
        UNREFERENCED_PARAMETER(
            write_factory->CreateTextFormat(
                _T(""), // TODO: customizable text format
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                20.0f * GetSystemDpiScale(),
                _T("zh-CN"),
                &text_format)
            );
        UNREFERENCED_PARAMETER(
            text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER)
            );
        UNREFERENCED_PARAMETER(
            text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)
            );
    }
	ATLTRACE("info: Lrc Direct2D surface created.\n");
    return 0;
}

int CLrcManagerWnd::InitLrcControllerWithFile(const CString& file_path)
{
    lrc_controller.parse_lrc_file(file_path);
    return lrc_controller.valid();
}

void CLrcManagerWnd::UpdateLyric()
{
    CRect rc;
    GetClientRect(&rc);
    if (lrc_controller.valid() && render_target && brush_unplay_text && brush_played_text && text_format)
    {

        CString lyric_main_text;
        int lrc_text_count = lrc_controller.get_current_lrc_lines_count();
        lrc_controller.get_current_lrc_line_at(0, lyric_main_text);

        D2D1_RECT_F layoutRect = D2D1::RectF(
            (FLOAT)rc.left, (FLOAT)rc.top,
            (FLOAT)rc.right, (FLOAT)rc.bottom);

        render_target->DrawText(
            lyric_main_text.GetString(),
            lyric_main_text.GetLength(),
            text_format,
            &layoutRect,
            brush_unplay_text);

    } else
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
            UNREFERENCED_PARAMETER(render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &brush_unplay_text));
            UNREFERENCED_PARAMETER(render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush_played_text));
        }
        else {
            AfxMessageBox(_T("err: direct2d initialization error!"), MB_ICONERROR);
        }
    }
}

void CLrcManagerWnd::DiscardDeviceResources()
{
    if (brush_unplay_text) { brush_unplay_text->Release(); brush_unplay_text = nullptr; }
    if (brush_played_text) { brush_played_text->Release(); brush_played_text = nullptr; }
    if (render_target) { render_target->Release(); render_target = nullptr; }
}

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
    } catch (const CFileException*& ex)
    {
        CString err_msg;
        LPTSTR err_msg_buf = err_msg.GetBufferSetLength(1024);
        ex->GetErrorMessage(err_msg_buf, 1024);
        err_msg.ReleaseBuffer();
        ATLTRACE(_T("err: err in file open:%s"), err_msg.GetString());
    } // nothing happened, LrcFileController remains invalid
}

void LrcFileController::parse_lrc_file_stream(CFile* file_stream)
{
    // 目前仅支持逐行lrc解析
    if (file_stream == nullptr) {
        return;
    }
    clear_lrc_nodes();
    CStringA file_content_a;
    const int buf_size = 4096;
    char buffer[buf_size];
    UINT bytes_read = 0;
    do {
        bytes_read = file_stream->Read(buffer, buf_size - 1);
        buffer[bytes_read] = '\0';
        file_content_a += buffer;
    } while (bytes_read > 0);

    // 转换为宽字符
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, file_content_a, -1, nullptr, 0);
    CString file_content_w;
    MultiByteToWideChar(CP_UTF8, 0, file_content_a, -1, file_content_w.GetBuffer(wide_len), wide_len);
    file_content_w.ReleaseBuffer();
    // 逐行解析
    int start = 0, flag_decoding_metadata = 1;
    std::stack<CString> lyrics_in_ms;
    int recorded_ms = 0;

    auto pump_stack = [&]() {
        CSimpleArray<CString> lrc_texts;
        while (!lyrics_in_ms.empty()) {
            lrc_texts.Add(lyrics_in_ms.top());
            lyrics_in_ms.pop();
        }
        if (lrc_texts.GetSize() > 1)
            std::reverse(lrc_texts.GetData(), lrc_texts.GetData() + lrc_texts.GetSize());
        if (lrc_texts.GetSize() == 0)
			return;
        if (LrcNodeBase* node = LrcNodeFactory::CreateLrcNode(recorded_ms, lrc_texts)) {
            lrc_nodes.Add(node);
        }
        else {
            AfxMessageBox(_T("err: create lrc node failed, aborting!"), MB_ICONERROR);
        }
    };

    while (start < file_content_w.GetLength()) {
        int end = file_content_w.Find('\n', start);
        if (end == -1) {
            end = file_content_w.GetLength();
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
            if (flag_decoding_metadata) {
                start = end + 1;
                continue;
            }
        }
        // 解析时间tag
        if (line.GetLength() <= 10) {
            AfxMessageBox(_T("err: invalid lrc line, aborting!"), MB_ICONERROR);
            // clear stack
            while (!lyrics_in_ms.empty()) {
                delete lyrics_in_ms.top();
                lyrics_in_ms.pop();
            }
            clear_lrc_nodes();
            return;
        }
        int time_tag_end_index = line.Find(']');
        if (time_tag_end_index == -1 || line[0] != '[' || line[3] != ':' || line[6] != '.') {
            AfxMessageBox(_T("err: invalid lrc time tag, aborting!"), MB_ICONERROR);
            while (!lyrics_in_ms.empty()) {
                delete lyrics_in_ms.top();
                lyrics_in_ms.pop();
            }
            clear_lrc_nodes();
            return;
        }
        int minutes = _ttoi(line.Mid(1, 2));
        int seconds = _ttoi(line.Mid(4, 2));
        int milliseconds = _ttoi(line.Mid(7, time_tag_end_index - 7));

        switch (int total_ms = minutes * 60000 + seconds * 1000 + milliseconds + lrc_offset_ms; WAY3RES(total_ms <=> recorded_ms))
        {
        case ThreeWayCompareResult::Less: // 歌词时间戳一定有序
            AfxMessageBox(_T("err: invalid time stamp order!"), MB_ICONERROR);
            while (!lyrics_in_ms.empty()) {
                delete lyrics_in_ms.top();
                lyrics_in_ms.pop();
            }
            clear_lrc_nodes();
            break;
        case ThreeWayCompareResult::Greater:
            // 先处理之前的歌词
            if (total_ms < 0) total_ms = 0;
            if (!lyrics_in_ms.empty())
                pump_stack();
            recorded_ms = total_ms;
            break;
        default:
            break;
        }
        lyrics_in_ms.push(line.Mid(time_tag_end_index + 1).Trim());
        start = end + 1;
    }
    pump_stack();
    cur_lrc_node_index = 0;
}

void LrcFileController::clear_lrc_nodes()
{
    for (size_t i = 0; i < lrc_nodes.GetCount(); i++) {
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
    for (size_t i = cur_lrc_node_index; i < lrc_nodes.GetCount(); i++) {
        if (lrc_nodes[i]->get_time_ms() > time_stamp_ms_in) {
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
    return lrc_nodes.GetCount() > 0;
}

int LrcFileController::get_current_lrc_lines_count() const
{
    return lrc_nodes[cur_lrc_node_index]->get_lrc_str_count();
}

int LrcFileController::get_current_lrc_line_at(int index, CString& out_str) const
{
    return lrc_nodes[cur_lrc_node_index]->get_lrc_str_at(index, out_str);
}

LrcMetadataType LrcFileController::get_metadata_type(const CString& str)
{
    if (str.IsEmpty() || str.GetLength() < 3 || str[0] != '[' || str[str.GetLength() - 1] != ']')
    {
        return LrcMetadataType::Error;
    }
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
    default:         return LrcMetadataType::Ignored;
    }
}

int LrcFileController::cstring_hash_fnv_64bit_int(const CString& str)
{
    const TCHAR* p = str.GetString();
    const int len = str.GetLength();
    unsigned long long h = 14695981039346656037ull; // fnv offset basis
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(p); // NOLINT(*-use-auto)
    const size_t count = static_cast<size_t>(len) * sizeof(TCHAR);
    for (size_t i = 0; i < count; ++i) {
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
