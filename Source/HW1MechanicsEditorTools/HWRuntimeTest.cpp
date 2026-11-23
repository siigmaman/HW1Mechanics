#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../HW1Mechanics/HWAICharacter.h"
#include "../HW1Mechanics/HWAIController.h"
#include "../HW1Mechanics/HWCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHW2RuntimeTest, "HW2.Runtime.FleeMovement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

class FHW2WaitForNavigation : public IAutomationLatentCommand
{
public:
    explicit FHW2WaitForNavigation(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
        FNavLocation Projected;
        if (Nav && !Nav->IsNavigationBuildingLocked()
            && Nav->ProjectPointToNavigation(FVector(0, 1200, 110), Projected, FVector(200, 200, 300)))
        {
            return true;
        }
        if (FPlatformTime::Seconds() - Start > 20.0)
        {
            Test->AddError(TEXT("Level navigation did not become ready before Play"));
            return true;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    double Start = FPlatformTime::Seconds();
};

class FHW2CheckMovement : public IAutomationLatentCommand
{
public:
    explicit FHW2CheckMovement(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override
    {
        UWorld* World = GEditor->PlayWorld;
        if (FPlatformTime::Seconds() - WallStart > 45.0)
        {
            Test->AddError(TEXT("PIE movement test timed out"));
            return true;
        }
        if (!World || World->GetTimeSeconds() < 2.0f) { return false; }
        AHWCharacter* Player = Cast<AHWCharacter>(UGameplayStatics::GetPlayerPawn(World, 0));
        AHWAICharacter* Enemy = nullptr;
        TActorIterator<AHWAICharacter> It(World);
        if (It) { Enemy = *It; }
        if (!Player || !Enemy)
        {
            Test->AddError(TEXT("Player or enemy missing in PIE"));
            return true;
        }
        AHWAIController* AI = Cast<AHWAIController>(Enemy->GetController());
        UBlackboardComponent* BB = AI ? AI->GetBlackboardComponent() : nullptr;
        if (!BB)
        {
            Test->AddError(TEXT("AI controller or blackboard missing"));
            return true;
        }
        if (Phase == 0)
        {
            UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
            ANavigationData* Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
            FNavLocation Projected;
            const bool bProjected = Nav && Nav->ProjectPointToNavigation(Enemy->GetActorLocation(), Projected, FVector(200, 200, 300));
            Test->AddInfo(FString::Printf(TEXT("HW2_NAV_RUNTIME: system=%s data=%s mode=%d projected=%d enemy=%s"),
                *GetNameSafe(Nav), *GetNameSafe(Data), Data ? int32(Data->GetRuntimeGenerationMode()) : -1,
                bProjected, *Enemy->GetActorLocation().ToCompactString()));
            for (TActorIterator<ANavMeshBoundsVolume> Bounds(World); Bounds; ++Bounds)
            {
                Test->AddInfo(FString::Printf(TEXT("HW2_NAV_BOUNDS: %s bounds=%s"), *Bounds->GetName(),
                    *Bounds->GetComponentsBoundingBox(true).ToString()));
            }
            UBehaviorTree* Tree = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/HW2/AI/BT_Enemy.BT_Enemy"));
            UBehaviorTreeComponent* Brain = Cast<UBehaviorTreeComponent>(AI->GetBrainComponent());
            Test->AddInfo(FString::Printf(TEXT("HW2_BRAIN: running=%d paused=%d tick=%d active=%s %s"),
                Brain->IsRunning(), Brain->IsPaused(), Brain->IsComponentTickEnabled(),
                *GetNameSafe(Brain->GetActiveNode()), *Brain->GetDebugInfoString()));
            for (int32 Index = 0; Tree && Tree->RootNode && Index < Tree->RootNode->Children.Num(); ++Index)
            {
                Test->AddInfo(FString::Printf(TEXT("Runtime branch %d decorators=%d"),
                    Index, Tree->RootNode->Children[Index].Decorators.Num()));
                UBTCompositeNode* Branch = Tree->RootNode->Children[Index].ChildComposite;
                Test->AddInfo(FString::Printf(TEXT("HW2_BRANCH: %s children=%d"), *GetNameSafe(Branch), Branch ? Branch->Children.Num() : -1));
                if (Branch)
                {
                    for (const FBTCompositeChild& Child : Branch->Children)
                    {
                        Test->AddInfo(FString::Printf(TEXT("HW2_TASK: %s"), *GetNameSafe(Child.ChildTask)));
                    }
                }
            }
            Player->SetActorLocation(FVector(-600, 900, 110), false, nullptr, ETeleportType::TeleportPhysics);
            Enemy->SetActorLocation(FVector(0, 900, 110), false, nullptr, ETeleportType::TeleportPhysics);
            Test->TestFalse(TEXT("E is rejected at six metres"), Player->TryInteract());
            Test->TestFalse(TEXT("Distant E does not trigger flee"), BB->GetValueAsBool(TEXT("IsFleeing")));
            Player->SetActorLocation(FVector(-301, 900, 110), false, nullptr, ETeleportType::TeleportPhysics);
            Test->TestFalse(TEXT("E is unavailable outside three metres"), Player->CanInteract());
            Player->SetActorLocation(FVector(-300, 900, 110), false, nullptr, ETeleportType::TeleportPhysics);
            Test->TestTrue(TEXT("E is available at exactly three metres"), Player->CanInteract());
            Player->SetActorLocation(FVector(-250, 900, 110), false, nullptr, ETeleportType::TeleportPhysics);
            // Headless tests must evaluate the pose even without a rendered viewport.
            Enemy->GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
            Start = Enemy->GetActorLocation();
            DistanceBefore = FVector::Dist2D(Start, Player->GetActorLocation());
            Test->TestTrue(TEXT("E is accepted nearby"), Player->TryInteract());
            Test->TestTrue(TEXT("Interaction sets IsFleeing"), BB->GetValueAsBool(TEXT("IsFleeing")));
            StartTime = World->GetTimeSeconds();
            Phase = 1;
            return false;
        }
        const FName LimbBones[] = {TEXT("foot_l"), TEXT("foot_r"), TEXT("hand_l"), TEXT("hand_r")};
        if (Phase == 1 && World->GetTimeSeconds() - StartTime > 0.4f && Enemy->GetVelocity().Size2D() > 100.0f)
        {
            for (int32 Index = 0; Index < 4; ++Index)
            {
                const FVector Position = Enemy->GetMesh()->GetSocketTransform(LimbBones[Index], RTS_Component).GetLocation();
                if (!bCapturedPose) { InitialPose[Index] = Position; }
                MaxLimbTravel[Index] = FMath::Max(MaxLimbTravel[Index], float(FVector::Dist(InitialPose[Index], Position)));
            }
            bCapturedPose = true;
        }
        if (Phase == 1 && World->GetTimeSeconds() - StartTime >= 2.0f)
        {
            const float Moved = FVector::Dist2D(Start, Enemy->GetActorLocation());
            const float DistanceAfter = FVector::Dist2D(Enemy->GetActorLocation(), Player->GetActorLocation());
            UBehaviorTreeComponent* Brain = Cast<UBehaviorTreeComponent>(AI->GetBrainComponent());
            Test->AddInfo(FString::Printf(TEXT("HW2_AFTER: running=%d paused=%d tick=%d active=%s %s"),
                Brain->IsRunning(), Brain->IsPaused(), Brain->IsComponentTickEnabled(),
                *GetNameSafe(Brain->GetActiveNode()), *Brain->GetDebugInfoString()));
            Test->AddInfo(FString::Printf(TEXT("HW2_RUNTIME: moved=%.1f distance_before=%.1f distance_after=%.1f flee_point=%s"),
                Moved, DistanceBefore, DistanceAfter, *BB->GetValueAsVector(TEXT("FleeLocation")).ToCompactString()));
            Test->TestTrue(TEXT("AI actually moved at least 100 cm"), Moved > 100.0f);
            Test->TestTrue(TEXT("AI moved away from player"), DistanceAfter > DistanceBefore + 100.0f);
            for (int32 Index = 0; Index < 4; ++Index)
            {
                Test->AddInfo(FString::Printf(TEXT("HW2_ANIMATION: %s travel=%.1f cm"), *LimbBones[Index].ToString(), MaxLimbTravel[Index]));
                Test->TestTrue(FString::Printf(TEXT("Moving AI animates %s"), *LimbBones[Index].ToString()), MaxLimbTravel[Index] > 5.0f);
            }
            Phase = 2;
        }
        if (Phase == 2 && World->GetTimeSeconds() - StartTime >= 4.5f)
        {
            Test->TestFalse(TEXT("Flee state expires"), BB->GetValueAsBool(TEXT("IsFleeing")));
            return true;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    double WallStart = FPlatformTime::Seconds();
    int32 Phase = 0;
    FVector Start = FVector::ZeroVector;
    float StartTime = 0;
    float DistanceBefore = 0;
    bool bCapturedPose = false;
    FVector InitialPose[4];
    float MaxLimbTravel[4] = {};
};

bool FHW2RuntimeTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/ThirdPerson/Lvl_ThirdPerson")));
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FHW2WaitForNavigation>(this));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FHW2CheckMovement>(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
