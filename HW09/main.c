#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> 
#include "log.h"

void* some_work(void* arg) 
{
    for(int i = 0; i < 2; ++i) 
    {
        puts(arg);
        char * k;
        sleep(1);  
        k = malloc(1232132131232132131);
        if (k == NULL){
            LOG_ERROR("malloc k error!");
        }
        sleep(2);
    }
    return NULL;
}
int main() {

    logger_init("log2.txt");
    LogLevel level = logger_getLevel();
    if (level < LogLevel_INFO) {
        printf("fff");
    }
    logger_setLevel(LogLevel_DEBUG);
    char * h;
    h = malloc(1232132131232132131);
    if (h == NULL){
        LOG_ERROR("malloc h error!");
    }
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, some_work, "Hello World1" );
    pthread_create(&thread2, NULL, some_work, "Hello World2" );
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("End...\n");
    char * b = "Ffd";
    LOG_INFO("console %s logging", "test");
    int l = logger_getLevel();
    //logger_setLevel(LogLevel_DEBUG);
    LOG_DEBUG("format example: %d%c%s", 1, '2', "3");
    LogLevel f = LogLevel_WARN;
    printf("Test print %p\n", b);
    printf("f: %d\n", f);
    printf("l: %d\n", l);
    return 0;
 }
