#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "HWBTTask_SetFleeLocation.generated.h"

UCLASS()
class HW1MECHANICS_API UHWBTTask_SetFleeLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UHWBTTask_SetFleeLocation();

    UPROPERTY(EditAnywhere, Category = "Flee")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditAnywhere, Category = "Flee")
    FName FleeLocationKey = TEXT("FleeLocation");

    UPROPERTY(EditAnywhere, Category = "Flee", meta = (ClampMin = "100.0"))
    float FleeDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Flee", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float DirectionJitterDegrees = 25.0f;

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
