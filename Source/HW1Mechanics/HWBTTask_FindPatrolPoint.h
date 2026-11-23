#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "HWBTTask_FindPatrolPoint.generated.h"

UCLASS()
class HW1MECHANICS_API UHWBTTask_FindPatrolPoint : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UHWBTTask_FindPatrolPoint();

    UPROPERTY(EditAnywhere, Category = "Patrol")
    FName HomeLocationKey = TEXT("HomeLocation");

    UPROPERTY(EditAnywhere, Category = "Patrol")
    FName PatrolLocationKey = TEXT("PatrolLocation");

    UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "100.0"))
    float PatrolRadius = 1200.0f;

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
