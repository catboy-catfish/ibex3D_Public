#include <ibex3D/utility/logger.h>
#include <stdio.h>

// - Functions ----------------------------------------------------------------------------------------

void logger::log(const char* text)
{
	fprintf(stdout, text);
}

void logger::logInfo(const char* description, const char* file, size_t line)
{
#ifdef IBEX3D_LOG_INFO
	fprintf(stdout, "\nINFO (File: %s, line %zu) - %s\n", file, line, description);
#endif
}

void logger::logWarning(const char* description, const char* file, size_t line)
{
#ifdef IBEX3D_LOG_WARNINGS
	fprintf(stderr, "\nWARNING (File: %s, line %zu) - %s\n", file, line, description);
#endif
}

void logger::logError(const char* description, const char* file, size_t line)
{
#ifdef IBEX3D_LOG_ERRORS
	fprintf(stderr, "\nERROR (File: %s, line %zu) - %s\n", file, line, description);
#endif
}