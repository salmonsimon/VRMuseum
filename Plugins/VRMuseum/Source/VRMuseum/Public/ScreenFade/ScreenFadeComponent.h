// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "ScreenFadeComponent.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UCurveFloat;
class UTimelineComponent;

#define FADE_TIMELINE_TICK 0.1f;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMUSEUM_API UScreenFadeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UScreenFadeComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void AttachFaderToCamera(USceneComponent* CameraComponent);

	UFUNCTION(BlueprintCallable)
	void FadeScreen(bool bFadeIn);

	UFUNCTION(BlueprintCallable)
	void SetFadeDurations(float NewFadeInDuration, float NewFadeOutDuration);

protected:
	virtual void BeginPlay() override;

private:

	UFUNCTION()
	void HandleFadeTimelineProgress(float Value);

	void SetFadeTimelineDuration(float NewDuration);

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Screen Fade")
	UStaticMeshComponent* ScreenFadeSphere;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Screen Fade")
	UMaterialInterface* OriginalScreenFadeMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicScreenFadeMaterial;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Screen Fade")
	float FadeInDuration = 1.f;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Screen Fade")
	float FadeOutDuration = 1.f;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Screen Fade")
	UCurveFloat* TimelineFloatCurve = nullptr;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* FadeTimeline;
		
};
