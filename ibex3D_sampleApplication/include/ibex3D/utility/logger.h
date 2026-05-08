#pragma once

enum class logLevel
{
	Info,
	Warning,
	Error
};

class logger
{
public:
	static void logError(const char* description, const char* file, size_t line);
	static void logWarning(const char* description, const char* file, size_t line);
	static void logInfo(const char* description, const char* file, size_t line);
};