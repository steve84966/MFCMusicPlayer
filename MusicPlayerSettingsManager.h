#pragma once
#include "pch.h"

class MusicPlayerSettingsManager
{
#pragma region Singleton
	public:
	static MusicPlayerSettingsManager& GetInstance()
	{
		static MusicPlayerSettingsManager instance;
		return instance;
	}

	MusicPlayerSettingsManager() = default;
	MusicPlayerSettingsManager(const MusicPlayerSettingsManager&) = delete;
	MusicPlayerSettingsManager& operator=(const MusicPlayerSettingsManager&) = delete;
#pragma endregion

#pragma region LyricSettings
private:
	int lyric_font_size = 20; // default 24pt
	CString lyric_font_name = _T("Microsoft YaHei UI");
	bool lyric_font_bold = false, lyric_font_italic = false;
	int lyric_aux_font_size = 16;
	CString lyric_aux_font_name = _T("Microsoft YaHei UI");
	bool lyric_aux_font_bold = false, lyric_aux_font_italic = false;
	COLORREF lyric_font_color = D2D1::ColorF::Black;
	COLORREF lyric_font_color_translation = D2D1::ColorF::DarkGray;
public:
	[[nodiscard]] int GetLyricFontSize() const { return lyric_font_size; }
	void SetLyricFontSize(int font_size) { lyric_font_size = font_size; }
	[[nodiscard]] COLORREF GetLyricFontColor() const { return lyric_font_color; }
	void SetLyricFontColor(COLORREF font_color) { lyric_font_color = font_color; }
	[[nodiscard]] CString GetLyricFontName() const { return lyric_font_name; }
	void SetLyricFontName(const CString& font_name) { lyric_font_name = font_name; }
	[[nodiscard]] bool IsLyricFontBold() const { return lyric_font_bold; }
	void SetLyricFontBold(bool is_bold) { lyric_font_bold = is_bold; }
	[[nodiscard]] bool IsLyricFontItalic() const { return lyric_font_italic; }
	void SetLyricFontItalic(bool is_italic) { lyric_font_italic = is_italic; }
	[[nodiscard]] int GetLyricAuxFontSize() const { return lyric_aux_font_size; }
	void SetLyricAuxFontSize(int font_size) { lyric_aux_font_size = font_size; }
	[[nodiscard]] CString GetLyricAuxFontName() const { return lyric_aux_font_name; }
	void SetLyricAuxFontName(const CString& font_name) { lyric_aux_font_name = font_name; }
	[[nodiscard]] COLORREF GetLyricFontColorTranslation() const { return lyric_font_color_translation; }
	void SetLyricFontColorTranslation(COLORREF font_color_translation) { lyric_font_color_translation = font_color_translation; }
#pragma endregion

#pragma region SerializeToINI
	public:
	void LoadSettingsFromINI(const CString& ini_filepath)
	{
		// judge if ini exists
		if (_taccess(ini_filepath, 0) == -1)
			return; // file not exist, use default settings
		lyric_font_size = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontSize"), 24, ini_filepath);
		lyric_font_color = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontColor"), static_cast<int>(D2D1::ColorF::Black), ini_filepath);
		lyric_font_color_translation = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontColorTranslation"), static_cast<int>(D2D1::ColorF::DarkGray), ini_filepath);
		LPTSTR buffer = lyric_font_name.GetBuffer(MAX_PATH);
		GetPrivateProfileString(_T("LyricSettings"), _T("LyricFontName"), _T("Microsoft YaHei UI"), buffer, 0, ini_filepath);
		lyric_font_name.ReleaseBuffer();
		lyric_font_bold = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontBold"), false, ini_filepath);
		lyric_font_italic = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontItalic"), false, ini_filepath);
		lyric_aux_font_bold = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricAuxFontBold"), false, ini_filepath);
		lyric_aux_font_italic = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricAuxFontItalic"), false, ini_filepath);
		buffer = lyric_aux_font_name.GetBuffer(MAX_PATH);
		GetPrivateProfileString(_T("LyricSettings"), _T("LyricAuxFontName"), _T("Microsoft YaHei UI"), buffer, 0, ini_filepath);
		lyric_aux_font_name.ReleaseBuffer();
	}
	void SaveSettingsToINI(const CString& ini_filepath) const
	{
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontSize"), std::to_wstring(lyric_font_size).c_str(), ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontColor"), std::to_wstring(static_cast<int>(lyric_font_color)).c_str(), ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontColorTranslation"), std::to_wstring(static_cast<int>(lyric_font_color_translation)).c_str(), ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontName"), lyric_font_name, ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontBold"), std::to_wstring(lyric_font_bold).c_str(), ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontItalic"), std::to_wstring(lyric_font_italic).c_str(), ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricAuxFontName"), lyric_aux_font_name, ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricAuxFontBold"), std::to_wstring(lyric_aux_font_bold).c_str(), ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricAuxFontItalic"), std::to_wstring(lyric_aux_font_italic).c_str(), ini_filepath);
	}
#pragma endregion
};

