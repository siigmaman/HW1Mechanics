#include "HWBTTask_MoveToBlackboard.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UHWBTTask_MoveToBlackboard::UHWBTTask_MoveToBlackboard()
{
    NodeName = TEXT("Move To Blackboard Value");
    bNotifyTick = true;
}

EBTNodeResult::Type UHWBTTask_MoveToBlackboard::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Controller || !Blackboard)
    {
        return EBTNodeResult::Failed;
    }

    EPathFollowingRequestResult::Type Result = EPathFollowingRequestResult::Failed;
    if (AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKeyName)))
    {
        Result = Controller->MoveToActor(TargetActor, AcceptanceRadius, true, true, true);
    }
    else
    {
        const FVector TargetLocation = Blackboard->GetValueAsVector(BlackboardKeyName);
        Result = Controller->MoveToLocation(
            TargetLocation, AcceptanceRadius, true, true, true, false);
    }

    if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        return EBTNodeResult::Succeeded;
    }
    return Result == EPathFollowingRequestResult::RequestSuccessful
        ? EBTNodeResult::InProgress
        : EBTNodeResult::Failed;
}

EBTNodeResult::Type UHWBTTask_MoveToBlackboard::AbortTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AAIController* Controller = OwnerComp.GetAIOwner())
    {
        Controller->StopMovement();
    }
    return EBTNodeResult::Aborted;
}

void UHWBTTask_MoveToBlackboard::TickTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    const AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
