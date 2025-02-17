#include "stdafx.h"
#include "AsioHelpers.h"
#include "AsioSharedHost.h"

static const TCHAR* ASIO_PATH = TEXT("software\\asio");
static const TCHAR* ASIO_DESC = TEXT("description");
static const TCHAR* COM_CLSID = TEXT("clsid");
static const TCHAR* INPROC_SERVER = TEXT("InprocServer32");

template<size_t bufferSize>
static bool ReadRegistryStringA(std::string& out, HKEY hKey, const TCHAR* valueName)
{
	char buffer[bufferSize];
	DWORD dataType = 0;
	DWORD readSize = bufferSize;
	LSTATUS regStatus = RegQueryValueEx(hKey, valueName, nullptr, &dataType, (BYTE*)buffer, &readSize);
	if (regStatus != ERROR_SUCCESS || dataType != REG_SZ)
		return false;

	if (readSize == 0)
	{
		out.clear();
		return false;
	}

#ifdef UNICODE
	char converted[bufferSize];

	if (WideCharToMultiByte(CP_ACP, 0, (wchar_t*)buffer, readSize / sizeof(wchar_t), converted, bufferSize, nullptr, nullptr) == 0)
		return false;

	out = converted;
#else
	out = buffer;
#endif

	return true;
}

template<size_t bufferSize>
static bool ReadRegistryStringW(std::wstring& out, HKEY hKey, const TCHAR* valueName)
{
	char buffer[bufferSize];
	DWORD dataType = 0;
	DWORD readSize = bufferSize;
	LSTATUS regStatus = RegQueryValueEx(hKey, valueName, nullptr, &dataType, (BYTE*)buffer, &readSize);
	if (regStatus != ERROR_SUCCESS || dataType != REG_SZ)
		return false;

	if (readSize == 0)
	{
		out.clear();
		return false;
	}

#ifdef UNICODE
	out = (wchar_t*)buffer;
#else
	wchar_t converted[bufferSize];

	if (MultiByteToWideChar(CP_ACP, 0, buffer, readSize, converted, bufferSize) == 0)
		return false;

	out = converted;
#endif

	return true;
}

static bool ReadRegistryClsid(CLSID& out, HKEY hKey, const TCHAR* valueName)
{
	//RegQueryValueEx

#ifdef UNICODE
	std::wstring str;
	bool res = ReadRegistryStringW<128>(str, hKey, valueName);
#else
	std::string str;
	bool res = ReadRegistryStringA<128>(str, hKey, valueName);
#endif
	if (!res)
		return false;

	HRESULT hr = CLSIDFromString(str.c_str(), &out);
	if (FAILED(hr))
		return false;

	return true;
}

static bool GetRegistryAsioDriverPath(std::string& out, const CLSID& clsid)
{
	LPOLESTR clsidStr = nullptr;
	if (FAILED(StringFromCLSID(clsid, &clsidStr)))
	{
		return false;
	}

	bool result = false;
	HKEY hKeyClsidRoot = nullptr;

	LSTATUS regStatus = RegOpenKey(HKEY_CLASSES_ROOT, COM_CLSID, &hKeyClsidRoot);
	if (regStatus == ERROR_SUCCESS)
	{
		HKEY hKeyClsidFound = nullptr;
		regStatus = RegOpenKeyEx(hKeyClsidRoot, clsidStr, 0, KEY_READ, &hKeyClsidFound);
		if (regStatus == ERROR_SUCCESS)
		{
			HKEY hKeyInprocServer = nullptr;
			regStatus = RegOpenKeyEx(hKeyClsidFound, INPROC_SERVER, 0, KEY_READ, &hKeyInprocServer);
			if (regStatus == ERROR_SUCCESS)
			{
				result = ReadRegistryStringA<MAX_PATH>(out, hKeyInprocServer, nullptr);
				RegCloseKey(hKeyInprocServer);
			}

			RegCloseKey(hKeyClsidFound);
		}

		RegCloseKey(hKeyClsidRoot);
	}

	CoTaskMemFree(clsidStr);

	return result;
}

static bool GetRegistryDriverInfo(AsioHelpers::DriverInfo& outInfo, HKEY hKey, const TCHAR* keyName)
{
	bool result = false;

	HKEY	hksub;
	LSTATUS regStatus = RegOpenKeyEx(hKey, keyName, 0, KEY_READ, &hksub);
	if (regStatus == ERROR_SUCCESS)
	{
		AsioHelpers::DriverInfo tmpInfo;

		// read CLSID
		result = ReadRegistryClsid(tmpInfo.Clsid, hksub, COM_CLSID);
		if (result)
		{
			// read name
			if (!ReadRegistryStringA<128>(tmpInfo.Name, hksub, ASIO_DESC))
			{
				tmpInfo.Name = "NoName";
			}

			// figure out DLL path
			result = GetRegistryAsioDriverPath(tmpInfo.DllPath, tmpInfo.Clsid);
		}

		RegCloseKey(hksub);

		if (result)
		{
			outInfo = std::move(tmpInfo);
		}
	}

	return result;
}

std::vector<AsioHelpers::DriverInfo> AsioHelpers::FindDrivers()
{
	std::cout << "\n" __FUNCTION__ "\n";

	std::vector<AsioHelpers::DriverInfo> result;

	HKEY hkEnum = 0;
	LSTATUS regStatus = RegOpenKey(HKEY_LOCAL_MACHINE, ASIO_PATH, &hkEnum);
	if (regStatus == ERROR_SUCCESS)
	{
		TCHAR keyName[MAX_PATH];
		DWORD keyIndex = 0;

		while (RegEnumKey(hkEnum, keyIndex++, keyName, MAX_PATH) == ERROR_SUCCESS)
		{
			AsioHelpers::DriverInfo info;
			if (GetRegistryDriverInfo(info, hkEnum, keyName))
			{
				std::cout << "  " << info.Name.c_str() << "\n";
				result.push_back(std::move(info));
			}
		}

		RegCloseKey(hkEnum);
	}

	return result;
}

AsioSharedHost* AsioHelpers::CreateAsioHost(const DriverInfo& driverInfo)
{
	AsioSharedHost* host = new AsioSharedHost(driverInfo.Clsid, driverInfo.DllPath);
	if (!host->IsValid())
	{
		host->Release();
		host = nullptr;
	}

	return host;
}