#pragma once
#include "framework.h"

class LrcLanguageHelper
{
public:
	enum class LanguageType
	{
		zh, en, jp, kr, others
	};
	static void detect_eng_vs_jpn_romaji_prob(const CString& input, float* eng_prob, float* jpn_romaji_prob);
	static LanguageType detect_language_type(const CString& input, float* probability = nullptr);
};

class LrcAbstractNode {
protected:
	int time_ms;            // time in milliseconds
public:
	explicit LrcAbstractNode(int time) : time_ms(time) {}
	virtual ~LrcAbstractNode() = default;
	[[nodiscard]] int get_time_ms() const { return time_ms; }
	[[nodiscard]] virtual int get_lrc_str_count() const = 0;
	virtual int get_lrc_str_at(int index, CString& out_str) const = 0;
	[[nodiscard]] virtual LrcAuxiliaryInfo get_auxiliary_info(int index) const = 0;
	[[nodiscard]] virtual int get_auxiliary_info_at(LrcAuxiliaryInfo info) const = 0;
	[[nodiscard]] virtual bool is_translation_enabled() const { return false; }
	[[nodiscard]] virtual bool is_romanization_enabled() const { return false; }

	bool operator<(const LrcAbstractNode& other) const {
		return time_ms < other.time_ms;
	}
};

class LrcNode : public LrcAbstractNode {
	CString lrc_text;       // lyric text
public:
	LrcNode(int t, const CString& text)
		: LrcAbstractNode(t), lrc_text(text) {
	}

	[[nodiscard]] int get_lrc_str_count() const override {
		return 1;
	}

	int get_lrc_str_at(int index, CString& out_str) const override {
		if (index != 0) return -1;
		return out_str = lrc_text, 0;
	}

	[[nodiscard]] LrcAuxiliaryInfo get_auxiliary_info(int index) const override {
		if (index == 0)
			return LrcAuxiliaryInfo::None;
		return LrcAuxiliaryInfo::Ignored;
	}

	[[nodiscard]] int get_auxiliary_info_at(LrcAuxiliaryInfo info) const override {
		if (info == LrcAuxiliaryInfo::None) return 0;
		return -1;
	}
};

/*
* for display lrc with translation or romanization
*/
class LrcMultiNode : public LrcAbstractNode {
	int str_count;
	CSimpleArray<CString> lrc_texts;
	CSimpleArray<LrcAuxiliaryInfo> aux_infos;
	CSimpleArray<LrcLanguageHelper::LanguageType> lang_types;

public:

	LrcMultiNode(int t, const CSimpleArray<CString>& texts);

	[[nodiscard]] int get_lrc_str_count() const override {
		return str_count;
	}

	int get_lrc_str_at(int index, CString& out_str) const override {
		if (index < 0 || index >= str_count) return -1;
		out_str = lrc_texts[index];
		return 0;
	}

	[[nodiscard]] LrcAuxiliaryInfo get_auxiliary_info(int index) const override
	{
		return aux_infos[index];
	}

	[[nodiscard]] int get_auxiliary_info_at(LrcAuxiliaryInfo info) const override {
		return aux_infos.Find(info);
	}

	[[nodiscard]] bool is_translation_enabled() const override {
		return aux_infos.Find(LrcAuxiliaryInfo::Translation) != -1;
	}

	[[nodiscard]] bool is_romanization_enabled() const override {
		return aux_infos.Find(LrcAuxiliaryInfo::Romanization) != -1;
	}
};

class LrcNodeFactory {
public:
	static LrcAbstractNode* CreateLrcNode(int time_ms, const CSimpleArray<CString>& lrc_texts) {
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

	CAtlArray<LrcAbstractNode*> lrc_nodes;
	int time_stamp_ms = 0, lrc_offset_ms = 0;
	size_t cur_lrc_node_index = 0;
	struct
	{
		CString artist, album, author, by, title;
	} metadata;
	int aux_enable_info = 0;
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
	[[nodiscard]] int get_current_lrc_line_index() const { return static_cast<int>(cur_lrc_node_index); }
	[[nodiscard]] int get_lrc_node_count() const { return static_cast<int>(lrc_nodes.GetCount()); }
	int get_current_lrc_line_at(int index, CString& out_str) const;
	[[nodiscard]] int get_current_lrc_line_aux_index(LrcAuxiliaryInfo info) const;

	[[nodiscard]] int is_auxiliary_info_enabled(LrcAuxiliaryInfo enable_info) const
	{
		return (aux_enable_info & (1 << static_cast<int>(enable_info))) != 0;
	}
	void set_auxiliary_info_enabled(LrcAuxiliaryInfo enable_info)
	{
		aux_enable_info |= (1 << static_cast<int>(enable_info));
	}

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

	void MeasureTextMetrics(const CString& str, float max_width, float* width_out, float* height_out, LrcAuxiliaryInfo aux_info = LrcAuxiliaryInfo::None);
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
	IDWriteTextFormat* text_format_translation;
	D2D1::ColorF default_dialog_color{0};

	LrcFileController lrc_controller;

	DECLARE_DYNAMIC(CLrcManagerWnd)
	DECLARE_MESSAGE_MAP()
};