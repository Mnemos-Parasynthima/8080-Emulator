#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Error.h"


static char buffer[150];

static char* errnames[ERR_AEF+1] = {
	"MEMORY ERROR",
	"SYMBOL REDEFINITION ERROR",
	"INVALID TOKEN ERROR",
	"MISSING TOKEN ERROR",
	"AEF ERROR"
};

static void formatMessage(const char* fmsg, va_list args) {
	vsnprintf(buffer, 150, fmsg, args);
}

void handleError(errType err, sevType sev, const char* fmsg, ...) {
	va_list args;
	va_start(args, fmsg);

	formatMessage(fmsg, args);

	if (sev == FATAL) {
		fprintf(stdout, RED "%s: %s" RESET, errnames[err], buffer);
    exit(-1);
	}
}