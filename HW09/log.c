#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <execinfo.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "log.h"

#define MaxLogFileNameLen 255
#define BUF_SIZE 100

static volatile int logger_activate;
static volatile LogLevel s_logLevel = LogLevel_INFO;
static pthread_mutex_t s_mutex;

static struct {
    FILE* output;
    char filename[MaxLogFileNameLen + 1]; //+1 for \0
} s_log;


void logger_setLevel(LogLevel level)
{
    s_logLevel = level;
}

LogLevel logger_getLevel(void)
{
    return s_logLevel;
}

int logger_levelenough(LogLevel level)
{
    return s_logLevel <= level;
}

static char * getLevel(LogLevel level)
{
    switch (level) {
        case LogLevel_DEBUG: return "DEBUG";
        case LogLevel_INFO:  return "INFO";
        case LogLevel_WARN:  return "WARN";
        case LogLevel_ERROR: return "ERROR";
        default: return "";
    }
}

int logger_init(const char * filename){
    if (logger_activate) {
        fprintf(stderr, "logger allready initializated!");
        return 1;
    }
    if (filename == NULL) {
        assert(0 && "filename must not be NULL");
        return 0;
    }
    if (strlen(filename) > MaxLogFileNameLen) {
        assert(0 && "filename exceeds the maximum number of characters");
        return 0;
    }
    pthread_mutex_init(&s_mutex, NULL);
    pthread_mutex_lock(&s_mutex);
    if (s_log.output != NULL) {
        fclose(s_log.output);
    }
    s_log.output = fopen(filename, "a");
    if (s_log.output == NULL) {
        fprintf(stderr, "ERROR: logger: Failed to open file: `%s`\n", filename);
        pthread_mutex_unlock(&s_mutex);
        return 0;
    }
    strncpy(s_log.filename, filename, sizeof(filename)+1);

    logger_activate = 1;
    pthread_mutex_unlock(&s_mutex);
    return 1;
}

static void logger_stacktraceprint(FILE* fp){
    int nptrs;
    void *buffer[BUF_SIZE];
    char **strings;
    nptrs = backtrace(buffer, BUF_SIZE);
    fprintf(fp, "\tbacktrace() returned %d addresses\n", nptrs);
    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        return;
    }
    for (int j = 0; j < nptrs; j++)
        fprintf(fp, "\t%s\n", strings[j]);
    free(strings);
}

static void write_log(FILE* fp, char * levelstr, int threadID,
        const char* file, int line, const char* fmt, va_list arg){
    fprintf(fp, "%s: threadId:%d %s:%d: ", levelstr, threadID, file, line);
    vfprintf(fp, fmt, arg);
    fprintf(fp, "\n");
    if (!strcmp(levelstr, "ERROR")){
        logger_stacktraceprint(fp);
    }
    return;
}

void logger_log(LogLevel level, const char* file, int line, const char* fmt, ...){

    char * levelstr;
    va_list arg;
    int threadID;

    if (!logger_activate) {
        assert(0 && "logger is not initialized");
        return;
    }

    if (!logger_levelenough(level)) {
        return;
    }
    threadID = syscall(SYS_gettid);
    pthread_mutex_lock(&s_mutex);
    levelstr = getLevel(level);

    va_start(arg, fmt);
    write_log(s_log.output, levelstr, threadID, file, line, fmt, arg);
    va_end(arg);

    pthread_mutex_unlock(&s_mutex);
}

void logger_exit(){
    pthread_mutex_destroy(&s_mutex);
    if (s_log.output) {
        fclose(s_log.output);
    }
}