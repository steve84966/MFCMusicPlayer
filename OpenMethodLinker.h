#pragma once

#include "framework.h"

class OpenMethodLinker
{
    static constexpr int NCM_LOGO_ID = 13;

public:
    static CString GetCurrentExecutablePath();
    static CString GetWorkingDirectory();
    static void LinkNcmOpenMethod();
    static void UnlinkNcmOpenMethod();
};

