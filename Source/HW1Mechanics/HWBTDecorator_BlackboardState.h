#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "HWBTDecorator_BlackboardState.generated.h"

UCLASS()
class HW1MECHANICS_API UHWBTDecorator_BlackboardState : public UBTDecorator
{
    GENERATED_BODY()

public:
    UHWBTDecorator_BlackboardState();

    UPROPERTY(EditAnywhere, Category = "Condition")
    FName BlackboardKeyName = TEXT("TargetActor");

    UPROPERTY(EditAnywhere, Category = "Condition")
    bool bTreatAsBoolean = false;

    UPROPERTY(EditAnywhere, Category = "Condition")
    bool bExpectedValue = true;

protected:
    virtual bool CalculateRawConditionValue(
        UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
    virtual FString GetStaticDescription() const override;
};
