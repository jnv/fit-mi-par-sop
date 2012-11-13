#include "ModifiedStack.h"

#ifndef COMMON_H_
#define COMMON_H_

#define DEBUG 1

#define PLACE_NONE -1
#define PLACE_TOMBSTONE -2

#define MAX_N 1024
#define MAX_LEN 1096

extern int n, c, a;
extern int _upperBound;
extern int * _inputSet;

extern int _thisRank;
extern int _procCnt;

extern FILE* _logFile;

#define LOG_OUTPUT _logFile // cerr
// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define logc(fmt) \
		do { fprintf(LOG_OUTPUT, fmt); fflush(LOG_OUTPUT); } while (0)
#define log(fmt, ...) \
        do { fprintf(LOG_OUTPUT, fmt, __VA_ARGS__); fflush(LOG_OUTPUT); } while (0)

enum State {IDLE, ACTIVE}

#endif /* COMMON_H_ */
