#pragma once
#include "framework.h"

class LrcNodeBase {
protected:
	int time_ms;            // time in milliseconds
public:
	explicit LrcNodeBase(int time) : time_ms(time) {}
	virtual ~LrcNodeBase() = default;
	[[nodiscard]] int get_time_ms() const { return time_ms; }
	[[nodiscard]] virtual int get_lrc_str_count() const = 0;
	virtual int get_lrc_str_at(int index, CString& out_str) const = 0;

	bool operator<(const LrcNodeBase& other) const {
		return time_ms < other.time_ms;
	}
};

class LrcNode : public LrcNodeBase {
	CString lrc_text;       // lyric text
public:
	LrcNode(int t, const CString& text)
		: LrcNodeBase(t), lrc_text(text) {
	}

	[[nodiscard]] int get_lrc_str_count() const override {
		return 1;
	}

	int get_lrc_str_at(int index, CString& out_str) const override {
		if (index != 0) return -1;
		return out_str = lrc_text, 0;
	}
};

/*
* for display lrc with translation or romanization
*/
class LrcMultiNode : public LrcNodeBase {
	int str_count;
	CSimpleArray<CString> lrc_texts;

	[[nodiscard]] int get_lrc_str_count() const override {
		return str_count;
	}

	int get_lrc_str_at(int index, CString& out_str) const override {
		if (index < 0 || index >= str_count) return -1;
		out_str = lrc_texts[index];
		return 0;
	}

public:
	LrcMultiNode(int t, const CSimpleArray<CString>& texts)
		: LrcNodeBase(t), str_count(texts.GetSize()), lrc_texts(texts) {
	}
};

class LrcNodeFactory {
public:
	static LrcNodeBase* CreateLrcNode(int time_ms, const CSimpleArray<CString>& lrc_texts) {
		if (lrc_texts.GetSize() == 1) {
			return new LrcNode(time_ms, lrc_texts[0]);
		}
		if (lrc_texts.GetSize() > 1) {
			return new LrcMultiNode(time_ms, lrc_texts);
		}
		return nullptr;
	}
};

/*
* internal helper of CLrcManagerWnd, perform lyric management
*/
class LrcFileController {
	friend class CLrcManagerWnd;

	CAtlArray<LrcNodeBase*> lrc_nodes;
	int time_stamp_ms = 0, lrc_offset_ms = 0;
	size_t cur_lrc_node_index = 0;
	struct
	{
		CString artist, album, author, by, title;
	} metadata;
public:
	~LrcFileController();
	void parse_lrc_file(const CString& file_path);
	void parse_lrc_file_stream(CFile* file_stream);
	void clear_lrc_nodes();
	void set_time_stamp(int time_stamp_ms_in);
	void time_stamp_increase(int ms);
	[[nodiscard]] bool valid() const;
	[[nodiscard]] int get_current_time_stamp() const { return time_stamp_ms; }
	[[nodiscard]] int get_current_lrc_lines_count() const;
	int get_current_lrc_line_at(int index, CString& out_str) const;

	// static helpers
	static LrcMetadataType get_metadata_type(const CString& str);
	static int cstring_hash_fnv_64bit_int(const CString& str);
	static CString get_metadata_value(const CString& str);
};

class CLrcManagerWnd : public CWnd
{

protected:

public:
	CLrcManagerWnd();
	~CLrcManagerWnd() override;
	afx_msg LRESULT OnPlayerTimeChange(WPARAM wParam, LPARAM lParam);

	void CreateDeviceResources();
	void DiscardDeviceResources();

	int InitDirect2D();
	int InitLrcControllerWithFile(const CString& file_path);
	void UpdateLyric();
	// note: passing static control via SubclassDlgItem, no OnCreate call
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);

protected:
	ID2D1Factory* d2d1_factory;
	ID2D1HwndRenderTarget* render_target;
	ID2D1SolidColorBrush* brush_unplay_text;
	ID2D1SolidColorBrush* brush_played_text;
	IDWriteFactory* write_factory;
	IDWriteTextFormat* text_format;
	D2D1::ColorF default_dialog_color{0};

	LrcFileController lrc_controller;

	DECLARE_DYNAMIC(CLrcManagerWnd)
	DECLARE_MESSAGE_MAP()
};