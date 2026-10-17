#include "HWCharacter.h"

#include "EnhancedInputComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"

AHWCharacter::AHWCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AHWCharacter::BeginPlay()
{
    Super::BeginPlay();

    WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    Stamina = MaxStamina;
    OnInteractionTriggered.AddDynamic(this, &AHWCharacter::RelayInteractionToBlueprint);
}

void AHWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (SprintAction)
        {
            Input->BindAction(SprintAction, ETriggerEvent::Started, this, &AHWCharacter::StartSprint);
            Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHWCharacter::StopSprint);
            Input->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AHWCharacter::StopSprint);
        }

        if (InteractAction)
        {
            Input->BindAction(InteractAction, ETriggerEvent::Started, this, &AHWCharacter::Interact);
        }
    }
}

void AHWCharacter::StartSprint()
{
    if (!bExhausted && Stamina > 0.0f)
    {
        SetSprinting(true);
    }
}

void AHWCharacter::StopSprint()
{
    SetSprinting(false);
}

void AHWCharacter::SetSprinting(bool bEnabled)
{
    bIsSprinting = bEnabled;
    GetCharacterMovement()->MaxWalkSpeed = bEnabled ? SprintSpeed : WalkSpeed;
}

void AHWCharacter::Interact()
{
    OnInteractionTriggered.Broadcast();
}

void AHWCharacter::RelayInteractionToBlueprint()
{
    OnInteractionTriggeredInBlueprint();
}

void AHWCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsSprinting)
    {
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrainPerSecond * DeltaSeconds);
        if (Stamina <= 0.0f)
        {
            bExhausted = true;
            SetSprinting(false);
        }
    }
    else
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRecoveryPerSecond * DeltaSeconds);
        if (Stamina >= RecoveryThreshold)
        {
            bExhausted = false;
        }
    }

    if (IsLocallyControlled() && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            41231, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Stamina: %.0f / %.0f"), Stamina, MaxStamina));
    }
}
