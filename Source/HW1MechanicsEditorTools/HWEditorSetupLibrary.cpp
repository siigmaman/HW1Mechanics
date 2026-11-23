#include "HWEditorSetupLibrary.h"
#include "../HW1Mechanics/HWCharacter.h"
#include "../HW1Mechanics/HWAnimInstance.h"
#include "../HW1Mechanics/HWBTDecorator_BlackboardState.h"
#include "../HW1Mechanics/HWBTTask_FindPatrolPoint.h"
#include "../HW1Mechanics/HWBTTask_MoveToBlackboard.h"
#include "../HW1Mechanics/HWBTTask_SetFleeLocation.h"

#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_TransitionResult.h"
#include "AnimStateNode.h"
#include "AnimStateTransitionNode.h"
#include "Animation/AnimSequence.h"
#include "AnimationStateMachineSchema.h"
#include "AIGraphNode.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Composites/BTComposite_Selector.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTree/Tasks/BTTask_Wait.h"
#include "BehaviorTreeGraph.h"
#include "BehaviorTreeGraphNode.h"
#include "BehaviorTreeGraphNode_Root.h"
#include "BlueprintEditorLibrary.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphUtilities.h"
#include "EdGraphSchema_K2.h"
#include "Engine/StaticMesh.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MeshSelectors/PCGMeshSelectorWeighted.h"
#include "Misc/PackageName.h"
#include "PCGCommon.h"
#include "PCGGraph.h"
#include "PCGNode.h"
#include "Elements/PCGCreatePointsGrid.h"
#include "Elements/PCGStaticMeshSpawner.h"
#include "Elements/PCGTransformPoints.h"
#include "UObject/Package.h"

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

namespace
{
template <typename T>
T* LoadOrCreateAsset(const TCHAR* ObjectPath)
{
    if (T* Existing = LoadObject<T>(nullptr, ObjectPath))
    {
        Existing->Modify();
        return Existing;
    }

	const FString ObjectPathString(ObjectPath);
	const FString PackageName = FPackageName::ObjectPathToPackageName(ObjectPathString);
	const FString AssetName = FPackageName::ObjectPathToObjectName(ObjectPathString);
    UPackage* Package = CreatePackage(*PackageName);
    T* Asset = NewObject<T>(
        Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
    if (Asset)
    {
        FAssetRegistryModule::AssetCreated(Asset);
        Package->MarkPackageDirty();
    }
    return Asset;
}

template <typename T>
T* AddBlackboardKey(UBlackboardData* Blackboard, const FName KeyName)
{
    FBlackboardEntry& Entry = Blackboard->Keys.AddDefaulted_GetRef();
    Entry.EntryName = KeyName;
    Entry.KeyType = NewObject<T>(Blackboard);
    return Cast<T>(Entry.KeyType);
}

UHWBTDecorator_BlackboardState* NewStateDecorator(
    UBehaviorTree* Tree, const FName KeyName, bool bTreatAsBoolean, bool bExpectedValue)
{
    UHWBTDecorator_BlackboardState* Decorator =
        NewObject<UHWBTDecorator_BlackboardState>(Tree, NAME_None, RF_Transactional);
    Decorator->BlackboardKeyName = KeyName;
    Decorator->bTreatAsBoolean = bTreatAsBoolean;
    Decorator->bExpectedValue = bExpectedValue;
    return Decorator;
}

void AddTaskChild(UBTCompositeNode* Parent, UBTTaskNode* Task)
{
    FBTCompositeChild& Child = Parent->Children.AddDefaulted_GetRef();
    Child.ChildTask = Task;
}

void AddCompositeChild(
    UBTCompositeNode* Parent, UBTCompositeNode* ChildComposite,
    UBTDecorator* Decorator)
{
    FBTCompositeChild& Child = Parent->Children.AddDefaulted_GetRef();
    Child.ChildComposite = ChildComposite;
    if (Decorator)
    {
        Child.Decorators.Add(Decorator);
    }
}

UHWBTTask_MoveToBlackboard* NewMoveTask(
    UBehaviorTree* Tree, const FName KeyName, float AcceptanceRadius)
{
    UHWBTTask_MoveToBlackboard* Move =
        NewObject<UHWBTTask_MoveToBlackboard>(Tree, NAME_None, RF_Transactional);
    Move->BlackboardKeyName = KeyName;
    Move->AcceptanceRadius = AcceptanceRadius;
    return Move;
}

UBTTask_Wait* NewWaitTask(UBehaviorTree* Tree, float Seconds)
{
    UBTTask_Wait* Wait = NewObject<UBTTask_Wait>(Tree, NAME_None, RF_Transactional);
    Wait->WaitTime = FValueOrBBKey_Float(Seconds);
    Wait->RandomDeviation = FValueOrBBKey_Float(0.15f);
    return Wait;
}

bool BuildBlackboard(UBlackboardData*& OutBlackboard)
{
    OutBlackboard = LoadOrCreateAsset<UBlackboardData>(
        TEXT("/Game/HW2/AI/BB_Enemy.BB_Enemy"));
    if (!OutBlackboard)
    {
        return false;
    }

    OutBlackboard->Keys.Reset();
    UBlackboardKeyType_Object* TargetKey =
        AddBlackboardKey<UBlackboardKeyType_Object>(OutBlackboard, TEXT("TargetActor"));
    TargetKey->BaseClass = AActor::StaticClass();
    AddBlackboardKey<UBlackboardKeyType_Vector>(OutBlackboard, TEXT("HomeLocation"));
    AddBlackboardKey<UBlackboardKeyType_Vector>(OutBlackboard, TEXT("PatrolLocation"));
    AddBlackboardKey<UBlackboardKeyType_Vector>(OutBlackboard, TEXT("FleeLocation"));
    AddBlackboardKey<UBlackboardKeyType_Bool>(OutBlackboard, TEXT("IsFleeing"));
    OutBlackboard->UpdateKeyIDs();
    OutBlackboard->UpdateIfHasSynchronizedKeys();
    OutBlackboard->MarkPackageDirty();
    return true;
}

bool BuildBehaviorTree(UBlackboardData* Blackboard, UBehaviorTree*& OutTree)
{
    OutTree = LoadOrCreateAsset<UBehaviorTree>(
        TEXT("/Game/HW2/AI/BT_Enemy.BT_Enemy"));
    if (!OutTree)
    {
        return false;
    }

    OutTree->BlackboardAsset = Blackboard;
    OutTree->RootDecorators.Reset();
    OutTree->RootDecoratorOps.Reset();

    UBTComposite_Selector* Root =
        NewObject<UBTComposite_Selector>(OutTree, NAME_None, RF_Transactional);
    UBTComposite_Sequence* FleeSequence =
        NewObject<UBTComposite_Sequence>(OutTree, NAME_None, RF_Transactional);
    UBTComposite_Sequence* ChaseSequence =
        NewObject<UBTComposite_Sequence>(OutTree, NAME_None, RF_Transactional);
    UBTComposite_Sequence* PatrolSequence =
        NewObject<UBTComposite_Sequence>(OutTree, NAME_None, RF_Transactional);

    UHWBTTask_SetFleeLocation* FleePoint =
        NewObject<UHWBTTask_SetFleeLocation>(OutTree, NAME_None, RF_Transactional);
    AddTaskChild(FleeSequence, FleePoint);
    AddTaskChild(FleeSequence, NewMoveTask(OutTree, TEXT("FleeLocation"), 120.0f));
    AddTaskChild(FleeSequence, NewWaitTask(OutTree, 0.2f));

    AddTaskChild(ChaseSequence, NewMoveTask(OutTree, TEXT("TargetActor"), 140.0f));
    AddTaskChild(ChaseSequence, NewWaitTask(OutTree, 0.15f));

    UHWBTTask_FindPatrolPoint* PatrolPoint =
        NewObject<UHWBTTask_FindPatrolPoint>(OutTree, NAME_None, RF_Transactional);
    AddTaskChild(PatrolSequence, PatrolPoint);
    AddTaskChild(PatrolSequence, NewMoveTask(OutTree, TEXT("PatrolLocation"), 100.0f));
    AddTaskChild(PatrolSequence, NewWaitTask(OutTree, 1.2f));

    AddCompositeChild(
        Root, FleeSequence,
        NewStateDecorator(OutTree, TEXT("IsFleeing"), true, true));
    AddCompositeChild(
        Root, ChaseSequence,
        NewStateDecorator(OutTree, TEXT("TargetActor"), false, true));
    AddCompositeChild(
        Root, PatrolSequence,
        NewStateDecorator(OutTree, TEXT("TargetActor"), false, false));
    OutTree->RootNode = Root;

    if (OutTree->BTGraph)
    {
        OutTree->BTGraph->Rename(
            nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_DoNotDirty);
        OutTree->BTGraph = nullptr;
    }

    UBehaviorTreeGraph* Graph = Cast<UBehaviorTreeGraph>(FBlueprintEditorUtils::CreateNewGraph(
        OutTree, TEXT("Behavior Tree"), UBehaviorTreeGraph::StaticClass(),
        GetDefault<UBehaviorTreeGraph>()->Schema));
    if (!Graph)
    {
        return false;
    }

    OutTree->BTGraph = Graph;
    Graph->GetSchema()->CreateDefaultNodesForGraph(*Graph);
    Graph->OnCreated();
    Graph->Initialize();

    int32 GraphNodeCount = 0;
    int32 MissingInstanceCount = 0;
    for (UEdGraphNode* EditorNode : Graph->Nodes)
    {
        UBehaviorTreeGraphNode* BehaviorNode = Cast<UBehaviorTreeGraphNode>(EditorNode);
        if (!BehaviorNode)
        {
            continue;
        }

        ++GraphNodeCount;
        if (!BehaviorNode->NodeInstance && !BehaviorNode->IsA<UBehaviorTreeGraphNode_Root>())
        {
            ++MissingInstanceCount;
        }
        for (UAIGraphNode* SubNode : BehaviorNode->SubNodes)
        {
            if (!SubNode || !SubNode->NodeInstance)
            {
                ++MissingInstanceCount;
            }
        }
    }
    if (MissingInstanceCount > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("HW2: Behavior Tree graph has %d missing node instances"), MissingInstanceCount);
        return false;
    }
    UE_LOG(LogTemp, Display, TEXT("HW2_BT_VALID: graph nodes=%d"), GraphNodeCount);
    OutTree->MarkPackageDirty();
    return true;
}

bool BuildPCGGraph(UPCGGraph*& OutGraph)
{
    OutGraph = LoadOrCreateAsset<UPCGGraph>(
        TEXT("/Game/HW2/PCG/PCG_ArenaProps.PCG_ArenaProps"));
    if (!OutGraph)
    {
        return false;
    }

    TArray<UPCGNode*> ExistingNodes = OutGraph->GetNodes();
    OutGraph->RemoveNodes(ExistingNodes);
    OutGraph->Description = FText::FromString(
        TEXT("Procedurally scatters varied obstacles around the arena while keeping the center open."));

    UPCGCreatePointsGridSettings* GridSettings = nullptr;
    UPCGNode* GridNode = OutGraph->AddNodeOfType(GridSettings);
    GridSettings->GridExtents = FVector(2100.0f, 2100.0f, 25.0f);
    GridSettings->CellSize = FVector(700.0f, 700.0f, 50.0f);
    GridSettings->CoordinateSpace = EPCGCoordinateSpace::OriginalComponent;
    GridSettings->PointPosition = EPCGPointPosition::CellCenter;
    GridSettings->bCullPointsOutsideVolume = true;

    UPCGTransformPointsSettings* TransformSettings = nullptr;
    UPCGNode* TransformNode = OutGraph->AddNodeOfType(TransformSettings);
    TransformSettings->OffsetMin = FVector(-150.0f, -150.0f, 70.0f);
    TransformSettings->OffsetMax = FVector(150.0f, 150.0f, 70.0f);
    TransformSettings->RotationMin = FRotator(0.0f, 0.0f, 0.0f);
    TransformSettings->RotationMax = FRotator(0.0f, 360.0f, 0.0f);
    TransformSettings->ScaleMin = FVector(0.65f);
    TransformSettings->ScaleMax = FVector(1.35f);
    TransformSettings->bUniformScale = true;

    UPCGStaticMeshSpawnerSettings* SpawnerSettings = nullptr;
    UPCGNode* SpawnerNode = OutGraph->AddNodeOfType(SpawnerSettings);
    SpawnerSettings->SetMeshSelectorType(UPCGMeshSelectorWeighted::StaticClass());
    SpawnerSettings->bSynchronousLoad = true;
    UPCGMeshSelectorWeighted* Selector =
        Cast<UPCGMeshSelectorWeighted>(SpawnerSettings->MeshSelectorParameters);
    UStaticMesh* Cube = LoadObject<UStaticMesh>(
        nullptr, TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(
        nullptr, TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder"));
    UStaticMesh* Quarter = LoadObject<UStaticMesh>(
        nullptr, TEXT("/Game/LevelPrototyping/Meshes/SM_QuarterCylinder.SM_QuarterCylinder"));
    if (!Selector || !Cube || !Cylinder || !Quarter)
    {
        return false;
    }
    Selector->MeshEntries.Reset();
    Selector->MeshEntries.Add(FPCGMeshSelectorWeightedEntry(Cube, 3));
    Selector->MeshEntries.Add(FPCGMeshSelectorWeightedEntry(Cylinder, 2));
    Selector->MeshEntries.Add(FPCGMeshSelectorWeightedEntry(Quarter, 1));

    GridNode->SetNodePosition(-600, 0);
    TransformNode->SetNodePosition(-200, 0);
    SpawnerNode->SetNodePosition(200, 0);
    OutGraph->GetOutputNode()->SetNodePosition(600, 0);

    OutGraph->AddEdge(
        GridNode, PCGPinConstants::DefaultOutputLabel,
        TransformNode, PCGPinConstants::DefaultInputLabel);
    OutGraph->AddEdge(
        TransformNode, PCGPinConstants::DefaultOutputLabel,
        SpawnerNode, PCGPinConstants::DefaultInputLabel);
    OutGraph->AddEdge(
        SpawnerNode, PCGPinConstants::DefaultOutputLabel,
        OutGraph->GetOutputNode(), PCGPinConstants::DefaultOutputLabel);
    OutGraph->MarkPackageDirty();
    return true;
}
}

bool UHWEditorSetupLibrary::SetupHW2Assets()
{
    UBlackboardData* Blackboard = nullptr;
    UBehaviorTree* BehaviorTree = nullptr;
    UPCGGraph* PCGGraph = nullptr;
    const bool bSuccess = BuildBlackboard(Blackboard)
        && BuildBehaviorTree(Blackboard, BehaviorTree)
        && BuildPCGGraph(PCGGraph);

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("HW2: asset generation failed"));
    }
    return bSuccess;
}
