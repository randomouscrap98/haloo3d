#ifndef __HVC_LOG_H
#define __HVC_LOG_H

// If set, error codes are output as strings rather than
// as codes. This increases the exe size considerably
// #define LOWLOGTOSTDOUT

// Change what is logged (which levels)
#define ERRORLOG
#define WARNLOG
#define DEBUGLOG
#define TRACELOG

// This isn't really the way you're supposed to do this but
#include <stdio.h>
#include <stdlib.h>

#define logfatal(...) { printf("FATAL: " __VA_ARGS__); printf("\n"); exit(1); }

#ifdef ERRORLOG
#define logerror(...) { printf("ERR: " __VA_ARGS__); printf("\n"); }
#else
#define logerror(...)
#endif
#ifdef WARNLOG
#define logwarn(...) { printf("WRN: " __VA_ARGS__); printf("\n"); }
#else
#define logwarn(...)
#endif
#ifdef DEBUGLOG
#ifdef LOWLOGTOSTDOUT
#define logdebug(...) { fprintf(stdout, "DBG: " __VA_ARGS__); fprintf(stdout, "\n"); }
#else
#define logdebug(...) { fprintf(stderr, "DBG: " __VA_ARGS__); fprintf(stderr, "\n"); }
#endif
#else
#define logdebug(...)
#endif
#ifdef TRACELOG
#ifdef LOWLOGTOSTDOUT
#define logtrace(...) { fprintf(stdout, "TRC: " __VA_ARGS__); fprintf(stdout, "\n"); }
#else
#define logtrace(...) { fprintf(stderr, "TRC: " __VA_ARGS__); fprintf(stderr, "\n"); }
#endif
#else
#define logtrace(...)
#endif

#endif
