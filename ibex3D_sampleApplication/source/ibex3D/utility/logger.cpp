#include <ibex3D/utility/logger.h>
#include <stdio.h>

void logger::log(const char* text)
{
	fprintf(stdout, text);
}

void logger::logError(const char* description, const char* file, size_t line)
{
	fprintf(stderr, "\nERROR (File: %s, line %zu) - %s\n", file, line, description);
}

void logger::logWarning(const char* description, const char* file, size_t line)
{
	fprintf(stdout, "\nWARNING (File: %s, line %zu) - %s\n", file, line, description);
}

void logger::logInfo(const char* description, const char* file, size_t line)
{
	fprintf(stdout, "\nINFO (File: %s, line %zu) - %s\n", file, line, description);
}
