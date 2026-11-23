#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HWCharacter.generated.h"

class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionTriggered);

UCLASS()
class HW1MECHANICS_API AHWCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AHWCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool TryInteract();

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool CanInteract() const;

    bool IsWithinInteractionRange(const AActor* Target) const;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractionTriggered OnInteractionTriggered;

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionTriggeredInBlueprint();

    UPROPERTY(BlueprintReadOnly, Category = "Sprint")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Sprint")
    float Stamina = 100.0f;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0", Units = "cm"))
    float InteractionRange = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
    float SprintSpeed = 900.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
    float MaxStamina = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
    float StaminaDrainPerSecond = 25.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
    float StaminaRecoveryPerSecond = 15.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
    float RecoveryThreshold = 25.0f;

private:
    float WalkSpeed = 0.0f;
    bool bExhausted = false;

    void StartSprint();
    void StopSprint();
    void Interact();
    void SetSprinting(bool bEnabled);

    UFUNCTION()
    void RelayInteractionToBlueprint();
};
