#include "stdafx.h"

DEFINE_PROPERTYKEY(PKEY_Device_DeviceIdHiddenKey1, 0xb3f8fa53, 0x0004, 0x438e, 0x90, 0x03, 0x51, 0xa4, 0x6e, 0x13, 0x9b, 0xfc, 2);
DEFINE_PROPERTYKEY(PKEY_Device_DeviceIdHiddenKey2, 0x83DA6326, 0x97A6, 0x4088, 0x94, 0x53, 0xA1, 0x92, 0x3F, 0x57, 0x3B, 0x29, 3);

std::string ConvertWStrToStr(const std::wstring& wstr)
{
	typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;

	static const std::locale locale("");
	const converter_type& converter = std::use_facet<converter_type>(locale);

	std::vector<char> to(wstr.length() * converter.max_length());
	std::mbstate_t state;
	const wchar_t* from_next;
	char* to_next;
	const converter_type::result result = converter.out(state, wstr.data(), wstr.data() + wstr.length(), from_next, &to[0], &to[0] + to.size(), to_next);

	if (result == converter_type::ok or result == converter_type::noconv)
	{
		return std::string(&to[0], to_next);
	}

	return std::string();
}

std::string IID2String(REFIID iid)
{
	std::string ret;

	LPOLESTR iidStr = nullptr;
	StringFromIID(iid, &iidStr);

	if (iidStr)
	{
#if defined(_WIN32) && !defined(OLE2ANSI)
		std::wstring wStr = iidStr;
		ret = ConvertWStrToStr(wStr);
#else
		ret = iidStr;
#endif
		CoTaskMemFree(iidStr);
	}

	return ret;
}

const char* Dataflow2String(EDataFlow dataFlow)
{
	if (dataFlow == eRender)
		return "eRender";
	else if (dataFlow == eCapture)
		return "eCapture";
	else if (dataFlow == eAll)
		return "eAll";
	return "N/A";
}

const char* Role2String(ERole role)
{
	switch (role)
	{
		case eConsole: return "eConsole";
		case eMultimedia: return "eMultimedia";
		case eCommunications: return "eCommunications";
	}
	return "N/A";
}

std::ostream & operator<<(std::ostream & os, REFIID iid)
{
	if (iid == __uuidof(IUnknown))
	{
		os << "IID_IUnknown";
	}
	else if (iid == __uuidof(IMMDevice))
	{
		os << "IID_IMMDevice";
	}
	else if (iid == __uuidof(IMMDeviceEnumerator))
	{
		os << "IID_IMMDeviceEnumerator";
	}
	else if (iid == __uuidof(IAudioClient))
	{
		os << "IID_IAudioClient";
	}
	else if (iid == __uuidof(IAudioClient2))
	{
		os << "IID_IAudioClient2";
	}
	else if (iid == __uuidof(IAudioClient3))
	{
		os << "IID_IAudioClient3";
	}
	else if (iid == __uuidof(IAudioRenderClient))
	{
		os << "IID_IAudioRenderClient";
	}
	else if (iid == __uuidof(IAudioCaptureClient))
	{
		os << "IID_IAudioCaptureClient";
	}
	else if (iid == __uuidof(IMMEndpoint))
	{
		os << "IID_IMMEndpoint";
	}
	else if (iid == __uuidof(IAudioEndpointVolume))
	{
		os << "IID_IAudioEndpointVolume";
	}
	else
	{
		LPOLESTR clsidStr = nullptr;
		if (FAILED(StringFromCLSID(iid, &clsidStr)))
		{
			os << "{N/A}";
			return os;
		}

		os << clsidStr;

		CoTaskMemFree(clsidStr);
	}

	return os;
}

std::ostream & operator<<(std::ostream & os, REFPROPERTYKEY key)
{
	if (key == PKEY_AudioEngine_DeviceFormat)
	{
		os << "PKEY_AudioEngine_DeviceFormat";
	}
	else if (key == PKEY_Device_FriendlyName)
	{
		os << "PKEY_Device_FriendlyName";
	}
	else if (key == PKEY_Device_DeviceIdHiddenKey1)
	{
		os << "PKEY_Device_DeviceIdHiddenKey1";
	}
	else if (key == PKEY_Device_DeviceIdHiddenKey2)
	{
		os << "PKEY_Device_DeviceIdHiddenKey2";
	}
	else if (key == PKEY_AudioEndpoint_FormFactor)
	{
		os << "PKEY_AudioEndpoint_FormFactor";
	}
	else
	{
		wchar_t propNameW[128];
		HRESULT hr = PSStringFromPropertyKey(key, propNameW, 128);
		if (SUCCEEDED(hr))
		{
			os << propNameW;
		}
		else
		{
			os << "N/A";
		}
	}

	return os;
}

std::ostream & operator<<(std::ostream & os, const wchar_t* wStr)
{
	std::string propName = ConvertWStrToStr(wStr);
	os << propName.c_str();
	return os;
}

std::ostream & operator<<(std::ostream & os, const std::wstring wStr)
{
	return os << wStr.c_str();
}

std::ostream & operator<<(std::ostream & os, const WAVEFORMATEX& fmt)
{
	os << "WAVEFORMATEX\n";
	os << "  wFormatTag: " << std::hex << fmt.wFormatTag << "\n";
	os << "  nChannels: " << std::dec << fmt.nChannels << "\n";
	os << "  nSamplesPerSec: " << std::dec << fmt.nSamplesPerSec << "\n";
	os << "  nAvgBytesPerSec: " << std::dec << fmt.nAvgBytesPerSec << "\n";
	os << "  nBlockAlign: " << std::dec << fmt.nBlockAlign << "\n";
	os << "  wBitsPerSample: " << std::dec << fmt.wBitsPerSample << "\n";
	os << "  cbSize: " << std::dec << fmt.cbSize << "\n";

	if (fmt.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		const WAVEFORMATEXTENSIBLE& ext = (WAVEFORMATEXTENSIBLE&)fmt;
		os << "  ext.SubFormat: " << std::hex << ext.SubFormat << "\n";
		os << "  ext.Samples: " << std::dec << ext.Samples.wSamplesPerBlock << "\n";
		os << "  ext.dwChannelMask: " << std::hex << ext.dwChannelMask << "\n";
	}

	return os;
}

std::ostream & operator<<(std::ostream & os, const AUDCLNT_SHAREMODE& mode)
{
	switch (mode)
	{
	case AUDCLNT_SHAREMODE_SHARED:
		os << "Shared";
		break;
	case AUDCLNT_SHAREMODE_EXCLUSIVE:
		os << "Exclusive";
		break;
	default:
		os << "?";
		break;
	}

	return os;
}

std::ostream & operator<<(std::ostream & os, const HResultToStr& hresult)
{
#define CASE_TO_STR(x) case x: os << #x; break

	switch (hresult.hr)
	{
		CASE_TO_STR(S_OK);
		CASE_TO_STR(S_FALSE);

		CASE_TO_STR(E_FAIL);
		CASE_TO_STR(E_POINTER);
		CASE_TO_STR(E_INVALIDARG);
		CASE_TO_STR(E_OUTOFMEMORY);

		CASE_TO_STR(AUDCLNT_E_ALREADY_INITIALIZED);
		CASE_TO_STR(AUDCLNT_E_WRONG_ENDPOINT_TYPE);
		CASE_TO_STR(AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED);
		CASE_TO_STR(AUDCLNT_E_BUFFER_SIZE_ERROR);
		CASE_TO_STR(AUDCLNT_E_CPUUSAGE_EXCEEDED);
		CASE_TO_STR(AUDCLNT_E_DEVICE_INVALIDATED);
		CASE_TO_STR(AUDCLNT_E_DEVICE_IN_USE);
		CASE_TO_STR(AUDCLNT_E_ENDPOINT_CREATE_FAILED);
		CASE_TO_STR(AUDCLNT_E_INVALID_DEVICE_PERIOD);
		CASE_TO_STR(AUDCLNT_E_UNSUPPORTED_FORMAT);
		CASE_TO_STR(AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED);
		CASE_TO_STR(AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL);
		CASE_TO_STR(AUDCLNT_E_SERVICE_NOT_RUNNING);
		CASE_TO_STR(AUDCLNT_E_EVENTHANDLE_NOT_SET);
		CASE_TO_STR(AUDCLNT_E_NOT_STOPPED);
	default:
		os << std::hex << hresult.hr;
		break;
	}

#undef CASE_TO_STR

	return os;
}

std::ostream & operator<<(std::ostream & os, ASIOSampleType sampleType)
{

#define CASE_TO_STR(x) case x: os << #x; break

	switch (sampleType)
	{
		CASE_TO_STR(ASIOSTInt16MSB);
		CASE_TO_STR(ASIOSTInt24MSB);
		CASE_TO_STR(ASIOSTInt32MSB);
		CASE_TO_STR(ASIOSTFloat32MSB);
		CASE_TO_STR(ASIOSTFloat64MSB);
		CASE_TO_STR(ASIOSTInt32MSB16);
		CASE_TO_STR(ASIOSTInt32MSB18);
		CASE_TO_STR(ASIOSTInt32MSB20);
		CASE_TO_STR(ASIOSTInt32MSB24);
		CASE_TO_STR(ASIOSTInt16LSB);
		CASE_TO_STR(ASIOSTInt24LSB);
		CASE_TO_STR(ASIOSTInt32LSB);
		CASE_TO_STR(ASIOSTFloat32LSB);
		CASE_TO_STR(ASIOSTFloat64LSB);
		CASE_TO_STR(ASIOSTInt32LSB16);
		CASE_TO_STR(ASIOSTInt32LSB18);
		CASE_TO_STR(ASIOSTInt32LSB20);
		CASE_TO_STR(ASIOSTInt32LSB24);
		CASE_TO_STR(ASIOSTDSDInt8LSB1);
		CASE_TO_STR(ASIOSTDSDInt8MSB1);
		CASE_TO_STR(ASIOSTDSDInt8NER8);
	default:
		os << std::dec << (long)sampleType;
		break;
	}

#undef CASE_TO_STR

	return os;
}

REFERENCE_TIME MilisecsToRefTime(LONGLONG ms)
{
	return ms * 10000;
}

LONGLONG RefTimeToMilisecs(const REFERENCE_TIME& time)
{
	return time / 10000;
}

LONGLONG DurationToAudioFrames(const REFERENCE_TIME& time, DWORD sampleRate)
{
	return (time * sampleRate) / 10000000;
}

REFERENCE_TIME AudioFramesToDuration(const LONGLONG& frames, DWORD sampleRate)
{
	return (frames * 10000000) / sampleRate;
}