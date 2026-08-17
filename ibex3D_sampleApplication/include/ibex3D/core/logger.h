#pragma once

// #define I3D_LOG_INFOS
#define I3D_LOG_WARNINGS
#define I3D_LOG_ERRORS

int i3D_logInfoMessage(const char* fmt, ...);
int i3D_logWarningMessage(const char* fmt, ...);
int i3D_logErrorMessage(const char* fmt, ...);