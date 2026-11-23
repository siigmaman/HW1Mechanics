#include "HWBTTask_FindPatrolPoint.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UHWBTTask_FindPatrolPoint::UHWBTTask_FindPatrolPoint()
{
    NodeName = TEXT("Find Random Patrol Point");
}

EBTNodeResult::Type UHWBTTask_FindPatrolPoint::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
    UNavigationSystemV1* Navigation = Pawn
        ? UNavigationSystemV1::GetCurrent(Pawn->GetWorld())
        : nullptr;
    if (!Pawn || !Blackboard || !Navigation)
    {
        return EBTNodeResult::Failed;
    }

    FVector Home = Blackboard->GetValueAsVector(HomeLocationKey);
    if (Home.ContainsNaN())
    {
        Home = Pawn->GetActorLocation();
    }

    FNavLocation PatrolPoint;
    if (!Navigation->GetRandomReachablePointInRadius(Home, PatrolRadius, PatrolPoint))
    {
        return EBTNodeResult::Failed;
    }

    Blackboard->SetValueAsVector(PatrolLocationKey, PatrolPoint.Location);
    return EBTNodeResult::Succeeded;
}
