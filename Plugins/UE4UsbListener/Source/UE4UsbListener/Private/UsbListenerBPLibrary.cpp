// Fill out your copyright notice in the Description page of Project Settings.


#include "UsbListenerBPLibrary.h"

#include "UsbListener.h"

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
	auto listener = UsbListener::GetInstance();
	listener->SetDeviceQueryCallback(DeviceQueryCallback);
	listener->SetDeviceChangeCallback(Callback);
	listener->Start();
}

void UUsbListenerBPLibrary::StopUsbListening()
{
	auto listener = UsbListener::GetInstance();
	listener->Stop();
	listener = nullptr;
}