#include "StdAfx.h"
#include "environment.h"

namespace Environment {

    LPCTSTR GetEnvironmentStringOption(LPCTSTR optionName, DWORD* size) {
		*size = 0;

        LPTSTR buffer = 0;
        DWORD returnSize = GetEnvironmentVariable(optionName, buffer, 0);
        if (returnSize > 0) {
            buffer = new TCHAR[returnSize];
            if (returnSize - 1 != GetEnvironmentVariable(optionName, buffer, returnSize)) {
                FreeStringResource(buffer);
                buffer = 0;
            }
        }

		*size = returnSize;
        return buffer;
    }

    void FreeStringResource(LPCTSTR buffer) {
        delete[] buffer;
    }

}

