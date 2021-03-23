// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Windows/WindowsApplication.h"
#include <mutex>

class UE4USBLISTENER_API UsbListener : public IWindowsMessageHandler
{
public:
	typedef std::function<void(const std::string& deviceName, bool plugOn)> UsbDeviceChangeCallback;
	typedef std::function<void(const std::string& deviceName)> UsbDeviceQueryCallback;
private:
	static std::shared_ptr<UsbListener> instance;
	static std::mutex instanceMutex;
protected:
	bool init;
	UsbDeviceChangeCallback usbDeviceChangeCallback;
	UsbDeviceQueryCallback usbDeviceQueryCallback;

	UsbListener();
	UsbListener(const UsbListener&) = delete;
public:
	static std::shared_ptr<UsbListener> GetInstance();
	virtual ~UsbListener();
	void SetDeviceChangeCallback(UsbDeviceChangeCallback callback);
	void SetDeviceQueryCallback(UsbDeviceQueryCallback callback);
	virtual bool Start();
	virtual void Stop();
	int64_t HandleHotplugMessage(void* hwndPtr, uint32_t uint, uint64_t wparam, int64_t lparam);

	// Begin IWindowsMessageHandler interface
	virtual bool ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult) override;
	// End IWindowsMessageHandler interface
};

DEFINE_LOG_CATEGORY_STATIC(UE4UsbListener, Log, All);