// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/VRMuseumFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"

#include "HeadMountedDisplayFunctionLibrary.h"

FTransform UVRMuseumFunctionLibrary::GetHeadPose(UObject* WorldContextObject)
{
	FRotator Rotation;
	FVector Position;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(Rotation, Position);

	FTransform TrackingSpaceTransform(Rotation, Position);
	FTransform TrackingToWorld = UHeadMountedDisplayFunctionLibrary::GetTrackingToWorldTransform(WorldContextObject);

	FTransform Result;
	FTransform::Multiply(&Result, &TrackingSpaceTransform, &TrackingToWorld);

	return Result;
}

FVector UVRMuseumFunctionLibrary::Slerp(const FVector& Vector1, const FVector& Vector2, const float Slerp)
{
	FVector Vector1Dir, Vector2Dir;
	float Vector1Size, Vector2Size;
	Vector1.ToDirectionAndLength(Vector1Dir, Vector1Size);
	Vector2.ToDirectionAndLength(Vector2Dir, Vector2Size);
	float Dot = FVector::DotProduct(Vector1Dir, Vector2Dir);

	float Scale1, Scale2;
	if (Dot < 0.9999f)
	{
		const float Omega = FMath::Acos(Dot);
		const float InvSin = 1.f / FMath::Sin(Omega);
		Scale1 = FMath::Sin((1.f - Slerp) * Omega) * InvSin;
		Scale2 = FMath::Sin(Slerp * Omega) * InvSin;
	}
	else
	{
		// Use linear interpolation.
		Scale1 = 1.0f - Slerp;
		Scale2 = Slerp;
	}

	FVector ResultDir = Vector1Dir * Scale1 + Vector2Dir * Scale2;
	float ResultSize = FMath::Lerp(Vector1Size, Vector2Size, Slerp);

	return ResultDir * ResultSize;
}
