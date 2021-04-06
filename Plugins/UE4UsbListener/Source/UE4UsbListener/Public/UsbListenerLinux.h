// Fill out your copyright notice in the Description page of Project Settings.
#if PLATFORM_LINUX
#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"

DEFINE_LOG_CATEGORY_STATIC(UE4UsbListenerLinux, Log, All);

class UE4USBLISTENER_API UsbListenerLinux
{
public:
	typedef std::function<void(const std::string& deviceName, bool plugOn)> UsbDeviceChangeCallback;
	typedef std::function<void(const std::string& deviceName)> UsbDeviceQueryCallback;
private:
	void MonitorDevices(int signal_fd, struct udev* udev);

	static std::shared_ptr<UsbListenerLinux> instance;
	static std::mutex instanceMutex;

	/** Unarchive task result */
	TOptional<TFuture<void>> ListenerTask;
protected:
	bool init;
	UsbDeviceChangeCallback usbDeviceChangeCallback;
	UsbDeviceQueryCallback usbDeviceQueryCallback;

	UsbListenerLinux();
	UsbListenerLinux(const UsbListenerLinux&) = delete;
public:
	static std::shared_ptr<UsbListenerLinux> GetInstance();
	virtual ~UsbListenerLinux();
	void SetDeviceChangeCallback(UsbDeviceChangeCallback callback);
	void SetDeviceQueryCallback(UsbDeviceQueryCallback callback);
	virtual bool Start();
	virtual void Stop();
};
#endif