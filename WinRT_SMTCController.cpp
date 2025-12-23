#include "pch.h"
#include "WinRT_SMTCController.h"
#include <systemmediatransportcontrolsinterop.h>

WinRT_SMTCController WinRT_SMTCController::instance;

BEGIN_MESSAGE_MAP(WinRT_SMTCController, CCmdTarget)
    ON_MESSAGE(WM_PLAYER_UPDATE_SMTC, &WinRT_SMTCController::OnSMTCUpdate)
END_MESSAGE_MAP()

void WinRT_SMTCController::Initialize(HWND hWnd) {

    CreateEx(0, AfxRegisterWndClass(0), _T("SMTCControllerWnd"), WS_OVERLAPPED, CRect(0,0,0,0), nullptr, 0);

    ATLTRACE("info: initializing SystemMediaTransportControls");
    auto interop = winrt::get_activation_factory<
        winrt::Windows::Media::SystemMediaTransportControls,
        ISystemMediaTransportControlsInterop>();

    winrt::check_hresult(interop->GetForWindow(
        hWnd,
        winrt::guid_of<winrt::Windows::Media::SystemMediaTransportControls>(),
        winrt::put_abi(SMTC)));


    SMTC.IsPlayEnabled(true);
    SMTC.IsPauseEnabled(true);
    SMTC.IsStopEnabled(true);
    SMTC.IsNextEnabled(true);
    SMTC.IsPreviousEnabled(true);

    auto result = SMTC.ButtonPressed(
        [hWnd](winrt::Windows::Media::SystemMediaTransportControls const& sender,
                       winrt::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs const& args) {
                switch (args.Button()) {
                case winrt::Windows::Media::SystemMediaTransportControlsButton::Play:
                    ATLTRACE("info: SMTC controller: play button pressed\n");
                    ::PostMessage(hWnd, WM_SMTC_PLAY, 0, 0);
                    break;
                case winrt::Windows::Media::SystemMediaTransportControlsButton::Pause:
                    ATLTRACE("info: SMTC controller: pause button pressed\n");
                    ::PostMessage(hWnd, WM_SMTC_PAUSE, 0, 0);
                    break;
                case winrt::Windows::Media::SystemMediaTransportControlsButton::Next:
                    ATLTRACE("info: SMTC controller: next button pressed\n");
                    ::PostMessage(hWnd, WM_SMTC_PLAY_NEXT, 0, 0);
                    break;
                case winrt::Windows::Media::SystemMediaTransportControlsButton::Previous:
                    ATLTRACE("info: SMTC controller: previous button pressed\n");
                    ::PostMessage(hWnd, WM_SMTC_PLAY_PREV, 0, 0);
                    break;
                case winrt::Windows::Media::SystemMediaTransportControlsButton::Stop:
                    ATLTRACE("info: SMTC controller: stop button pressed\n");
                    ::PostMessage(hWnd, WM_SMTC_STOP, 0, 0);
                    break;
                default:
                    break;
        }
    });
    UNREFERENCED_PARAMETER(result);
}

LRESULT WinRT_SMTCController::OnSMTCUpdate(WPARAM wParam, LPARAM lParam) {
    auto updateInfo = reinterpret_cast<SMTCControllerUpdateInfo*>(wParam);

    if (!SMTC)  return 0;
    if (!updateInfo) {
        SMTC.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Stopped);
        return 0;
    }

    try {
        auto updater = SMTC.DisplayUpdater();
        updater.Type(winrt::Windows::Media::MediaPlaybackType::Music);

        auto musicProps = updater.MusicProperties();

        if (!updateInfo->title.IsEmpty()) {
            musicProps.Title(winrt::to_hstring(updateInfo->title.GetString()));
        }
        if (!updateInfo->artist.IsEmpty()) {
            musicProps.Artist(winrt::to_hstring(updateInfo->artist.GetString()));
        }
        if (!updateInfo->album.IsEmpty()) {
            musicProps.AlbumTitle(winrt::to_hstring(updateInfo->album.GetString()));
        }

        updater.Update();
        SMTC.IsEnabled(true);
        SMTC.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Playing);

        ATLTRACE("info: SMTC updated - Title: %S, Artist: %S, Album: %S\n",
                 updateInfo->title.GetString(),
                 updateInfo->artist.GetString(),
                 updateInfo->album.GetString());
    }
    catch (const winrt::hresult_error& ex) {
        ATLTRACE("error: SMTC update failed: %S\n", ex.message().c_str());
    }

    return 0;
}