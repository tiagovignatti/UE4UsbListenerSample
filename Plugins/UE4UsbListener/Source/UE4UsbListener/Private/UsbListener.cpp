#include "UsbListener.h"

#include "Windows/PreWindowsApi.h"
#include <windows.h>
#include <winuser.h>
#include <Dbt.h>
#include <stdexcept>
#include <sstream>
#include <regex>
#include <initguid.h>
#include <Usbiodef.h>
#include <SetupAPI.h>
#include "Windows/PostWindowsApi.h"

std::shared_ptr<UsbListener> UsbListener::instance = nullptr;
std::mutex UsbListener::instanceMutex;

UsbListener::UsbListener()
{
	init = false;

	FWindowsApplication* WindowsApplication = (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();
	check(WindowsApplication);

	WindowsApplication->AddMessageHandler(*this);
}

UsbListener::~UsbListener()
{
	Stop();
}

void UsbListener::Stop()
{
}

static const GUID UsbGuid =
{
	// GUID_DEVINTERFACE_USB_DEVICE
	0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED },
};

bool DeviceIsValid(PDEV_BROADCAST_DEVICEINTERFACE_A lpdbv)
{
	if (lpdbv == nullptr)
	{
		return false;
	}

	// Check if it's a USB device.
	bool isUsb = (memcmp(&(lpdbv->dbcc_classguid), &UsbGuid, sizeof(GUID)) == 0);
	return isUsb;
}

int64_t UsbListener::HandleHotplugMessage(void* hwndPtr, uint32_t uint, uint64_t wparam, int64_t lparam)
{
	switch (uint)
	{
	case WM_DEVICECHANGE:
	{
		if (wparam != DBT_DEVICEARRIVAL && wparam != DBT_DEVICEREMOVECOMPLETE)
		{
			break;
		}
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lparam;
		PDEV_BROADCAST_DEVICEINTERFACE_A lpdbv = (PDEV_BROADCAST_DEVICEINTERFACE_A)lpdb;
		
		if (lpdb == nullptr)
		{
			break;
		}
		if (DeviceIsValid(lpdbv) == false)
		{
			break;
		}
		
		// God damn it! https://stackoverflow.com/a/27512342/1415058
		wchar_t* name = (wchar_t*)lpdbv->dbcc_name;
		std::string mystring;
		while (*name)
			mystring += (char)*name++;
		
		if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
		{
			if (usbDeviceChangeCallback != nullptr)
			{
				UE_LOG(UE4UsbListener, Verbose, TEXT("HandleHotplugMessage: WM_DEVICECHANGE: %s"), name);
				usbDeviceChangeCallback(mystring, wparam == DBT_DEVICEARRIVAL);
			}
		}
		break;
	}
	default:
		break;

	}
	return 0L;
}

HWND GetWindowHandle()
{
	HWND hWnd = nullptr;

	const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (ParentWindow.IsValid() && (ParentWindow->GetNativeWindow().IsValid()))
	{
		hWnd = (HWND)ParentWindow->GetNativeWindow()->GetOSWindowHandle();
	}

	return hWnd;
}

void UsbListener::SetDeviceChangeCallback(UsbDeviceChangeCallback callback)
{
	this->usbDeviceChangeCallback = callback;
}

void UsbListener::SetDeviceQueryCallback(UsbDeviceQueryCallback callback)
{
	this->usbDeviceQueryCallback = callback;
}

bool UsbListener::Start()
{
	UE_LOG(UE4UsbListener, Log, TEXT("Start"));
	if (init)
	{
		return true;
	}

	// Query for the current attached USB devices
	{
		HDEVINFO devicesHandle = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
		SP_DEVINFO_DATA deviceInfo;
		ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
		deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
		DWORD deviceNumber = 0;
		SP_DEVICE_INTERFACE_DATA devinterfaceData;
		devinterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		while (SetupDiEnumDeviceInterfaces(devicesHandle, NULL, &GUID_DEVINTERFACE_USB_DEVICE, deviceNumber++, &devinterfaceData))
		{
			DWORD bufSize = 0;
			SetupDiGetDeviceInterfaceDetail(devicesHandle, &devinterfaceData, NULL, NULL, &bufSize, NULL);
			BYTE* buffer = new BYTE[bufSize];
			PSP_DEVICE_INTERFACE_DETAIL_DATA devinterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)buffer;
			devinterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
			SetupDiGetDeviceInterfaceDetail(devicesHandle, &devinterfaceData, devinterfaceDetailData, bufSize, NULL, NULL);

			if (usbDeviceQueryCallback != nullptr)
			{
				// God damn it! https://stackoverflow.com/a/27512342/1415058
				wchar_t* name = (wchar_t*)devinterfaceDetailData->DevicePath;
				std::string mystring;
				while (*name)
					mystring += (char)*name++;

				UE_LOG(UE4UsbListener, Verbose, TEXT("UsbListener::Start: device: %s"), name);
				usbDeviceQueryCallback(mystring);
			}
		}
	}

	HWND windowHandle = GetWindowHandle();
	if (windowHandle == nullptr)
	{
		auto errorCode = GetLastError();
		return false;
	}

	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
	ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	HDEVNOTIFY dev_notify = RegisterDeviceNotification(windowHandle, &NotificationFilter,
		DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
	if (dev_notify == NULL)
	{
		UE_LOG(UE4UsbListener, Error, TEXT("Could not register for device notifications!"));
	}

	init = true;

	return true;
}

std::shared_ptr<UsbListener> UsbListener::GetInstance()
{
	std::lock_guard<std::mutex> lock(instanceMutex);
	if (instance == nullptr)
	{
		instance = std::shared_ptr<UsbListener>(new UsbListener());
	}
	return instance;
}

bool UsbListener::ProcessMessage(const HWND hwnd, const uint32 Msg, const WPARAM wParam, const LPARAM lParam, int32& OutResult)
{
	bool bHandled = false;

	UsbListener::GetInstance()->HandleHotplugMessage(hwnd, Msg, wParam, lParam);

	return bHandled;
}