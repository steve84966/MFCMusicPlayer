#include "pch.h"
#include "WinRT_SMTCController.h"
#include <systemmediatransportcontrolsinterop.h>

WinRT_SMTCController WinRT_SMTCController::instance;

BEGIN_MESSAGE_MAP(WinRT_SMTCController, CWnd)
    ON_MESSAGE(WM_PLAYER_UPDATE_SMTC, &WinRT_SMTCController::OnSMTCUpdate)
    ON_MESSAGE(WM_PLAYER_UPDATE_SMTC_STATUS, &WinRT_SMTCController::OnSMTCUpdateStatus)
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
    SMTC.DisplayUpdater().Type(winrt::Windows::Media::MediaPlaybackType::Music);
    SMTC.DisplayUpdater().Update();

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

// ReSharper disable once CppDFAConstantFunctionResult
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

        if (winrt::Windows::Storage::Streams::IRandomAccessStream stream =
                ConvertHBitmapToStream(updateInfo->albumPic)) {
            auto reference = winrt::Windows::Storage::Streams::RandomAccessStreamReference::CreateFromStream(stream);
            updater.Thumbnail(reference);
        }

        updater.Update();
        SMTC.IsEnabled(true);
    }
    catch (const winrt::hresult_error& ex) {
        ATLTRACE("error: SMTC update failed: %S\n", ex.message().c_str());
    }
    delete updateInfo;
    return 0;
}

LRESULT WinRT_SMTCController::OnSMTCUpdateStatus(WPARAM wParam, LPARAM lParam) {
    auto status = static_cast<winrt::Windows::Media::MediaPlaybackStatus>(wParam);
    SMTC.PlaybackStatus(status);
    return {};
}


winrt::Windows::Storage::Streams::IRandomAccessStream WinRT_SMTCController::ConvertHBitmapToStream(HBITMAP hbitmap) {
    if (!hbitmap)
        return nullptr;

    BITMAP bmp;
    if (GetObject(hbitmap, sizeof(BITMAP), &bmp) == 0)
        return nullptr;

    HDC hScreenDC = ::GetDC(nullptr);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);
    auto hOldBmp = static_cast<HBITMAP>(SelectObject(hMemDC, hbitmap));

    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = -bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize = bmp.bmWidth * bmp.bmHeight * 4;
    std::vector<BYTE> pixelData(dwBmpSize);

    GetDIBits(hMemDC, hbitmap, 0, bmp.bmHeight, pixelData.data(), 
              reinterpret_cast<BITMAPINFO *>(&bi), DIB_RGB_COLORS);

    SelectObject(hMemDC, hOldBmp);
    DeleteDC(hMemDC);
    ::ReleaseDC(nullptr, hScreenDC);

    BITMAPFILEHEADER bfh = {};
    bfh.bfType = 0x4D42;
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    std::vector<BYTE> bmpData;
    bmpData.reserve(bfh.bfSize);
    bmpData.insert(bmpData.end(), reinterpret_cast<BYTE *>(&bfh), reinterpret_cast<BYTE *>(&bfh) + sizeof(bfh));
    bmpData.insert(bmpData.end(), reinterpret_cast<BYTE *>(&bi), reinterpret_cast<BYTE *>(&bi) + sizeof(bi));
    bmpData.insert(bmpData.end(), pixelData.begin(), pixelData.end());

    try {
        return std::async(std::launch::async, [bmpData = std::move(bmpData)]() -> winrt::Windows::Storage::Streams::IRandomAccessStream {
            winrt::init_apartment(winrt::apartment_type::multi_threaded);

            winrt::Windows::Storage::Streams::InMemoryRandomAccessStream stream;
            {
                winrt::Windows::Storage::Streams::DataWriter writer(stream);
                writer.WriteBytes(winrt::array_view(bmpData.data(), bmpData.data() + bmpData.size()));
                UNREFERENCED_PARAMETER(writer.StoreAsync().get());
                UNREFERENCED_PARAMETER(writer.FlushAsync().get());
                UNREFERENCED_PARAMETER(writer.DetachStream());
            }

            stream.Seek(0);
            return stream;
        }).get();
    }
    catch (const winrt::hresult_error& ex) {
        ATLTRACE("err: ConvertHBitmapToStream failed: 0x%08X %S\n",
            ex.code(), ex.message().c_str());
        return nullptr;
    }
}
