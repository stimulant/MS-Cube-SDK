#pragma once
#include <string>

class RegistryHelper
{
public:
	static bool RegistryHelper::GetBoolRegValue(HKEY hKey, const std::string &strValueName, bool &bValue, bool bDefaultValue);
	static bool RegistryHelper::GetIntRegValue(HKEY hKey, const std::string &strValueName, int &iValue, int iDefaultValue);
	static bool RegistryHelper::GetStringRegValue(HKEY hKey, const std::string &strValueName, std::string &strValue, const std::string &strDefaultValue);
	static bool RegistryHelper::CreateRegistryKey(HKEY hKeyRoot, LPCTSTR pszSubKey, HKEY &hNewKey);
};

