#include "HWBTTask_SetFleeLocation.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UHWBTTask_SetFleeLocation::UHWBTTask_SetFleeLocation()
{
    NodeName = TEXT("Compute Flee Location (Custom)");
}

EBTNodeResult::Type UHWBTTask_SetFleeLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
    AActor* Threat = Blackboard
        ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey))
        : nullptr;
    UNavigationSystemV1* Navigation = Pawn
        ? UNavigationSystemV1::GetCurrent(Pawn->GetWorld())
        : nullptr;
    if (!Pawn || !Threat || !Blackboard || !Navigation)
    {
        return EBTNodeResult::Failed;
    }

    FVector Away = Pawn->GetActorLocation() - Threat->GetActorLocation();
    Away.Z = 0.0f;
    if (!Away.Normalize())
    {
        Away = Pawn->GetActorForwardVector();
    }

    const float Jitter = FMath::FRandRange(-DirectionJitterDegrees, DirectionJitterDegrees);
    Away = Away.RotateAngleAxis(Jitter, FVector::UpVector);
    const FVector DesiredLocation = Pawn->GetActorLocation() + Away * FleeDistance;

    FNavLocation ProjectedLocation;
    if (!Navigation->ProjectPointToNavigation(
            DesiredLocation, ProjectedLocation, FVector(500.0f, 500.0f, 300.0f)))
    {
        return EBTNodeResult::Failed;
    }

    Blackboard->SetValueAsVector(FleeLocationKey, ProjectedLocation.Location);
    return EBTNodeResult::Succeeded;
}
