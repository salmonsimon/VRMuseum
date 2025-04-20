// Fill out your copyright notice in the Description page of Project Settings.


#include "ScreenFade/ScreenFadeComponent.h"

#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Components/TimelineComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include "Engine/World.h"

UScreenFadeComponent::UScreenFadeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	FadeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("FadeTimeline"));

	ScreenFadeSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenFadeSphere"));
	ScreenFadeSphere->SetMaterial(0, OriginalScreenFadeMaterial);
	ScreenFadeSphere->bUseAsOccluder = true;
}


void UScreenFadeComponent::BeginPlay()
{
	Super::BeginPlay();

	DynamicScreenFadeMaterial = UMaterialInstanceDynamic::Create(OriginalScreenFadeMaterial, this);

	if (ScreenFadeSphere)
	{
		ScreenFadeSphere->SetMaterial(0, DynamicScreenFadeMaterial);

		ScreenFadeSphere->SetOnlyOwnerSee(true);
	}

	if (FadeTimeline)
	{
		FadeTimeline->PrimaryComponentTick.TickInterval = FADE_TIMELINE_TICK;
		FadeTimeline->PrimaryComponentTick.bCanEverTick = false;

		FOnTimelineFloat FadeTimelineProgress;
		FadeTimelineProgress.BindUFunction(this, FName("HandleFadeTimelineProgress"));

		FadeTimeline->AddInterpFloat(TimelineFloatCurve, FadeTimelineProgress);
		FadeTimeline->SetLooping(false);

		FadeTimeline->RegisterComponent();
	}
}

void UScreenFadeComponent::AttachFaderToCamera(USceneComponent* CameraComponent)
{
	if (CameraComponent)
	{
		ScreenFadeSphere->RegisterComponent();

		ScreenFadeSphere->SetWorldTransform(CameraComponent->GetComponentTransform());
		ScreenFadeSphere->AttachToComponent(CameraComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

void UScreenFadeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FadeTimeline)
		FadeTimeline->TickComponent(DeltaTime, LEVELTICK_TimeOnly, nullptr);
}

void UScreenFadeComponent::FadeScreen(bool bFadeIn)
{
	if (FadeTimeline == nullptr)
		return;

	if (bFadeIn)
	{
		SetFadeTimelineDuration(FadeInDuration);

		FadeTimeline->PlayFromStart();
	}
	else
	{
		SetFadeTimelineDuration(FadeOutDuration);

		FadeTimeline->ReverseFromEnd();
	}
}

void UScreenFadeComponent::SetFadeDurations(float NewFadeInDuration, float NewFadeOutDuration)
{
	FadeInDuration = NewFadeInDuration;
	FadeOutDuration = NewFadeOutDuration;
}

void UScreenFadeComponent::HandleFadeTimelineProgress(float Value)
{
	if (ScreenFadeSphere)
		ScreenFadeSphere->SetScalarParameterValueOnMaterials(FName("Opacity"), Value);
}

void UScreenFadeComponent::SetFadeTimelineDuration(float NewDuration)
{
	if (FadeTimeline == nullptr)
		return;

	float NewPlayRate = UKismetMathLibrary::SafeDivide(FadeTimeline->GetTimelineLength(), NewDuration);

	FadeTimeline->SetPlayRate(NewPlayRate);
}

