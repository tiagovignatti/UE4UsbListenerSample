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

	// Convert FPortConfig to FString
	UFUNCTION(BlueprintPure)
	static FString ConvertPortConfigToString(int ComNumber);
};
