#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "HWAIController.generated.h"

class AHWCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;
class UBehaviorTreeComponent;

UCLASS()
class HW1MECHANICS_API AHWAIController : public AAIController
{
    GENERATED_BODY()

public:
    AHWAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<UAIPerceptionComponent> SightPerception;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Flee")
    float InteractionReactionRadius = 1400.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Flee")
    float FleeDuration = 4.0f;

    TWeakObjectPtr<AHWCharacter> BoundPlayer;
    bool bPlayerVisible = false;
    FTimerHandle FleeTimerHandle;
    FTimerHandle BindRetryTimerHandle;

    UFUNCTION()
    void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void HandlePlayerInteraction();

    void BindToPlayerInteraction();
    void ClearFleeState();
    void SetTargetActor(AActor* TargetActor);
};
