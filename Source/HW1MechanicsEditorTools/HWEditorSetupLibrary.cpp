#include "HWEditorSetupLibrary.h"
#include "../HW1Mechanics/HWCharacter.h"
#include "../HW1Mechanics/HWAnimInstance.h"

#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_TransitionResult.h"
#include "AnimStateNode.h"
#include "AnimStateTransitionNode.h"
#include "Animation/AnimSequence.h"
#include "AnimationStateMachineSchema.h"
#include "BlueprintEditorLibrary.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphUtilities.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"

bool UHWEditorSetupLibrary::AddInteractionPrint()
{
    UBlueprint* Blueprint = LoadObject<UBlueprint>(
        nullptr, TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter"));
    if (!Blueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: character Blueprint not found"));
        return false;
    }

    UK2Node_Event* Event = UBlueprintEditorLibrary::AddEventOverride(
        Blueprint, GET_FUNCTION_NAME_CHECKED(AHWCharacter, OnInteractionTriggeredInBlueprint),
        FIntPoint(1200, 100));
    if (!Event)
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: interaction event could not be added"));
        return false;
    }

    UEdGraph* Graph = Event->GetGraph();
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
        {
            if (Call->FunctionReference.GetMemberName() == GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, PrintString)
                && Call->FindPin(TEXT("InString"))
                && Call->FindPin(TEXT("InString"))->DefaultValue == TEXT("Interaction triggered!"))
            {
                return true;
            }
        }
    }

    UFunction* PrintFunction = UKismetSystemLibrary::StaticClass()->FindFunctionByName(
        GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, PrintString));
    if (!PrintFunction)
    {
        return false;
    }

    Blueprint->Modify();
    Graph->Modify();
    FGraphNodeCreator<UK2Node_CallFunction> Creator(*Graph);
    UK2Node_CallFunction* Print = Creator.CreateNode(false);
    Print->SetFromFunction(PrintFunction);
    Print->NodePosX = Event->NodePosX + 300;
    Print->NodePosY = Event->NodePosY;
    Creator.Finalize();

    UEdGraphPin* MessagePin = Print->FindPin(TEXT("InString"));
    UEdGraphPin* ThenPin = Event->FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* ExecPin = Print->GetExecPin();
    if (!MessagePin || !ThenPin || !ExecPin)
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: Print String pins are unavailable"));
        return false;
    }
    MessagePin->DefaultValue = TEXT("Interaction triggered!");
    if (!Graph->GetSchema()->TryCreateConnection(ThenPin, ExecPin))
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: could not connect interaction event"));
        return false;
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    return UBlueprintEditorLibrary::CompileBlueprint(Blueprint);
}

namespace
{
bool SetTransitionRule(UAnimStateTransitionNode* Transition, bool bNegate)
{
    UEdGraph* RuleGraph = Transition->BoundGraph;
    UAnimGraphNode_TransitionResult* Result = nullptr;
    for (UEdGraphNode* Node : RuleGraph->Nodes)
    {
        Result = Cast<UAnimGraphNode_TransitionResult>(Node);
        if (Result)
        {
            break;
        }
    }
    if (!Result)
    {
        return false;
    }

    FGraphNodeCreator<UK2Node_VariableGet> GetCreator(*RuleGraph);
    UK2Node_VariableGet* GetSprint = GetCreator.CreateNode(false);
    GetSprint->VariableReference.SetSelfMember(GET_MEMBER_NAME_CHECKED(UHWAnimInstance, bIsSprinting));
    GetSprint->NodePosX = Result->NodePosX - 500;
    GetSprint->NodePosY = Result->NodePosY;
    GetCreator.Finalize();

    UEdGraphPin* Value = GetSprint->FindPin(GET_MEMBER_NAME_CHECKED(UHWAnimInstance, bIsSprinting));
    if (!Value)
    {
        return false;
    }

    if (bNegate)
    {
        UFunction* NotFunction = UKismetMathLibrary::StaticClass()->FindFunctionByName(
            GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Not_PreBool));
        if (!NotFunction)
        {
            return false;
        }
        FGraphNodeCreator<UK2Node_CallFunction> NotCreator(*RuleGraph);
        UK2Node_CallFunction* Not = NotCreator.CreateNode(false);
        Not->SetFromFunction(NotFunction);
        Not->NodePosX = Result->NodePosX - 250;
        Not->NodePosY = Result->NodePosY;
        NotCreator.Finalize();
        UEdGraphPin* Input = Not->FindPin(TEXT("A"));
        UEdGraphPin* Output = Not->GetReturnValuePin();
        if (!Input || !Output || !RuleGraph->GetSchema()->TryCreateConnection(Value, Input))
        {
            return false;
        }
        Value = Output;
    }

    UEdGraphPin* RuleResult = nullptr;
    for (UEdGraphPin* Pin : Result->Pins)
    {
        if (Pin->Direction == EGPD_Input && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Boolean)
        {
            RuleResult = Pin;
            break;
        }
    }
    return RuleResult && RuleGraph->GetSchema()->TryCreateConnection(Value, RuleResult);
}
}

bool UHWEditorSetupLibrary::AddSprintAnimation()
{
    UBlueprint* AnimBlueprint = LoadObject<UBlueprint>(
        nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed"));
    if (!AnimBlueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: ABP_Unarmed not found"));
        return false;
    }
    if (!AnimBlueprint->ParentClass->IsChildOf(UHWAnimInstance::StaticClass()))
    {
        UBlueprintEditorLibrary::ReparentBlueprint(AnimBlueprint, UHWAnimInstance::StaticClass());
    }

    UEdGraph* StateMachine = UBlueprintEditorLibrary::FindGraph(AnimBlueprint, TEXT("Locomotion"));
    UAnimStateNode* WalkRun = nullptr;
    if (StateMachine)
    {
        for (UEdGraphNode* Node : StateMachine->Nodes)
        {
            if (UAnimStateNode* State = Cast<UAnimStateNode>(Node))
            {
                if (State->BoundGraph && State->BoundGraph->GetName() == TEXT("Walk / Run"))
                {
                    WalkRun = State;
                }
                if (State->BoundGraph && State->BoundGraph->GetName() == TEXT("Sprint"))
                {
                    return true;
                }
            }
        }
    }
    if (!StateMachine || !WalkRun)
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: Locomotion / Walk Run state not found"));
        return false;
    }

    UAnimSequence* Jog = LoadObject<UAnimSequence>(
        nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd"));
    if (!Jog)
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: Jog animation not found"));
        return false;
    }

    AnimBlueprint->Modify();
    StateMachine->Modify();
    UAnimStateNode* Sprint = FEdGraphSchemaAction_NewStateNode::SpawnNodeFromTemplate<UAnimStateNode>(
        StateMachine, NewObject<UAnimStateNode>(), FVector2f(WalkRun->NodePosX + 360, WalkRun->NodePosY));
    if (!Sprint || !Sprint->BoundGraph)
    {
        return false;
    }
    FEdGraphUtilities::RenameGraphToNameOrCloseToName(Sprint->BoundGraph, TEXT("Sprint"));

    UEdGraph* SprintGraph = Sprint->BoundGraph;
    FGraphNodeCreator<UAnimGraphNode_SequencePlayer> SequenceCreator(*SprintGraph);
    UAnimGraphNode_SequencePlayer* Sequence = SequenceCreator.CreateNode(false);
    Sequence->Node.SetSequence(Jog);
    Sequence->Node.SetPlayRate(1.4f);
    Sequence->NodePosX = -300;
    Sequence->NodePosY = 0;
    SequenceCreator.Finalize();

    UEdGraphPin* PoseOutput = nullptr;
    for (UEdGraphPin* Pin : Sequence->Pins)
    {
        if (Pin->Direction == EGPD_Output)
        {
            PoseOutput = Pin;
            break;
        }
    }
    if (!PoseOutput || !SprintGraph->GetSchema()->TryCreateConnection(PoseOutput, Sprint->GetPoseSinkPinInsideState()))
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: Sprint animation pose could not be connected"));
        return false;
    }

    UAnimStateTransitionNode* ToSprint = FEdGraphSchemaAction_NewStateNode::SpawnNodeFromTemplate<UAnimStateTransitionNode>(
        StateMachine, NewObject<UAnimStateTransitionNode>(),
        FVector2f(WalkRun->NodePosX + 180, WalkRun->NodePosY - 80), false);
    ToSprint->CreateConnections(WalkRun, Sprint);
    UAnimStateTransitionNode* FromSprint = FEdGraphSchemaAction_NewStateNode::SpawnNodeFromTemplate<UAnimStateTransitionNode>(
        StateMachine, NewObject<UAnimStateTransitionNode>(),
        FVector2f(WalkRun->NodePosX + 180, WalkRun->NodePosY + 80), false);
    FromSprint->CreateConnections(Sprint, WalkRun);

    if (!SetTransitionRule(ToSprint, false) || !SetTransitionRule(FromSprint, true))
    {
        UE_LOG(LogTemp, Error, TEXT("HW1: Sprint transition rules could not be connected"));
        return false;
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimBlueprint);
    return UBlueprintEditorLibrary::CompileBlueprint(AnimBlueprint);
}
