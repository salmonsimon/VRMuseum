// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "Rig/IsdkRayInteractionRigComponent.h"
#include "Interaction/IsdkRayInteractor.h"

#include "Rig/IsdkInputActionsRigComponent.h"

#include "Subsystem/IsdkWidgetSubsystem.h"

#include "MuseumRigComponent.generated.h"

class UIsdkRayInteractionRigComponent;
class UIsdkInputActionsRigComponent;
class UIsdkControllerVisualsRigComponent;
class UEnhancedInputComponent;
class IIsdkIHmdDataSource;
enum class EIsdkHandBones : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMuseumRigComponentLifecycleEvent);

UCLASS(Abstract, Blueprintable, ClassGroup = ("MuseumInteraction|Rig"), meta = (BlueprintSpawnableComponent, DisplayName = "Museum Rig Component"))
class VRMUSEUM_API UMuseumRigComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UMuseumRigComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	void CreateInteractionGroupConditionals();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * @brief Binds input actions used by the interactors to the given component. This method will not
	 * undo any previous bindings, therefore should only be called once. If you intend to call this,
	 * make sure to set `bAutoBindInputActions` to false.
	 */
	virtual void BindInputActions(UEnhancedInputComponent* EnhancedInputComponent);

	/**
   * @brief When true, during BeginPlay this actor will bind the configured input actions to the
   * PlayerController at index 0.
   * If false, a manual call to BindInputActionEvents must be made to bind the input actions.
   */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
	bool bAutoBindInputActions = true;

	UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
	FMuseumRigComponentLifecycleEvent DataSourcesReady;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "InteractionSDK|Customization")
	EIsdkHandBones RayInteractorSocket;

	UPROPERTY(BlueprintGetter = GetControllerVisuals, EditAnywhere, Category = InteractionSDK)
	TObjectPtr<UIsdkControllerVisualsRigComponent> ControllerVisuals;

protected:

	virtual void UpdateComponentDataSources();
	
	virtual void RegisterInteractorWidgetIndices();
	virtual void UnregisterInteractorWidgetIndices();

	void SetRigComponentDefaults(EIsdkHandType HandType);

	virtual void OnControllerVisualsAttached();

	// Attempts to find another already created HmdDataSource on other RigComponents. If unable,
	// creates one. Sets either result to member variable.
	void InitializeHmdDataSource();

	UPROPERTY(BlueprintGetter = GetRayInteraction, EditAnywhere, Category = InteractionSDK)
	UIsdkRayInteractionRigComponent* RayInteraction;

	UPROPERTY(BlueprintGetter = GetInputActions, EditAnywhere, Category = InteractionSDK)
	TObjectPtr<UIsdkInputActionsRigComponent> InputActions;

	UPROPERTY(BlueprintGetter = GetInteractionGroup, EditAnywhere, Category = InteractionSDK)
	TObjectPtr<UIsdkInteractionGroupRigComponent> InteractionGroup;

	bool bHasBoundInputActions = false;
	bool bAreDataSourcesReady = false;

	UPROPERTY(
		BlueprintGetter = GetWidgetVirtualUser,
		BlueprintSetter = SetWidgetVirtualUser,
		EditAnywhere,
		Category = InteractionSDK,
		meta = (ExposeOnSpawn = true))
	FIsdkVirtualUserInfo WidgetVirtualUser;

	UPROPERTY()
	TScriptInterface<IIsdkIHmdDataSource> HmdDataSource;

public:	
		
	UFUNCTION(BlueprintGetter, Category = InteractionSDK)
	UIsdkRayInteractor* GetRayInteractor() const
	{
		return RayInteraction->RayInteractor;
	}

	UFUNCTION(BlueprintGetter, Category = InteractionSDK)
	UIsdkRayInteractionRigComponent* GetRayInteraction() const
	{
		return RayInteraction;
	}
	
	UFUNCTION(BlueprintGetter, Category = InteractionSDK)
	UIsdkInteractionGroupRigComponent* GetInteractionGroup() const
	{
		return InteractionGroup;
	}

	UFUNCTION(BlueprintGetter, Category = InteractionSDK)
	const FIsdkVirtualUserInfo& GetWidgetVirtualUser() const
	{
		return WidgetVirtualUser;
	}

	UFUNCTION(BlueprintSetter, Category = InteractionSDK)
	void SetWidgetVirtualUser(const FIsdkVirtualUserInfo& InWidgetVirtualUser)
	{
		WidgetVirtualUser = InWidgetVirtualUser;
	}

	UFUNCTION(BlueprintGetter, Category = InteractionSDK)
	UIsdkInputActionsRigComponent* GetInputActions() const
	{
		return InputActions;
	}

	/* Returns true if HMD DataSource is valid and returns reference to it */
	UFUNCTION(BlueprintCallable, Category = InteractionSDK)
	bool GetHmdDataSource(TScriptInterface<IIsdkIHmdDataSource>& HmdDataSourceOut) const;

	UFUNCTION(BlueprintGetter, Category = InteractionSDK)
	UIsdkControllerVisualsRigComponent* GetControllerVisuals() const
	{
		return ControllerVisuals;
	}


};

UCLASS(
	Blueprintable,
	ClassGroup = ("InteractionSDK|Rig"),
	meta = (BlueprintSpawnableComponent, DisplayName = "Museum Hand Rig (L)"))
	class VRMUSEUM_API UMuseumHandRigComponentLeft : public UMuseumRigComponent
{
	GENERATED_BODY()

public:
	UMuseumHandRigComponentLeft(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("ControllerVisuals")))
	{
		SetRigComponentDefaults(EIsdkHandType::HandLeft);
	}

};

UCLASS(
	Blueprintable,
	ClassGroup = ("InteractionSDK|Rig"),
	meta = (BlueprintSpawnableComponent, DisplayName = "Museum Hand Rig (R)"))
	class VRMUSEUM_API UMuseumHandRigComponentRight : public UMuseumRigComponent
{
	GENERATED_BODY()

public:
	UMuseumHandRigComponentRight(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("ControllerVisuals")))
	{
		SetRigComponentDefaults(EIsdkHandType::HandRight);
	}

};
