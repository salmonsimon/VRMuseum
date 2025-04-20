// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "VRMuseumFunctionLibrary.generated.h"

UCLASS(ClassGroup = VRMuseum)
class VRMUSEUM_API UVRMuseumFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "VRMuseum|Utils", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static FTransform GetHeadPose(UObject* WorldContextObject);

	/** Spherical linear interpolate between two vectors */
	UFUNCTION(BlueprintPure, Category = "VRMuseum|Utils")
	static FVector Slerp(const FVector& Vector1, const FVector& Vector2, const float Slerp);
	
};
