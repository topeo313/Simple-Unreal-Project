#pragma once
#include <string>

class LoggerHelper
{
public:
	static void LogOutput(const FString& message, ELogVerbosity::Type Verbosity = ELogVerbosity::Warning);
};
