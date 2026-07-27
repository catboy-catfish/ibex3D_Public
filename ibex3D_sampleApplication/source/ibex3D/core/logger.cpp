#include <ibex3D/core/logger.h>

#include <stdio.h>
#include <stdarg.h>

// ----------------------------------------------------------------------------------------------------

int i3D_logInfoMessage(const char* fmt, ...)
{
#ifdef I3D_LOG_INFOS
	va_list vargs;
	va_start(vargs, fmt);

	int done = vfprintf(stdout, fmt, vargs);

	va_end(vargs);
	return done;
#endif

	return 0;
}

int i3D_logWarningMessage(const char* fmt, ...)
{
#ifdef I3D_LOG_WARNINGS
	va_list vargs;
	va_start(vargs, fmt);

	int done = vfprintf(stdout, fmt, vargs);

	va_end(vargs);
	return done;
#endif

	return 0;
}

int i3D_logErrorMessage(const char* fmt, ...)
{
#ifdef I3D_LOG_ERRORS
	va_list vargs;
	va_start(vargs, fmt);

	int done = vfprintf(stderr, fmt, vargs);

	va_end(vargs);
	return done;
#endif

	return 0;
}