#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "HWBTDecorator_BlackboardState.generated.h"

UCLASS()
class HW1MECHANICS_API UHWBTDecorator_BlackboardState : public UBTDecorator_BlackboardBase
{
    GENERATED_BODY()

public:
    UHWBTDecorator_BlackboardState();
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

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
