#include "EditorGUIDGenerator.h"

#include "Editor.h"

#include "LevelEditor.h"
#include "GUIDMenu.h"

#define LOCTEXT_NAMESPACE "FEditorGUIDGeneratorModule"

void FEditorGUIDGeneratorModule::AddModuleListeners()
{
	// add tools later
	ModuleListeners.Add(MakeShareable(new FGUIDMenu));
}

void FEditorGUIDGeneratorModule::StartupModule()
{
	if (!IsRunningCommandlet())
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
		MenuExtender = MakeShareable(new FExtender);
		MenuExtender->AddMenuBarExtension("Window", EExtensionHook::After, NULL, FMenuBarExtensionDelegate::CreateRaw(this, &FEditorGUIDGeneratorModule::MakePulldownMenu));
		LevelEditorMenuExtensibilityManager->AddExtender(MenuExtender);
	}
	IGUIDModuleInterface::StartupModule();
}

void FEditorGUIDGeneratorModule::ShutdownModule()
{
	IGUIDModuleInterface::ShutdownModule();
}

TSharedRef<FWorkspaceItem> FEditorGUIDGeneratorModule::MenuRoot = FWorkspaceItem::NewGroup(FText::FromString("Menu Root"));


void FEditorGUIDGeneratorModule::AddMenuExtension(const FMenuExtensionDelegate &extensionDelegate, FName extensionHook, const TSharedPtr<FUICommandList> &CommandList, EExtensionHook::Position position)
{
	MenuExtender->AddMenuExtension(extensionHook, position, CommandList, extensionDelegate);
}

void FEditorGUIDGeneratorModule::MakePulldownMenu(FMenuBarBuilder &menuBuilder)
{
	menuBuilder.AddPullDownMenu(
		FText::FromString("GUID"),
		FText::FromString("GUID Generation"),
		FNewMenuDelegate::CreateRaw(this, &FEditorGUIDGeneratorModule::FillPulldownMenu),
		"Example",
		FName(TEXT("ExampleMenu"))
	);
}

void FEditorGUIDGeneratorModule::FillPulldownMenu(FMenuBuilder &menuBuilder)
{
	// just a frame for tools to fill in
	menuBuilder.BeginSection("ExampleSection", FText::FromString("Generation"));
	menuBuilder.AddMenuSeparator(FName("Section_1"));
	menuBuilder.EndSection();
}


#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FEditorGUIDGeneratorModule, EditorGUIDGenerator)
