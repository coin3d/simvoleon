#ifndef SIMVOLEON_DEBUG
#error The define SIMVOLEON_DEBUG needs to be defined to true or false
#endif

#if SIMVOLEON_DEBUG
#include "config-debug.h"
#else /* !SIMVOLEON_DEBUG */
#include "config-release.h"
#endif /* !SIMVOLEON_DEBUG */
