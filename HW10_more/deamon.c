#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>

void daemonize(const char* cmd)
{
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    //syslog(LOG_CRIT, "!!!!!!!!!!!!!!!!");
    umask(0);

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    perror("невозможно получить максимальный номер дескриптора");
    pid_t pid;
    if ((pid = fork()) < 0)
        perror("ошибка вызова функции fork");
    else if (pid != 0) /* родительский процесс */
        exit(EXIT_SUCCESS);
    //syslog(LOG_CRIT, "Before set sid");
    setsid();
    //syslog(LOG_CRIT, "After set sid");
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        syslog(LOG_CRIT, "невозможно игнорировать сигнал SIGHUP");
    if ((pid = fork()) < 0)
        syslog(LOG_CRIT, "ошибка вызова функции fork");
    else if (pid != 0) /* родительский процесс */
        exit(EXIT_SUCCESS);
/*
* Назначить корневой каталог текущим рабочим каталогом,
* чтобы впоследствии можно было отмонтировать файловую систему.
*/
    if (chdir("/") < 0)
        syslog(LOG_CRIT, "невозможно сделать текущим рабочим каталогом /");
/*
* Закрыть все открытые файловые дескрипторы.
*/
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (int i = 0; i < rl.rlim_max; i++)
        close(i);
    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
        syslog(LOG_CRIT, "ошибочные файловые дескрипторы %d %d %d", fd0, fd1, fd2);
}

int main() {
    printf("lalal\n");
    daemonize("my_mega_deamon");
}