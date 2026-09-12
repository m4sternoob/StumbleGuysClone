// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FStumbleGuysCloneModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};