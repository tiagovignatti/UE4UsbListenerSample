#if PLATFORM_LINUX
#include "UsbListenerLinux.h"

#include <libudev.h>

std::shared_ptr<UsbListenerLinux> UsbListenerLinux::instance = nullptr;
std::mutex UsbListenerLinux::instanceMutex;

UsbListenerLinux::UsbListenerLinux()
{
	init = false;


}

UsbListenerLinux::~UsbListenerLinux()
{
	Stop();
}

void UsbListenerLinux::Stop()
{
	init = false;
}

void UsbListenerLinux::SetDeviceChangeCallback(UsbDeviceChangeCallback callback)
{
	this->usbDeviceChangeCallback = callback;
}

void UsbListenerLinux::SetDeviceQueryCallback(UsbDeviceQueryCallback callback)
{
	this->usbDeviceQueryCallback = callback;
}

bool UsbListenerLinux::Start()
{
	UE_LOG(UE4UsbListenerLinux, Verbose, TEXT("Start"));
	if (init)
	{
		return true;
	}

	// Create a new udev object
	struct udev* udev = udev_new();
	if (!udev) {
		printf("Error while initialization!\n");
		return EXIT_FAILURE;
	}

	init = true;

	return true;
}

std::shared_ptr<UsbListenerLinux> UsbListenerLinux::GetInstance()
{
	std::lock_guard<std::mutex> lock(instanceMutex);
	if (instance == nullptr)
	{
		instance = std::shared_ptr<UsbListenerLinux>(new UsbListenerLinux());
	}
	return instance;
}
#endif