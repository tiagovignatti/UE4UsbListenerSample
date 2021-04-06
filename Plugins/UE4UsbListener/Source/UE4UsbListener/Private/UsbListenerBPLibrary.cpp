// Fill out your copyright notice in the Description page of Project Settings.


#include "UsbListenerBPLibrary.h"
#if PLATFORM_WINDOWS
#include "UsbListenerWindows.h"
#elif PLATFORM_LINUX
#include "UsbListenerLinux.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(UE4UsbListener, Log, All);

void Callback(const std::string& deviceName, bool plugOn)
{
	FName TheFName(ANSI_TO_TCHAR(deviceName.c_str()));
	FString MyConvertedFName = TheFName.ToString();

	FString debugText = FString::Printf(TEXT("Device plug: %s, name: %s"), *FString::FromInt(plugOn), *MyConvertedFName);
	UE_LOG(UE4UsbListener, Log, TEXT("%s"), *debugText);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, debugText);
}

void DeviceQueryCallback(const std::string& deviceName)
{
	FName TheFName(ANSI_TO_TCHAR(deviceName.c_str()));
	FString MyConvertedFName = TheFName.ToString();

	FString debugText = FString::Printf(TEXT("Device attached: name: %s"), *MyConvertedFName);
	UE_LOG(UE4UsbListener, Log, TEXT("%s"), *debugText);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, debugText);
}

void UUsbListenerBPLibrary::StartUsbListening()
{
#if PLATFORM_WINDOWS
	auto listener = UsbListenerWindows::GetInstance();
	listener->SetDeviceQueryCallback(DeviceQueryCallback);
	listener->SetDeviceChangeCallback(Callback);
#elif PLATFORM_LINUX
	auto listener = UsbListenerLinux::GetInstance();
#endif
	listener->Start();
}

void UUsbListenerBPLibrary::StopUsbListening()
{
#if PLATFORM_WINDOWS
	auto listener = UsbListenerWindows::GetInstance();
#elif PLATFORM_LINUX
	auto listener = UsbListenerLinux::GetInstance();
#endif
	listener->Stop();
	listener = nullptr;
}