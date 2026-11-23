#include "HWAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "HWCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"

namespace HWAIKeys
{
const FName TargetActor(TEXT("TargetActor"));
const FName HomeLocation(TEXT("HomeLocation"));
const FName IsFleeing(TEXT("IsFleeing"));
}

AHWAIController::AHWAIController()
{
    Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTree"));
    BrainComponent = BehaviorTreeComponent;

    SightPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("SightPerception"));
    SetPerceptionComponent(*SightPerception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1900.0f;
    SightConfig->PeripheralVisionAngleDegrees = 80.0f;
    SightConfig->SetMaxAge(2.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightPerception->ConfigureSense(*SightConfig);
    SightPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    SightPerception->OnTargetPerceptionUpdated.AddDynamic(
        this, &AHWAIController::HandleTargetPerceptionUpdated);
}

void AHWAIController::BeginPlay()
{
    Super::BeginPlay();
    BindToPlayerInteraction();
}

void AHWAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!BehaviorTreeAsset)
    {
        BehaviorTreeAsset = LoadObject<UBehaviorTree>(
            nullptr, TEXT("/Game/HW2/AI/BT_Enemy.BT_Enemy"));
    }

    if (!BehaviorTreeAsset || !BehaviorTreeAsset->BlackboardAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("HW2: Behavior Tree or Blackboard asset is missing"));
        return;
    }

    UBlackboardComponent* InitializedBlackboard = Blackboard;
    if (!UseBlackboard(BehaviorTreeAsset->BlackboardAsset, InitializedBlackboard))
    {
        UE_LOG(LogTemp, Error, TEXT("HW2: Blackboard initialization failed"));
        return;
    }

    Blackboard = InitializedBlackboard;
    Blackboard->SetValueAsVector(HWAIKeys::HomeLocation, InPawn->GetActorLocation());
    Blackboard->SetValueAsBool(HWAIKeys::IsFleeing, false);

    if (!RunBehaviorTree(BehaviorTreeAsset))
    {
        UE_LOG(LogTemp, Error, TEXT("HW2: Behavior Tree could not start"));
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("HW2: AI ready; Behavior Tree started"));
    }

    BindToPlayerInteraction();
}

void AHWAIController::OnUnPossess()
{
    if (BoundPlayer.IsValid())
    {
        BoundPlayer->OnInteractionTriggered.RemoveDynamic(
            this, &AHWAIController::HandlePlayerInteraction);
    }
    BoundPlayer.Reset();
    GetWorldTimerManager().ClearTimer(FleeTimerHandle);
    GetWorldTimerManager().ClearTimer(BindRetryTimerHandle);
    Super::OnUnPossess();
}

void AHWAIController::BindToPlayerInteraction()
{
    AHWCharacter* Player = Cast<AHWCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player)
    {
        GetWorldTimerManager().SetTimer(
            BindRetryTimerHandle, this, &AHWAIController::BindToPlayerInteraction, 0.5f, false);
        return;
    }

    if (BoundPlayer.Get() != Player)
    {
        if (BoundPlayer.IsValid())
        {
            BoundPlayer->OnInteractionTriggered.RemoveDynamic(
                this, &AHWAIController::HandlePlayerInteraction);
        }
        BoundPlayer = Player;
        Player->OnInteractionTriggered.AddUniqueDynamic(
            this, &AHWAIController::HandlePlayerInteraction);
        UE_LOG(LogTemp, Display, TEXT("HW2: AI bound to player interaction"));
    }
}

void AHWAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor != UGameplayStatics::GetPlayerPawn(this, 0))
    {
        return;
    }

    bPlayerVisible = Stimulus.WasSuccessfullySensed();
    if (bPlayerVisible)
    {
        SetTargetActor(Actor);
    }
    else if (!Blackboard || !Blackboard->GetValueAsBool(HWAIKeys::IsFleeing))
    {
        SetTargetActor(nullptr);
    }
}

void AHWAIController::HandlePlayerInteraction()
{
    APawn* ControlledPawn = GetPawn();
    AHWCharacter* Player = BoundPlayer.Get();
    if (!ControlledPawn || !Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("HW2: interaction received without AI pawn or player"));
        return;
    }

    const float Distance = FVector::Dist(
        ControlledPawn->GetActorLocation(), Player->GetActorLocation());
    if (Distance > InteractionReactionRadius)
    {
        UE_LOG(LogTemp, Display, TEXT("HW2: interaction ignored at distance %.0f"), Distance);
        return;
    }

    if (Blackboard)
    {
        Blackboard->SetValueAsObject(HWAIKeys::TargetActor, Player);
        Blackboard->SetValueAsBool(HWAIKeys::IsFleeing, true);
    }

    GetWorldTimerManager().SetTimer(
        FleeTimerHandle, this, &AHWAIController::ClearFleeState, FleeDuration, false);

    UE_LOG(LogTemp, Display, TEXT("HW2: FLEE triggered at distance %.0f"), Distance);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1, 2.0f, FColor::Red, TEXT("AI: FLEE! (interaction detected)"));
    }
}

void AHWAIController::ClearFleeState()
{
    if (!Blackboard)
    {
        return;
    }

    Blackboard->SetValueAsBool(HWAIKeys::IsFleeing, false);
    if (!bPlayerVisible)
    {
        Blackboard->ClearValue(HWAIKeys::TargetActor);
    }
}

void AHWAIController::SetTargetActor(AActor* TargetActor)
{
    if (!Blackboard)
    {
        return;
    }

    if (TargetActor)
    {
        Blackboard->SetValueAsObject(HWAIKeys::TargetActor, TargetActor);
    }
    else
    {
        Blackboard->ClearValue(HWAIKeys::TargetActor);
    }
}
