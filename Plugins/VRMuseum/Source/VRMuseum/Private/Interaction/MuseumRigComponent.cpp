// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/MuseumRigComponent.h"

#include "Kismet/GameplayStatics.h"

#include "Rig/IsdkControllerVisualsRigComponent.h"

#include "IsdkControllerMeshComponent.h"
#include "IsdkHandMeshComponent.h"

#include "Rig/IsdkInteractionGroupRigComponent.h"
#include "Utilities/IsdkXRUtils.h"


UMuseumRigComponent::UMuseumRigComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

    RayInteractorSocket = EIsdkHandBones::HandWristRoot;

    ControllerVisuals = CreateDefaultSubobject<UIsdkControllerVisualsRigComponent>(TEXT("ControllerVisuals"));
    ControllerVisuals->HandVisibilityMode = EIsdkRigHandVisibility::Manual;

    ControllerVisuals->GetAnimatedHandMeshComponent()->SetHiddenInGame(true);
    ControllerVisuals->GetControllerMeshComponent()->SetHiddenInGame(true);
    ControllerVisuals->GetPoseableHandMeshComponent()->SetHiddenInGame(true);

    InputActions = CreateDefaultSubobject<UIsdkInputActionsRigComponent>(TEXT("InputActions"));

	RayInteraction = CreateDefaultSubobject<UIsdkRayInteractionRigComponent>(TEXT("RayInteraction"));
	InteractionGroup = CreateDefaultSubobject<UIsdkInteractionGroupRigComponent>(TEXT("InteractionGroup"));
}

void UMuseumRigComponent::CreateInteractionGroupConditionals()
{
    auto CalculateInteractorGroupMemberState = [](const FIsdkInteractorStateEvent& Event)
    {
        FIsdkInteractionGroupMemberState State{};
        if (IsValid(Event.Interactor.GetObject()))
        {
            State.bIsSelectStateBlocking = Event.Interactor->HasSelectedInteractable();
        }
        return State;
    };

    constexpr auto RayInteractorGroupBehavior = FIsdkInteractionGroupMemberBehavior{
      .bDisableOnOtherSelect = true,
      .bDisableOnOtherNearFieldHover = true,
      .bIsNearField = false // false here means a ray wont prevent near-field interactors hovering.
    };
    RayInteraction->GetEnabledConditional()->AddConditional(InteractionGroup->AddInteractor(
        RayInteraction->RayInteractor,
        *RayInteraction->RayInteractor->GetInteractorStateChangedDelegate(),
        CalculateInteractorGroupMemberState,
        RayInteractorGroupBehavior));
}


void UMuseumRigComponent::BindInputActions(UEnhancedInputComponent* EnhancedInputComponent)
{
    // This just ensures we don't call this twice
  // Override this method to do the actual input assignments
    check(!bHasBoundInputActions);
    bHasBoundInputActions = true;

    RayInteraction->BindInputActions(
        EnhancedInputComponent, InputActions->SelectAction, InputActions->SelectStrengthAction);

    if (ControllerVisuals)
    {
        ControllerVisuals->BindInputActions(EnhancedInputComponent, InputActions);
    }

    // Pinch select input mapping is currently not available for OpenXR hands so we use the native
    // finger recognizer events to trigger pinch select/unselect
    if (IsdkXRUtils::IsUsingOpenXR())
    {
        RayInteraction->PinchRecognizer->PinchGrabStarted.AddUniqueDynamic(
            RayInteraction->RayInteractor, &UIsdkRayInteractor::Select);
        RayInteraction->PinchRecognizer->PinchGrabFinished.AddUniqueDynamic(
            RayInteraction->RayInteractor, &UIsdkRayInteractor::Unselect);
    }
}

// TO DO
void UMuseumRigComponent::UpdateComponentDataSources()
{
    const FIsdkTrackingDataSources& DataSources = ControllerVisuals->GetDataSources();

    // UpdateComponentDataSources may be called before data sources have been initialized.
    if (!DataSources.bIsInitialized)
    {
    return;
    }

    FName InteractorAttachSocket = NAME_None;
    USceneComponent* InteractorAttachComponent = nullptr;

    // Wire up data sources to interactors that work with either controllers or hands
    ControllerVisuals->GetInteractorSocket(
        InteractorAttachComponent, InteractorAttachSocket, RayInteractorSocket);

    RayInteraction->BindDataSources(
        DataSources, HmdDataSource, InteractorAttachComponent, InteractorAttachSocket);
}

void UMuseumRigComponent::RegisterInteractorWidgetIndices()
{
    if (GetOwner()->HasActorBegunPlay() || GetOwner()->IsActorBeginningPlay())
    {
        // Registering the interactors with the ISDK Widget Subsystem will allow them to send
        // VirtualUserPointerEvents to Slate Widgets.
        UIsdkWidgetSubsystem& IsdkWidgetSubsystem = UIsdkWidgetSubsystem::Get(GetWorld());
        IsdkWidgetSubsystem.RegisterVirtualUserInfo(GetRayInteractor(), WidgetVirtualUser);
    }
}

void UMuseumRigComponent::UnregisterInteractorWidgetIndices()
{
    UIsdkWidgetSubsystem& IsdkWidgetSubsystem = UIsdkWidgetSubsystem::Get(GetWorld());
    IsdkWidgetSubsystem.UnregisterVirtualUserInfo(GetRayInteractor());
}

void UMuseumRigComponent::SetRigComponentDefaults(EIsdkHandType HandType)
{
    check(HandType != EIsdkHandType::None);

    if (IsValid(ControllerVisuals))
    {
        ControllerVisuals->SetSubobjectPropertyDefaults(HandType);
    }

    InputActions->SetSubobjectPropertyDefaults(HandType);

    if (HandType == EIsdkHandType::HandLeft)
    {
        WidgetVirtualUser.VirtualUserIndex = 0;
    }
    else if (HandType == EIsdkHandType::HandRight)
    {
        WidgetVirtualUser.VirtualUserIndex = 1;
    }
}

void UMuseumRigComponent::OnControllerVisualsAttached()
{
    ControllerVisuals->CreateDataSourcesAsTrackedController();
    UpdateComponentDataSources();
}

void UMuseumRigComponent::InitializeHmdDataSource()
{
    // If our HMD Data Source is good, we're done
    if (IsValid(HmdDataSource.GetObject()))
    {
        return;
    }

    AActor* OwningActor = this->GetAttachParentActor();
    if (!ensureMsgf(
        IsValid(OwningActor),
        TEXT("UIsdkRigComponent::InitializeHmdDataSource() - OwningActor isn't valid!")))
    {
        return;
    }

    TArray<UMuseumRigComponent*> ActorRigComponents;
    OwningActor->GetComponents(UMuseumRigComponent::StaticClass(), ActorRigComponents);

    // Check every other RigComponent on this actor and if any of them already have an HMD Data
    // Source, grab a reference to that
    for (UMuseumRigComponent* RigComponent : ActorRigComponents)
    {
        if (RigComponent == this || !IsValid(RigComponent))
        {
            continue;
        }
        if (RigComponent->GetHmdDataSource(HmdDataSource))
        {
            break;
        }
    }

    // Our search was in vain, set up an HMD Data Source ourselves
    if (!IsValid(HmdDataSource.GetObject()))
    {
        const auto TrackingDataSubsystem = GetControllerVisuals()->GetTrackingDataSubsystem();
        if (ensureMsgf(
            IsValid(TrackingDataSubsystem.GetObject()),
            TEXT(
                "UIsdkRigComponent::InitializeHmdDataSource() - Could not get the IsdkTrackingDataSubsystem.")))
        {
            HmdDataSource = IIsdkITrackingDataSubsystem::Execute_CreateHmdDataSourceComponent(
                TrackingDataSubsystem.GetObject(), GetOwner());
        }
    }
}

bool UMuseumRigComponent::GetHmdDataSource(TScriptInterface<IIsdkIHmdDataSource>& HmdDataSourceOut) const
{
    HmdDataSourceOut = HmdDataSource;
    return IsValid(HmdDataSource.GetObject());
}

void UMuseumRigComponent::BeginPlay()
{
	Super::BeginPlay();

    // Subscribe to input action events
    if (bAutoBindInputActions)
    {
        const auto FirstLocalPlayer = UGameplayStatics::GetPlayerController(this, 0);

        AActor* OwnerActor = GetOwner();
        OwnerActor->EnableInput(FirstLocalPlayer);
        const auto EnhancedInputComponent = Cast<UEnhancedInputComponent>(OwnerActor->InputComponent);
        if (ensureMsgf(
            IsValid(EnhancedInputComponent),
            TEXT("IsdkRigHand: Could not create UEnhancedInputComponent.")))
        {
            BindInputActions(EnhancedInputComponent);
        }
    }

    // Register interactors to the widget system
    RegisterInteractorWidgetIndices();

    // Setup interaction groups
    CreateInteractionGroupConditionals();
}

void UMuseumRigComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    UnregisterInteractorWidgetIndices();
}


void UMuseumRigComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bAreDataSourcesReady)
    {
        if (!IsValid(HmdDataSource.GetObject()))
        {
            InitializeHmdDataSource();
        }

        if (IsValid(ControllerVisuals) && !IsValid(ControllerVisuals->AttachedToMotionController))
        {
            if (ControllerVisuals->TryAttachToParentMotionController(this))
            {
                OnControllerVisualsAttached();
            }
        }

        bAreDataSourcesReady = IsValid(HmdDataSource.GetObject());
        if (bAreDataSourcesReady)
        {
            // Tell interested parties that data sources are available.
            DataSourcesReady.Broadcast();
        }
    }
}

