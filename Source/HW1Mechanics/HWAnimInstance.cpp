#include "HWAnimInstance.h"
#include "HWCharacter.h"

void UHWAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    const AHWCharacter* Character = Cast<AHWCharacter>(TryGetPawnOwner());
    bIsSprinting = Character && Character->bIsSprinting;
}
