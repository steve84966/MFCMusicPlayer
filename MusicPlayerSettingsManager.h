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

	// eq settings
	CSimpleArray<int> eq_bands{};
	int eq_preset_id = 0;
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
	[[nodiscard]] bool IsLyricAuxFontBold() const { return lyric_aux_font_bold; }
	void SetLyricAuxFontBold(bool is_bold) { lyric_aux_font_bold = is_bold; }
	[[nodiscard]] bool IsLyricAuxFontItalic() const { return lyric_aux_font_italic; }
	void SetLyricAuxFontItalic(bool is_italic) { lyric_aux_font_italic = is_italic; }
	[[nodiscard]] COLORREF GetLyricFontColorTranslation() const { return lyric_font_color_translation; }
	void SetLyricFontColorTranslation(COLORREF font_color_translation) { lyric_font_color_translation = font_color_translation; }
	[[nodiscard]] int GetEqPresetID() const { return eq_preset_id; }
	void SetEqPresetID(int preset_id) { eq_preset_id = preset_id; }
	[[nodiscard]] const CSimpleArray<int>& GetEqBands() const { return eq_bands; }
	void SetEqBands(const CSimpleArray<int>& bands) { eq_bands = bands; }
#pragma endregion

#pragma region SerializeToINI
	constexpr static TCHAR ini_filepath[] = _T(".\\settings.ini");
	public:
	static CString GetIniPath()
	{
		static CString ini_fullpath = ini_filepath;
		if (ini_fullpath != ini_filepath) return ini_fullpath;
		CString module_name;
		AfxGetModuleFileName(nullptr, module_name);
		// remove exe name
		int last_backslash_index = module_name.ReverseFind(_T('\\'));
		if (last_backslash_index != -1)
		{
			module_name = module_name.Left(last_backslash_index + 1);
		} else
		{
			module_name = _T(".\\");
		}
		ini_fullpath = module_name + ini_filepath;
		return ini_fullpath;
	}

	void LoadSettingsFromINI()
	{
		CString ini_fullpath = GetIniPath();
		ATLTRACE(_T("info: loading settings from ini file %s\n"), ini_fullpath.GetString());
		lyric_font_size = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontSize"), 24, ini_fullpath);
		lyric_font_color = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontColor"), static_cast<int>(D2D1::ColorF::Black), ini_fullpath);
		lyric_font_color_translation = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontColorTranslation"), static_cast<int>(D2D1::ColorF::DarkGray), ini_fullpath);

		LPTSTR buffer = lyric_font_name.GetBuffer(MAX_PATH);
		GetPrivateProfileString(_T("LyricSettings"), _T("LyricFontName"), _T("Microsoft YaHei UI"), buffer, MAX_PATH, ini_fullpath);
		lyric_font_name.ReleaseBuffer();

		lyric_font_bold = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontBold"), false, ini_fullpath);
		lyric_font_italic = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricFontItalic"), false, ini_fullpath);
		lyric_aux_font_bold = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricAuxFontBold"), false, ini_fullpath);
		lyric_aux_font_italic = GetPrivateProfileInt(_T("LyricSettings"), _T("LyricAuxFontItalic"), false, ini_fullpath);

		buffer = lyric_aux_font_name.GetBuffer(MAX_PATH);
		GetPrivateProfileString(_T("LyricSettings"), _T("LyricAuxFontName"), _T("Microsoft YaHei UI"), buffer, MAX_PATH, ini_fullpath);
		lyric_aux_font_name.ReleaseBuffer();

		eq_preset_id = GetPrivateProfileInt(_T("EqSettings"), _T("EqPresetID"), 0, ini_fullpath);
		for (int i = 0; i < 10; ++i)
		{
			CString keyName;
			keyName.Format(_T("EqBand%d"), i);
			eq_bands.Add(GetPrivateProfileInt(_T("EqSettings"), keyName.GetString(), 0, ini_fullpath));
		}
	}
	void SaveSettingsToINI()
	{
		CString ini_fullpath = GetIniPath();
		ATLTRACE(_T("info: saving settings to ini file %s\n"), ini_fullpath.GetString());
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontSize"), std::to_wstring(lyric_font_size).c_str(), ini_fullpath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontColor"), std::to_wstring(static_cast<int>(lyric_font_color)).c_str(), ini_fullpath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontColorTranslation"), std::to_wstring(static_cast<int>(lyric_font_color_translation)).c_str(), ini_fullpath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontName"), lyric_font_name, ini_fullpath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontBold"), std::to_wstring(lyric_font_bold).c_str(), ini_fullpath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricFontItalic"), std::to_wstring(lyric_font_italic).c_str(), ini_fullpath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricAuxFontName"), lyric_aux_font_name, ini_fullpath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricAuxFontBold"), std::to_wstring(lyric_aux_font_bold).c_str(), ini_fullpath);
		WritePrivateProfileString(_T("LyricSettings"), _T("LyricAuxFontItalic"), std::to_wstring(lyric_aux_font_italic).c_str(), ini_fullpath);

		WritePrivateProfileString(_T("EqSettings"), _T("EqPresetID"), std::to_wstring(eq_preset_id).c_str(), ini_fullpath);
		for (int i = 0; i < 10; ++i)
		{
			CString keyName;
			keyName.Format(_T("EqBand%d"), i);
			WritePrivateProfileString(_T("EqSettings"), keyName.GetString(), std::to_wstring(eq_bands[i]).c_str(), ini_fullpath);
		}
	}

	bool IsIniExists() const {
		CString ini_fullpath = GetIniPath();
		ATLTRACE(_T("info: ini_path %s exists? %d\n"), GetIniPath().GetString(), _taccess(ini_fullpath, 0));
		return _taccess(ini_fullpath, 0) == 0;
	}

	void CreateIniAndWriteDefault() {
		if (IsIniExists()) return;
		SaveSettingsToINI();
	}

	void LoadIniOrDefault() {
		if (IsIniExists()) {
			ATLTRACE("info: ini exists, loading settings\n");
			LoadSettingsFromINI();
		}
		else CreateIniAndWriteDefault();
	}

	void SaveIni() { SaveSettingsToINI(); }
#pragma endregion
};

