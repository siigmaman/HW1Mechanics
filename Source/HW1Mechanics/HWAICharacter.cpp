#include "HWAICharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HWAIController.h"
#include "UObject/ConstructorHelpers.h"

AHWAICharacter::AHWAICharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = true;
    Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    Movement->MaxWalkSpeed = 450.0f;
    // ABP_Unarmed enters locomotion when both speed and acceleration are nonzero.
    Movement->GetNavMovementProperties()->bUseAccelerationForPaths = true;

    AIControllerClass = AHWAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
    if (MeshFinder.Succeeded())
    {
        GetMesh()->SetSkeletalMeshAsset(MeshFinder.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -97.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }

    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimFinder(
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
    if (AnimFinder.Succeeded())
    {
        GetMesh()->SetAnimInstanceClass(AnimFinder.Class);
    }
}
