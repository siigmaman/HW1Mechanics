#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HWEditorSetupLibrary.generated.h"

UCLASS()
class HW1MECHANICSEDITORTOOLS_API UHWEditorSetupLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "HW1 Setup")
    static bool AddInteractionPrint();

    UFUNCTION(BlueprintCallable, Category = "HW1 Setup")
    static bool AddSprintAnimation();
};
