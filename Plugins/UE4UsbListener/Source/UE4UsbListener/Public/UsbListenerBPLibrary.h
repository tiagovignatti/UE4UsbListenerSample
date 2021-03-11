// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UsbListenerBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UE4USBLISTENER_API UUsbListenerBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Start monitoring for USB device hotplugging
	UFUNCTION(BlueprintCallable, Category = "UsbListener")
	static void StartUsbListening();

	// Stop monitoring
	UFUNCTION(BlueprintCallable, Category = "UsbListener")
	static void StopUsbListening();
};
