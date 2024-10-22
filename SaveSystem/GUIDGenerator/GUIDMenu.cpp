#include "GUIDMenu.h"

#include "EditorGUIDGenerator.h"
#include "EditorStyleSet.h"
#include "ScreenCapture/SaveLoadSystem/SaveComponent.h"

#define LOCTEXT_NAMESPACE "MenuTool"

class FMenuToolCommands : public TCommands<FMenuToolCommands>
{
public:

	FMenuToolCommands()
		: TCommands<FMenuToolCommands>(
		TEXT("MenuTool"), // Context name for fast lookup
		FText::FromString("Example Menu tool"), // Context name for displaying
		NAME_None,   // No parent context
		FEditorStyle::GetStyleSetName() // Icon Style Set
		)
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(RegenerateAllCommand, "Regenerate All", "Regenerate All GUIDS in the World", EUserInterfaceActionType::Button, FInputGesture());
		UI_COMMAND(RegenerateNonValidCommand, "Regenerate Non Valid", "Regenerate all Non Valid GUID in the World", EUserInterfaceActionType::Button, FInputGesture());

	}

public:
	TSharedPtr<FUICommandInfo> RegenerateAllCommand;
	TSharedPtr<FUICommandInfo> RegenerateNonValidCommand;
};

void FGUIDMenu::MapCommands()
{
	const auto& Commands = FMenuToolCommands::Get();

	CommandList->MapAction(
		Commands.RegenerateAllCommand,
		FExecuteAction::CreateSP(this, &FGUIDMenu::RegenerateAll),
		FCanExecuteAction());

	CommandList->MapAction(
	Commands.RegenerateNonValidCommand,
	FExecuteAction::CreateSP(this, &FGUIDMenu::RegenerateNonValid),
	FCanExecuteAction());
}

void FGUIDMenu::OnStartupModule()
{
	CommandList = MakeShareable(new FUICommandList);
	FMenuToolCommands::Register();
	MapCommands();
	FEditorGUIDGeneratorModule::Get().AddMenuExtension(
		FMenuExtensionDelegate::CreateRaw(this, &FGUIDMenu::MakeMenuEntry),
		FName("Section_1"),
		CommandList);
}

void FGUIDMenu::OnShutdownModule()
{
	FMenuToolCommands::Unregister();
}

void FGUIDMenu::MakeMenuEntry(FMenuBuilder &menuBuilder)
{
	menuBuilder.AddMenuEntry(FMenuToolCommands::Get().RegenerateAllCommand);
	menuBuilder.AddMenuEntry(FMenuToolCommands::Get().RegenerateNonValidCommand);

}

void FGUIDMenu::RegenerateAll()
{
	for ( TObjectIterator<USaveComponent> SavableObj; SavableObj; ++SavableObj )
	{
		if(SavableObj->IsTemplate()) continue;
		SavableObj->GeneratePersistentID(true);
	}
}

void FGUIDMenu::RegenerateNonValid()
{
	for ( TObjectIterator<USaveComponent> SavableObj; SavableObj; ++SavableObj )
	{
		if(SavableObj->IsTemplate()) continue;
		if(SavableObj->PersistentID.IsValid()) continue;;
		SavableObj->GeneratePersistentID(false);
	}
}

#undef LOCTEXT_NAMESPACE
