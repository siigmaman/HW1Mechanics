#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "HWBTTask_MoveToBlackboard.generated.h"

UCLASS()
class HW1MECHANICS_API UHWBTTask_MoveToBlackboard : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UHWBTTask_MoveToBlackboard();

    UPROPERTY(EditAnywhere, Category = "Move")
    FName BlackboardKeyName = TEXT("PatrolLocation");

    UPROPERTY(EditAnywhere, Category = "Move", meta = (ClampMin = "0.0"))
    float AcceptanceRadius = 120.0f;

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(
        UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
