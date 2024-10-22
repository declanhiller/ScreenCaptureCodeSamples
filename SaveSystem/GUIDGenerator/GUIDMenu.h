#pragma once
#include "IGUIDModuleInterface.h"

class FGUIDMenu : public IGUIDModuleListenerInterface, public TSharedFromThis<FGUIDMenu>
{
public:
	virtual ~FGUIDMenu() {}

	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;

	void MakeMenuEntry(FMenuBuilder &menuBuilder);

protected:
	TSharedPtr<FUICommandList> CommandList;

	void MapCommands();

	// UI Command functions
	void RegenerateAll();

	void RegenerateNonValid();
};
