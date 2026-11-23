#include "HWCharacter.h"

#include "EnhancedInputComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HWAICharacter.h"
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
    TryInteract();
}

bool AHWCharacter::IsWithinInteractionRange(const AActor* Target) const
{
    return IsValid(Target) && FVector::DistSquared(GetActorLocation(), Target->GetActorLocation())
        <= FMath::Square(InteractionRange);
}

bool AHWCharacter::CanInteract() const
{
    if (!GetWorld()) { return false; }
    for (TActorIterator<AHWAICharacter> Enemy(GetWorld()); Enemy; ++Enemy)
    {
        if (IsWithinInteractionRange(*Enemy)) { return true; }
    }
    return false;
}

bool AHWCharacter::TryInteract()
{
    if (!CanInteract())
    {
        if (IsLocallyControlled() && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(41233, 1.5f, FColor::Silver,
                FString::Printf(TEXT("Move closer to AI (within %.0f m)"), InteractionRange / 100.0f));
        }
        return false;
    }
    OnInteractionTriggered.Broadcast();
    return true;
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
        if (CanInteract())
        {
            GEngine->AddOnScreenDebugMessage(41232, 0.0f, FColor::Green, TEXT("E - Scare AI"));
        }
        else
        {
            GEngine->RemoveOnScreenDebugMessage(41232);
        }
        GEngine->AddOnScreenDebugMessage(
            41231, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Stamina: %.0f / %.0f"), Stamina, MaxStamina));
    }
}
