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
private:
	MusicPlayerSettingsManager() = default;
	MusicPlayerSettingsManager(const MusicPlayerSettingsManager&) = delete;
	MusicPlayerSettingsManager& operator=(const MusicPlayerSettingsManager&) = delete;
#pragma endregion

#pragma region LyricSettings
private:
	int lyric_font_size = 24; // default 24pt
	COLORREF lyric_font_color = D2D1::ColorF::Black; // default white
	COLORREF lyric_font_color_translation = D2D1::ColorF::DarkGray; // default darkgray
public:
	int GetLyricFontSize() const { return lyric_font_size; }
	void SetLyricFontSize(int font_size) { lyric_font_size = font_size; }
	COLORREF GetLyricFontColor() const { return lyric_font_color; }
	void SetLyricFontColor(COLORREF font_color) { lyric_font_color = font_color; }
	COLORREF GetLyricFontColorTranslation() const { return lyric_font_color_translation; }
	void SetLyricFontColorTranslation(COLORREF font_color_translation) { lyric_font_color_translation = font_color_translation; }
#pragma endregion

#pragma region SerializeToINI
	public:
	void LoadSettingsFromINI(const CString& ini_filepath)
	{
		lyric_font_size = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontSize"), 24, ini_filepath);
		lyric_font_color = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontColor"), static_cast<int>(D2D1::ColorF::Black), ini_filepath);
		lyric_font_color_translation = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontColorTranslation"), static_cast<int>(D2D1::ColorF::DarkGray), ini_filepath);
	}
	void SaveSettingsToINI(const CString& ini_filepath) const
	{
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontSize"), std::to_wstring(lyric_font_size).c_str(), ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontColor"), std::to_wstring(static_cast<int>(lyric_font_color)).c_str(), ini_filepath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontColorTranslation"), std::to_wstring(static_cast<int>(lyric_font_color_translation)).c_str(), ini_filepath);
	}
#pragma endregion
};

