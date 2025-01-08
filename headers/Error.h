#ifndef _ERROR_H
#define _ERROR_H

#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define PURPLE "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"

typedef enum {
	ERR_MEM,
	ERR_REDEFINED,
	ERR_INVALID,
	ERR_MISSING_TOKEN
} errType;

typedef enum {
	FATAL,
	WARNING
} sevType;

void handleError(errType err, sevType sev, const char* fmsg, ...);

#endif