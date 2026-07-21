#pragma once

/*
	----------------------------------------------------------------------------------------------------
	- DEPRECATION NOTICE -------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------------

	The logger class is deprecated and will soon be removed because it's been clunky and difficult to
	use, and I've always been feeling uneasy about it since its introduction. Please use printf/fprintf
	(#included in <stdio.h>) or std::cout (#included in <iostream>) instead, unless if I re-implement
	the logger class in a better way or use a separate library for the same purpose in the future.
*/
class logger
{
public:
	static void log(const char* text);
	static void logInfo(const char* description, const char* file, size_t line);
	static void logWarning(const char* description, const char* file, size_t line);
	static void logError(const char* description, const char* file, size_t line);
};