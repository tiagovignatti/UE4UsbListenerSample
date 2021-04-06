#if PLATFORM_LINUX
#include "UsbListenerLinux.h"

#include <unistd.h>
#include <poll.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/signalfd.h>
#include <csignal>

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

void scanDevices(struct udev* udev) {
    struct udev_device* device;
    struct udev_enumerate* enumerate;
    struct udev_list_entry* devices, * dev_list_entry;

    // Create enumerate object
    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while creating udev enumerate"));
        return;
    }

    // Scan devices
    udev_enumerate_scan_devices(enumerate);

    // Fill up device list
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices) {
        UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while getting device list"));
        return;
    }

    udev_list_entry_foreach(dev_list_entry, devices) {
        // Get the device
        device = udev_device_new_from_syspath(udev, udev_list_entry_get_name(dev_list_entry));
        // Print device information
        UE_LOG(UE4UsbListenerLinux, Log, TEXT("DEVNODE: %s"), udev_device_get_devnode(device));
        UE_LOG(UE4UsbListenerLinux, Log, TEXT("KERNEL: %s"), udev_device_get_sysname(device));
        UE_LOG(UE4UsbListenerLinux, Log, TEXT("DEVPATH: %s"), udev_device_get_devpath(device));
        UE_LOG(UE4UsbListenerLinux, Log, TEXT("DEVTYPE: %s"), udev_device_get_devtype(device));
        // Free the device
        udev_device_unref(device);
    }
    // Free enumerate
    udev_enumerate_unref(enumerate);
}

void UsbListenerLinux::MonitorDevices(int signal_fd, struct udev* udev) {
    if (ListenerTask.IsSet())
    {
        UE_LOG(UE4UsbListenerLinux, Error, TEXT("Implementation error: ListenerTask already set"));
        return;
    }

    ListenerTask = Async(EAsyncExecution::Thread, [this, signal_fd, udev] {
        udev_monitor* monitor = udev_monitor_new_from_netlink(udev, "udev");
        struct pollfd pfd[2];
        int ret_poll;
        ssize_t  n;

        // Enable receiving hotplug events
        udev_monitor_enable_receiving(monitor);

        pfd[0].events = POLLIN;
        pfd[0].fd = signal_fd;
        pfd[1].events = POLLIN;
        pfd[1].fd = udev_monitor_get_fd(monitor);
        if (pfd[1].fd < 0) {
            UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while getting hotplug monitor"));
            udev_monitor_unref(monitor);
            return;
        }

        while (true) {
            // Wait for events without time limit
            ret_poll = poll(pfd, 2, -1);
            if (ret_poll < 0) {
                UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while polling file descriptors"));
                break;
            }
            // True, if a signal from the operating system was sent to this process
            if (pfd[0].revents & POLLIN) {
                struct signalfd_siginfo signal_info;
                // Get the signal
                n = read(pfd[0].fd, &signal_info, sizeof(signal_info));
                // True, if an error occurred while getting the signal
                if (n == -1) {
                    UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while read on signal file descriptor"));
                    break;
                }
                // Check which signal was caught
                switch (signal_info.ssi_signo) {
                case SIGINT:
                    UE_LOG(UE4UsbListenerLinux, Log, TEXT("SIGINT received"));
                    break;

                case SIGTERM:
                    UE_LOG(UE4UsbListenerLinux, Log, TEXT("SIGTERM received"));
                    break;

                default:
                    UE_LOG(UE4UsbListenerLinux, Log, TEXT("Unknown signal received"));
                }
                break;
            }
            if (pfd[1].revents & POLLIN) {
                // Get the device
                struct udev_device* device = udev_monitor_receive_device(monitor);
                if (!device) {
                    UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while getting device...returning to work"));
                    continue;
                }
                // Print device information
                UE_LOG(UE4UsbListenerLinux, Log, TEXT("DEVNODE: %s"), udev_device_get_devnode(device));
                UE_LOG(UE4UsbListenerLinux, Log, TEXT("KERNEL: %s"), udev_device_get_sysname(device));
                UE_LOG(UE4UsbListenerLinux, Log, TEXT("DEVPATH: %s"), udev_device_get_devpath(device));
                UE_LOG(UE4UsbListenerLinux, Log, TEXT("DEVTYPE: %s"), udev_device_get_devtype(device));

                // Free the device
                udev_device_unref(device);
            }
        }
        // Free the monitor
        udev_monitor_unref(monitor);
    });
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
	UE_LOG(UE4UsbListenerLinux, Log, TEXT("Start"));
	if (init)
	{
		return true;
	}

    ListenerTask = TOptional<TFuture<void>>();

	// Create a new udev object
	struct udev* udev = udev_new();
	if (!udev) {
        UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while initialization!"));
		return EXIT_FAILURE;
	}

	sigset_t mask;

	// Set signals we want to catch
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGINT);

	// Change the signal mask and check
	if (sigprocmask(SIG_BLOCK, &mask, nullptr) < 0) {
        UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while sigprocmask(): %s"), std::strerror(errno));
		return EXIT_FAILURE;
	}
	// Get a signal file descriptor
	int signal_fd = signalfd(-1, &mask, 0);
	// Check the signal file descriptor
	if (signal_fd < 0) {
        UE_LOG(UE4UsbListenerLinux, Error, TEXT("Error while signalfd(): %s"), std::strerror(errno));
		return EXIT_FAILURE;
	}
	// First scan already attached devices
	scanDevices(udev);
	// Second monitor hotplug events
	MonitorDevices(signal_fd, udev);
	// Free the udev object
	udev_unref(udev);

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