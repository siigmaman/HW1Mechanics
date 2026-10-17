#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HWAnimInstance.generated.h"

UCLASS()
class HW1MECHANICS_API UHWAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category = "Sprint")
    bool bIsSprinting = false;
};
