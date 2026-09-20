#include "C:\Users\Tope\Documents\Unreal Projects\SimpleGame\Intermediate\Build\Win64\x64\SimpleGameEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.ValExpApi.Cpp20.h"
#include "LoggerHelper.h"
#include "Logging/LogMacros.h"

void LoggerHelper::LogOutput(const FString& message, ELogVerbosity::Type Verbosity)
{
	GLog->Log(LogTemp.GetCategoryName(), Verbosity, *message);
}