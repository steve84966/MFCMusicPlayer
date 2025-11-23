#pragma once

#pragma region ReSharper
// disable ReSharper warning when work with CLion

// ReSharper disable CppUnusedIncludeDirective
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppCStyleCast
// ReSharper disable CppDFAUnreadVariable
// ReSharper disable CppEntityAssignedButNoRead
// ReSharper disable CppJoinDeclarationAndAssignment
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppZeroConstantCanBeReplacedWithNullptr
// ReSharper disable CppDFAUnusedValue
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppFunctionalStyleCast
// ReSharper disable CppUseAuto
// ReSharper disable CppClassCanBeFinal
#pragma endregion

#pragma region Visual Studio Application Wizard

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的 NOLINT(*-reserved-identifier)

// 关闭 MFC 的一些常见且经常可放心忽略的隐藏警告消息
#define _AFX_ALL_WARNINGS // NOLINT(*-reserved-identifier)

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展

#include <atltrace.h>       // For Debug
#include <atlcoll.h>        // ATL Header File

#include <afxdisp.h>        // MFC 自动化类

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC 支持功能区和控制条
#pragma endregion

#pragma region Win32 Header and debug setting

#if defined(_WIN32) || defined(WIN32) || defined(__cplusplus)
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#if defined(_WIN32) || defined(WIN32) || defined(__cplusplus)
}
#endif

#if defined(_MSC_VER) // if uses msvc...
#define _CRT_SECURE_NO_WARNINGS // NOLINT(*-reserved-identifier)
#pragma warning (disable: 4819) // avoid msvc utf-8 warning
#endif

#define _CRTDBG_MAP_ALLOC // NOLINT(*-reserved-identifier)
#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#include <avrt.h>
#include <crtdbg.h>
#include <synchapi.h>
#include <ShellScalingAPI.h>
#include <io.h>
#pragma endregion

#pragma region Standard C++ Header Files
#include <cstdlib>
#include <cassert>
#include <list>
#include <functional>
#include <string>
#include <stack>
#include <algorithm>
#pragma endregion

#pragma region XAudio2 Header
#include <comdef.h>
#include <xaudio2.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#pragma endregion

#pragma region Direct2D Header
#include <d2d1.h>
#include <dwrite.h>
#pragma endregion

#pragma region DbgHelp Header
#include <dbghelp.h>
#pragma endregion

#pragma region ATL Trace Redirect Header
#include "AtlTraceRedirect.h"
#pragma endregion

#pragma region Modern Appearance
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
#pragma endregion

#pragma region User-defined Message
#define WM_PLAYER_FILE_INIT		  (WM_USER + 100)
#define WM_PLAYER_TIME_CHANGE	  (WM_USER + 101)
#define WM_PLAYER_START           (WM_USER + 102)
#define WM_PLAYER_PAUSE			  (WM_USER + 103)
#define WM_PLAYER_STOP			  (WM_USER + 104)
#define WM_PLAYER_ALBUM_ART_INIT  (WM_USER + 105)
#define WM_PLAYER_DESTROY		  (WM_USER + 106)
#pragma endregion

#pragma region Common Type Definition
enum audio_playback_state: unsigned long long
{
	audio_playback_state_init,
	audio_playback_state_playing,
	audio_playback_state_paused,
	audio_playback_state_decoder_exit_pre_stop,
	audio_playback_state_stopped
};

enum class LrcMetadataType
{
	Artist, Album, Author, By, Offset, Title, Ignored, Error
};

enum class ThreeWayCompareResult
{
	Less = -1, Equal = 0, Greater = 1
};
#pragma endregion

#pragma region Tool Macro
#define FFMPEG_CRITICAL_ERROR(err_code) \
	do { \
		dialog_ffmpeg_critical_error(err_code, __FILE__, __LINE__); \
	} while(0)

#define WAY3RES(ord) \
	((ord) == std::strong_ordering::less ? ThreeWayCompareResult::Less : \
	 (ord) == std::strong_ordering::greater ? ThreeWayCompareResult::Greater : \
	 ThreeWayCompareResult::Equal)
#pragma endregion

#pragma region Global Tool Function
float GetSystemDpiScale();
#pragma endregion