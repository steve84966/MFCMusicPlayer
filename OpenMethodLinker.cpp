#include "pch.h"
#include "OpenMethodLinker.h"

CString OpenMethodLinker::GetCurrentExecutablePath() {
    CString path;
    auto path_buffer = path.GetBufferSetLength(2048);
    ::GetModuleFileName(nullptr, path_buffer, 2048);
    path.ReleaseBuffer();
    return path;
}

CString OpenMethodLinker::GetWorkingDirectory() {
    CString module_name = GetCurrentExecutablePath();
    int last_backslash_index = module_name.ReverseFind(_T('\\'));
    if (last_backslash_index != -1)
    {
        module_name = module_name.Left(last_backslash_index + 1);
    } else
    {
        module_name = _T(".\\");
    }
    return module_name;
}

void OpenMethodLinker::LinkNcmOpenMethod() {
    CString strReg =
        _T("Windows Registry Editor Version 5.00\n\n")
        _T("[HKEY_CLASSES_ROOT\\.ncm]\n")
        _T("@=\"MFCMusicPlayer.Ncm\"\n\n")
        _T("[HKEY_CLASSES_ROOT\\MFCMusicPlayer.Ncm]\n")
        _T("@=\"NCM Encoded music\"\n\n")
        _T("[HKEY_CLASSES_ROOT\\MFCMusicPlayer.Ncm\\DefaultIcon]\n")
        _T("@=\"{EXECUTABLE_PATH},{ICON_ID}\"\n\n")
        _T("[HKEY_CLASSES_ROOT\\MFCMusicPlayer.Ncm\\shell\\open\\command]\n")
        _T("@=\"{EXECUTABLE_PATH} \\\"%1\\\"\"\n");

    CString exe_path = GetCurrentExecutablePath();
    exe_path.Replace(_T("\\"), _T("\\\\"));
    strReg.Replace(_T("{EXECUTABLE_PATH}"), exe_path.GetString());
    CString icon_id_str;
    icon_id_str.Format(_T("%d"), NCM_LOGO_ID);
    strReg.Replace(_T("{ICON_ID}"), icon_id_str);
    ATLTRACE(_T("info: reg built: %s\n"), strReg.GetString());

    CStdioFile file;
    if (file.Open(GetWorkingDirectory() + _T("assoc.reg"), CFile::modeCreate | CFile::modeWrite)) {
        file.WriteString(strReg);
        file.Close();
    }

    auto parameter_str = GetWorkingDirectory();
    parameter_str += _T("\\assoc.reg");
    parameter_str.Insert(0, _T("/s "));
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = _T("runas");
    sei.lpFile = _T("regedit.exe");
    sei.lpParameters = parameter_str.GetString();
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteEx(&sei))
    {
        DWORD dwError = GetLastError();
        ATLTRACE("warn: ShellExecuteEx failed, reason = %d\n", dwError);
        AfxMessageBox(_T("打开方式设置失败"), MB_ICONEXCLAMATION | MB_OK);
    }
    ::PostMessage(HWND_BROADCAST, WM_COMMAND, 41504, NULL);
}

void OpenMethodLinker::UnlinkNcmOpenMethod() {
    CString strReg =
        _T("Windows Registry Editor Version 5.00\n\n")
        _T("[-HKEY_CLASSES_ROOT\\.ncm]\n")
        _T("[-HKEY_CLASSES_ROOT\\MFCMusicPlayer.Ncm]");

    ATLTRACE(_T("info: reg built: %s\n"), strReg.GetString());

    CStdioFile file;
    if (file.Open(GetWorkingDirectory() + _T("deassoc.reg"), CFile::modeCreate | CFile::modeWrite)) {
        file.WriteString(strReg);
        file.Close();
    }

    auto parameter_str = GetWorkingDirectory();
    parameter_str += _T("\\deassoc.reg");
    parameter_str.Insert(0, _T("/s "));
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = _T("runas");
    sei.lpFile = _T("regedit.exe");
    sei.lpParameters = parameter_str.GetString();
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteEx(&sei))
    {
        DWORD dwError = GetLastError();
        ATLTRACE("warn: ShellExecuteEx failed, reason = %d\n", dwError);
        AfxMessageBox(_T("打开方式设置失败"), MB_ICONEXCLAMATION | MB_OK);
    }
    ::PostMessage(HWND_BROADCAST, WM_COMMAND, 41504, NULL);
}
