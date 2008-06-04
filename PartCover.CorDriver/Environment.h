#pragma once

namespace Environment {
    bool    GetEnvironmentBoolOption(LPCTSTR optionName, bool defaultValue);
    LPCTSTR GetEnvironmentStringOption(LPCTSTR optionName);

    void FreeStringResource(LPCTSTR);
}
