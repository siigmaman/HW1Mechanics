#include "HWBTDecorator_BlackboardState.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UHWBTDecorator_BlackboardState::UHWBTDecorator_BlackboardState()
{
    NodeName = TEXT("Check AI State");
    FlowAbortMode = EBTFlowAbortMode::Both;
}

bool UHWBTDecorator_BlackboardState::CalculateRawConditionValue(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        return false;
    }

    const bool bActualValue = bTreatAsBoolean
        ? Blackboard->GetValueAsBool(BlackboardKeyName)
        : Blackboard->GetValueAsObject(BlackboardKeyName) != nullptr;
    return bActualValue == bExpectedValue;
}

FString UHWBTDecorator_BlackboardState::GetStaticDescription() const
{
    return FString::Printf(
        TEXT("%s is %s"), *BlackboardKeyName.ToString(),
        bExpectedValue ? TEXT("set/true") : TEXT("unset/false"));
}
