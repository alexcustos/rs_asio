#include "stdafx.h"
#include "DebugDeviceEnum.h"

#include "RSAggregatorDeviceEnum.h"
#include "DebugWrapperDevice.h"

DebugDeviceEnum::DebugDeviceEnum(IMMDeviceEnumerator* enumerator)
	: m_RealEnumerator(enumerator)
{
	std::cout << "\n\n NEW DEVICE ENUM\n\n";

	if (m_RealEnumerator)
	{
		m_RealEnumerator->AddRef();
		m_DeviceListNeedsUpdate = true;
	}
}

DebugDeviceEnum::~DebugDeviceEnum()
{
	if (m_RealEnumerator)
	{
		m_RealEnumerator->Release();
		m_RealEnumerator = nullptr;
	}
}

HRESULT STDMETHODCALLTYPE DebugDeviceEnum::QueryInterface(REFIID riid, void **ppvObject)
{
	std::cout << __FUNCTION__ "\n";

	return m_RealEnumerator->QueryInterface(riid, ppvObject);
}

HRESULT STDMETHODCALLTYPE DebugDeviceEnum::EnumAudioEndpoints(EDataFlow dataFlow, DWORD dwStateMask, IMMDeviceCollection **ppDevices)
{
	std::cout << __FUNCTION__ " - dataFlow: " << Dataflow2String(dataFlow) << " - dwStateMask: " << dwStateMask << "\n";
	
	HRESULT hr = RSBaseDeviceEnum::EnumAudioEndpoints(dataFlow, dwStateMask, ppDevices);
	std::cout << "  hr: " << std::hex << hr << "\n";
	if (ppDevices)
	{
		std::cout << "  *ppDevices: " << *ppDevices << "\n";
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE DebugDeviceEnum::GetDefaultAudioEndpoint(EDataFlow dataFlow, ERole role, IMMDevice **ppEndpoint)
{
	std::cout << __FUNCTION__ " - dataFlow: " << Dataflow2String(dataFlow) << " - role: " << Role2String(role) << "\n";

	HRESULT hr = RSBaseDeviceEnum::GetDefaultAudioEndpoint(dataFlow, role, ppEndpoint);
	std::cout << "  hr: " << std::hex << hr << "\n";
	if (ppEndpoint)
	{
		std::cout << "  *ppEndpoint: " << *ppEndpoint << "\n";
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE DebugDeviceEnum::GetDevice(LPCWSTR pwstrId, IMMDevice **ppDevice)
{
	HRESULT hr = RSBaseDeviceEnum::GetDevice(pwstrId, ppDevice);
	std::cout << "  hr: " << std::hex << hr << "\n";
	if (ppDevice)
	{
		std::cout << "  *ppEndpoint: " << *ppDevice << "\n";
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE DebugDeviceEnum::RegisterEndpointNotificationCallback(IMMNotificationClient *pClient)
{
	std::cout << __FUNCTION__ "\n";

	return m_RealEnumerator->RegisterEndpointNotificationCallback(pClient);
}

HRESULT STDMETHODCALLTYPE DebugDeviceEnum::UnregisterEndpointNotificationCallback(IMMNotificationClient *pClient)
{
	std::cout << __FUNCTION__ "\n";

	return m_RealEnumerator->UnregisterEndpointNotificationCallback(pClient);
}

void DebugDeviceEnum::UpdateAvailableDevices()
{
	RSDeviceCollection aggregatedRenderCollection;
	RSDeviceCollection aggregatedCaptureCollection;

	// release defaults
	for (IMMDevice*& dev : m_DefaultRenderDevices)
	{
		if (dev)
		{
			dev->Release();
			dev = nullptr;
		}
	}
	for (IMMDevice*& dev : m_DefaultCaptureDevices)
	{
		if (dev)
		{
			dev->Release();
			dev = nullptr;
		}
	}

	auto fnUpdateCollection = [](IMMDeviceEnumerator* enumerator, EDataFlow dataFlow, RSDeviceCollection& out, std::array<IMMDevice*, ERole_enum_count>& outDefaults)
	{
		IMMDeviceCollection* collection = nullptr;
		if (SUCCEEDED(enumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATEMASK_ALL, &collection)))
		{
			UINT num = 0;
			collection->GetCount(&num);
			for (UINT i = 0; i < num; ++i)
			{
				IMMDevice* dev = nullptr;
				if (SUCCEEDED(collection->Item(i, &dev)) && dev)
				{
					IMMDevice* debugDev = new DebugWrapperDevice(*dev);
					dev->Release();
					out.AddDevice(debugDev);
					debugDev->Release();
				}
			}

			collection->Release();
		}

		for (int i = eConsole; i < ERole_enum_count; ++i)
		{
			const ERole role = (ERole)i;
			if (!outDefaults[role])
			{
				IMMDevice* defaultDevice = nullptr;

				if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(dataFlow, role, &defaultDevice)))
				{
					outDefaults[role] = new DebugWrapperDevice(*defaultDevice);
					defaultDevice->Release();
				}
			}
		}
	};

	if (m_RealEnumerator)
	{
		fnUpdateCollection(m_RealEnumerator, eRender, aggregatedRenderCollection, m_DefaultRenderDevices);
		fnUpdateCollection(m_RealEnumerator, eCapture, aggregatedCaptureCollection, m_DefaultCaptureDevices);
	}

	m_RenderDevices.UpdateDevicesFromCollection(aggregatedRenderCollection, true);
	m_CaptureDevices.UpdateDevicesFromCollection(aggregatedCaptureCollection, true);

	std::cout << __FUNCTION__ << " - " << m_RenderDevices.size() << " render devices, " << m_CaptureDevices.size() << " capture devices\n";

	m_DeviceListNeedsUpdate = false;
}
