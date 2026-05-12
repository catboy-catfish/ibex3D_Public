#pragma once

class logger
{
public:
	static void log(const char* text);

	static void logError(const char* description, const char* file, size_t line);
	static void logWarning(const char* description, const char* file, size_t line);
	static void logInfo(const char* description, const char* file, size_t line);
};