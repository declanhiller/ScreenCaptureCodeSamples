#pragma once

#include "CoreMinimal.h"
#include "IGUIDModuleInterface.h"
#include "Modules/ModuleManager.h"

class FEditorGUIDGeneratorModule : public IGUIDModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;


    virtual void AddModuleListeners() override;

    static inline FEditorGUIDGeneratorModule& Get()
    {
        return FModuleManager::LoadModuleChecked< FEditorGUIDGeneratorModule >("EditorGUIDGenerator");
    }

    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("EditorGUIDGenerator");
    }
    
public:
    void AddMenuExtension(const FMenuExtensionDelegate &extensionDelegate, FName extensionHook, const TSharedPtr<FUICommandList> &CommandList = NULL, EExtensionHook::Position position = EExtensionHook::Before);
    TSharedRef<FWorkspaceItem> GetMenuRoot() { return MenuRoot; };

protected:
    TSharedPtr<FExtensibilityManager> LevelEditorMenuExtensibilityManager;
    TSharedPtr<FExtender> MenuExtender;

    static TSharedRef<FWorkspaceItem> MenuRoot;

    void MakePulldownMenu(FMenuBarBuilder &menuBuilder);
    void FillPulldownMenu(FMenuBuilder &menuBuilder);
    
    
};
