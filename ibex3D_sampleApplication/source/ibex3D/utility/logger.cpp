#include <ibex3D/utility/logger.h>
#include <stdio.h>

void logger::log(const char* text)
{
	fprintf(stdout, text);
}

void logger::logError(const char* description, const char* file, size_t line)
{
	fprintf(stderr, "ERROR (File: %s, line %zu) - %s\n\n", file, line, description);
}

void logger::logWarning(const char* description, const char* file, size_t line)
{
	fprintf(stdout, "WARNING (File: %s, line %zu) - %s\n\n", file, line, description);
}

void logger::logInfo(const char* description, const char* file, size_t line)
{
	fprintf(stdout, "INFO (File: %s, line %zu) - %s\n\n", file, line, description);
}
