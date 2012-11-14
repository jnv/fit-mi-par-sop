#ifndef COMMON_H_
#define COMMON_H_

#include <string>

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

#define INIT_PROC 0
#define LOG_OUTPUT _logFile // cerr
// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define logc(fmt) \
		do { fprintf(LOG_OUTPUT, fmt); fflush(LOG_OUTPUT); } while (0)
#define log(fmt, ...) \
        do { fprintf(LOG_OUTPUT, fmt, __VA_ARGS__); fflush(LOG_OUTPUT); } while (0)

#define isInitProc() (_thisRank == INIT_PROC)

#define CUT_LEVEL 6

#define CHECK_MSG_AMOUNT 100

enum State {IDLE, ACTIVE};

enum TokenColor {WHITE = 0, BLACK = 1};

std::string COLORS[] = {"WHITE", "BLACK"};

#endif /* COMMON_H_ */
