#pragma once
#include "framework.h"
#include "resource.h"
#include "MusicPlayerSettingsManager.h"


class LrcLanguageHelper
{
public:
	enum class LanguageType
	{
		zh, en, jp, kr, others
	};
	static void detect_eng_vs_jpn_romaji_prob(const CString& input, float* eng_prob, float* jpn_romaji_prob);
	static LanguageType detect_language_type(const CString& input_trimmed, float* probability = nullptr);
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
	[[nodiscard]] virtual float get_lrc_percentage(float current_timestamp) const = 0;
	[[nodiscard]] virtual bool is_lrc_percentage_enabled() const { return false; }
	virtual void set_lrc_end_timestamp(int end_time_ms) { }

	bool operator<(const LrcAbstractNode& other) const {
		return time_ms < other.time_ms;
	}
};

class LrcNode final: public LrcAbstractNode {
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
			return LrcAuxiliaryInfo::Lyric;
		return LrcAuxiliaryInfo::Ignored;
	}

	[[nodiscard]] int get_auxiliary_info_at(LrcAuxiliaryInfo info) const override {
		if (info == LrcAuxiliaryInfo::Lyric) return 0;
		return -1;
	}
	[[nodiscard]] float get_lrc_percentage(float current_timestamp) const override
	{
		return 1.0f;
	}
};

/*
* for display lrc with translation or romanization
*/
class LrcMultiNode : virtual public LrcAbstractNode {
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

	[[nodiscard]] float get_lrc_percentage(float current_timestamp) const override
	{
		return 1.0f;
	}
};

class LrcProgressNode: virtual public LrcAbstractNode
{
protected:
	int node_count;
	struct node_info
	{
		int time_ms;
		CString node_text;
	};
	int end_time_ms;
	CSimpleArray<node_info> nodes;
public:
	LrcProgressNode(int t, const CString& text_with_node);
	[[nodiscard]] int get_lrc_str_count() const override { return 1; }
	int get_lrc_str_at(int index, CString& out_str) const override
	{
		if (index != 0) return -1;
		CString text;
		for (int i = 0; i < node_count; ++i)
		{
			text.Append(nodes[i].node_text);
		}
		return out_str = text, 0;
	}
	[[nodiscard]] LrcAuxiliaryInfo get_auxiliary_info(int index) const override
	{
		if (index == 0)
			return LrcAuxiliaryInfo::Lyric;
		return LrcAuxiliaryInfo::Ignored;
	}
	[[nodiscard]] int get_auxiliary_info_at(LrcAuxiliaryInfo info) const override
	{
		if (info == LrcAuxiliaryInfo::Lyric) return 0;
		return -1;
	}
	[[nodiscard]] float get_lrc_percentage(float current_timestamp) const override;
	[[nodiscard]] bool is_lrc_percentage_enabled() const override { return true; }
	void set_lrc_end_timestamp(int time_ms) override { this->end_time_ms = time_ms; }
};

class LrcProgressMultiNode final:
	public LrcProgressNode, public LrcMultiNode
{
public:
	LrcProgressMultiNode(int t, const CString& str_1, const CSimpleArray<CString>& str_arr_2);
	[[nodiscard]] int get_lrc_str_count() const override
	{
		return LrcMultiNode::get_lrc_str_count();
	}
	int get_lrc_str_at(int index, CString& out_str) const override
	{
		return LrcMultiNode::get_lrc_str_at(index, out_str);
	}
	[[nodiscard]] LrcAuxiliaryInfo get_auxiliary_info(int index) const override
	{
		return LrcMultiNode::get_auxiliary_info(index);
	}
	[[nodiscard]] int get_auxiliary_info_at(LrcAuxiliaryInfo info) const override
	{
		return LrcMultiNode::get_auxiliary_info_at(info);
	}
	[[nodiscard]] float get_lrc_percentage(float current_timestamp) const override
	{
		return LrcProgressNode::get_lrc_percentage(current_timestamp);
	}
	[[nodiscard]] bool is_translation_enabled() const override { return LrcMultiNode::is_translation_enabled(); }
	[[nodiscard]] bool is_romanization_enabled() const override { return LrcMultiNode::is_romanization_enabled(); }
	[[nodiscard]] bool is_lrc_percentage_enabled() const override { return true; }
	void set_lrc_end_timestamp(int time_ms) override { LrcProgressNode::set_lrc_end_timestamp(time_ms); }
};

class LrcNodeFactory {
public:
	static LrcAbstractNode* CreateLrcNode(int time_ms, const CSimpleArray<CString>& lrc_texts) {
		auto ifLrcContainsControllerNode = [](const CString& lrc_text)
		{
			const auto last_index = lrc_text.GetLength() - 1;
			return lrc_text.GetLength() > 0 &&
				(lrc_text[last_index] == ']' || lrc_text[last_index] == '>');
		};
		if (lrc_texts.GetSize() == 1) {
			if (ifLrcContainsControllerNode(lrc_texts[0]))
				return new LrcProgressNode(time_ms, lrc_texts[0]);
			return new LrcNode(time_ms, lrc_texts[0]);
		}
		if (lrc_texts.GetSize() > 1) {
			if (ifLrcContainsControllerNode(lrc_texts[0]))
				return new LrcProgressMultiNode(time_ms, lrc_texts[0], lrc_texts);
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
	float song_duration_sec = 0;
public:
	~LrcFileController();
	void parse_lrc_file(const CString& file_path);
	void parse_lrc_file_stream(CFile* file_stream);
	void clear_lrc_nodes();
	void set_time_stamp(int time_stamp_ms_in);
	void time_stamp_increase(int ms);
	void set_song_duration(float duration_sec) { song_duration_sec = duration_sec; }
	[[nodiscard]] bool valid() const;
	[[nodiscard]] int get_current_time_stamp() const { return time_stamp_ms; }
	[[nodiscard]] int get_current_lrc_lines_count() const;
	[[nodiscard]] int get_current_lrc_node_index() const { return static_cast<int>(cur_lrc_node_index); }
	[[nodiscard]] int get_lrc_node_count() const { return static_cast<int>(lrc_nodes.GetCount()); }
	[[nodiscard]] int get_lrc_node_time_ms(int index) const { assert(index < lrc_nodes.GetCount());  return lrc_nodes[index]->get_time_ms(); }
	int get_current_lrc_line_at(int index, CString& out_str) const;
	int get_lrc_line_at(int lrc_node_index, int index, CString& out_str) const;
	[[nodiscard]] int get_current_lrc_line_aux_index(LrcAuxiliaryInfo info) const;
	[[nodiscard]] int get_lrc_line_aux_index(int lrc_node_index, LrcAuxiliaryInfo info) const;

	[[nodiscard]] int is_auxiliary_info_enabled(LrcAuxiliaryInfo enable_info) const
	{
		return (aux_enable_info & (1 << static_cast<int>(enable_info))) != 0;
	}
	void set_auxiliary_info_enabled(LrcAuxiliaryInfo enable_info)
	{
		aux_enable_info |= (1 << static_cast<int>(enable_info));
	}
	void clear_auxiliary_info_enabled(LrcAuxiliaryInfo enable_info)
	{
		aux_enable_info &= ~(1 << static_cast<int>(enable_info));
	}
	void reset_auxiliary_info_enabled() { aux_enable_info = 0; }
	bool is_percentage_enabled(int index) { return lrc_nodes[index]->is_lrc_percentage_enabled(); }
	float get_lrc_percentage(int index) { return lrc_nodes[index]->get_lrc_percentage(time_stamp_ms / 1000.0f); }

	// static helpers
	static LrcMetadataType get_metadata_type(const CString& str);
	static int cstring_hash_fnv_64bit_int(const CString& str);
	static CString get_metadata_value(const CString& str);
};

class CLrcManagerWnd : public CWnd
{

protected:
	struct LrcTextCustomization {
		float font_size;
		CString font_name;
		bool is_bold;
		bool is_italic;
	};
	LrcTextCustomization text_customization {
		.font_size = 20, .font_name = _T("Microsoft YaHei"), .is_bold = false, .is_italic = false
	};
	LrcTextCustomization text_translation_customization {
		.font_size = 16, .font_name = _T("Microsoft YaHei"), .is_bold = false, .is_italic = false
	};
	D2D1::ColorF text_played_color = D2D1::ColorF::Black, text_unplayed_color = D2D1::ColorF::DarkGray;

public:
	CLrcManagerWnd();
	~CLrcManagerWnd() override;
	afx_msg LRESULT OnPlayerTimeChange(WPARAM wParam, LPARAM lParam);

	void CreateDeviceResources();
	void DiscardDeviceResources();
	int InitDirectWrite();
	void DiscardDirectWrite();
	void ReInitializeDirect2D();

	bool IsFontNameValid(const CString& font_name);

	CString GetDirectWriteFontName(LOGFONT *logfont);

	int InitLrcControllerWithFile(const CString& file_path, float);
	int InitLrcControllerWithStream(const CString& stream, float);
	void DestroyLrcController();
	void UpdateLyric();

	[[nodiscard]] int GetCurrentLrcNodeIndex() const { return lrc_controller.get_current_lrc_node_index(); }
	[[nodiscard]] int GetLrcNodeCount() const { return lrc_controller.get_lrc_node_count(); }
	[[nodiscard]] bool IsValid() const { return lrc_controller.valid(); }
	[[nodiscard]] int GetLrcNodeTimeStamp(int index) { return lrc_controller.get_lrc_node_time_ms(index); }

	void SetTranslationEnabled(bool enable) {
		enable_translation = enable;
		if (enable_translation && enable_romanization) {
			enable_romanization = false;
		}
		Invalidate(FALSE);
	}
	[[nodiscard]] bool IsTranslationEnabled() const { return enable_translation; }
	void SetRomanizationEnabled(bool enable) {
		enable_romanization = enable;
		if (enable_romanization && enable_translation) {
			enable_romanization = false;
		}
		Invalidate(FALSE);
	}
	[[nodiscard]] bool IsRomanizationEnabled() const { return enable_romanization; }
	[[nodiscard]] bool IsAuxiliaryInfoEnabled(LrcAuxiliaryInfo info) const { return lrc_controller.is_auxiliary_info_enabled(info); }

	void MeasureTextMetrics(const CString& str, float max_width, float* width_out, float* height_out, LrcAuxiliaryInfo aux_info = LrcAuxiliaryInfo::Lyric);
	// note: passing static control via SubclassDlgItem, no OnCreate call
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	void ModifyTextColor(bool is_playing, D2D1::ColorF color);
	void ModifyTextFont(bool is_translation, CString font_name);
	void ModifyTextSize(bool is_translation, float font_size);
	D2D1::ColorF GetTextColor(bool is_playing) const { return is_playing ? text_played_color : text_unplayed_color; }
	CString GetTextFont(bool is_translation) const { return is_translation ? text_translation_customization.font_name : text_customization.font_name; }
	float GetTextSize(bool is_translation) const { return is_translation ? text_translation_customization.font_size : text_customization.font_size; }

	void ModifyTextBold(bool is_translation, bool is_bold);
	void ModifyTextItalic(bool is_translation, bool is_italic);
	bool IsTextBold(bool is_translation) const { return is_translation ? text_translation_customization.is_bold : text_customization.is_bold; }
	bool IsTextItalic(bool is_translation) const { return is_translation ? text_translation_customization.is_italic : text_customization.is_italic; }
	void LoadSettingsFromManager(const MusicPlayerSettingsManager& settings_manager);

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
	bool enable_translation = false;
	bool enable_romanization = false;

	DECLARE_DYNAMIC(CLrcManagerWnd)
	DECLARE_MESSAGE_MAP()
};