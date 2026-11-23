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
        UE_LOG(LogTemp, Warning, TEXT("HW2: flee task missing pawn, threat, blackboard or navigation"));
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
        UE_LOG(LogTemp, Warning, TEXT("HW2: flee destination could not be projected to NavMesh"));
        return EBTNodeResult::Failed;
    }

    Blackboard->SetValueAsVector(FleeLocationKey, ProjectedLocation.Location);
    UE_LOG(
        LogTemp, Display, TEXT("HW2: flee destination set to %s"),
        *ProjectedLocation.Location.ToCompactString());
    return EBTNodeResult::Succeeded;
}
