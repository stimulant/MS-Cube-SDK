#include <windows.h>
#include "RegistryHelper.h"

bool RegistryHelper::GetBoolRegValue(HKEY hKey, const std::string &strValueName, bool &bValue, bool bDefaultValue)
{
    DWORD nDefValue((bDefaultValue) ? 1 : 0);
    DWORD nResult(nDefValue);
	DWORD dwBufferSize(sizeof(DWORD));
    LONG nError = ::RegQueryValueEx(hKey,
        strValueName.c_str(),
        0,
        NULL,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        bValue = (nResult != 0);
    }
    return (ERROR_SUCCESS == nError);
}


LONG RegistryHelper::GetStringRegValue(HKEY hKey, const std::string &strValueName, std::string &strValue, const std::string &strDefaultValue)
{
    strValue = strDefaultValue;
    CHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueEx(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        strValue = szBuffer;
    }
    return (ERROR_SUCCESS == nError);
}

bool RegistryHelper::CreateRegistryKey(HKEY hKeyRoot, LPCTSTR pszSubKey, HKEY &hNewKey)
{
    DWORD dwFunc;
    LONG  lRet;
    SECURITY_DESCRIPTOR SD;
    SECURITY_ATTRIBUTES SA;

    if(!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
        return false;
    if(!SetSecurityDescriptorDacl(&SD, true, 0, false))
        return false;

    SA.nLength             = sizeof(SA);
    SA.lpSecurityDescriptor = &SD;
    SA.bInheritHandle      = false;
    lRet = RegCreateKeyEx(
        hKeyRoot,
        pszSubKey,
        0,
        (LPTSTR)NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        &SA,
        &hNewKey,
        &dwFunc
    );

    if(lRet == ERROR_SUCCESS)
        return true;

    SetLastError((DWORD)lRet);
    return false;
}