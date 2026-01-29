#pragma once
#include "pch.h"
#include "framework.h"

struct SMTCControllerUpdateInfo {
    CString title;
    CString artist;
    CString album;
    HBITMAP albumPic;
};

class WinRT_SMTCController:
    public CWnd
{
#pragma region Singelton
public:
    WinRT_SMTCController(const WinRT_SMTCController&) = delete;
    WinRT_SMTCController& operator=(const WinRT_SMTCController&) = delete;
    static WinRT_SMTCController& GetInstance() { return instance; }
    ~WinRT_SMTCController() override = default;
    void Initialize(HWND hWnd);
private:
    winrt::Windows::Media::SystemMediaTransportControls SMTC{ nullptr };
    WinRT_SMTCController() = default; // NOLINT(*-use-equals-delete)
    static WinRT_SMTCController instance;
#pragma endregion

#pragma region Message Dispatching
protected:
    DECLARE_MESSAGE_MAP()
    static winrt::Windows::Storage::Streams::IRandomAccessStream ConvertHBitmapToStream(HBITMAP hbitmap);

public:
    afx_msg LRESULT OnSMTCUpdate(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSMTCUpdateStatus(WPARAM wParam, LPARAM lParam);
#pragma endregion
};

