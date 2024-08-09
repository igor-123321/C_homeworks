#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libconfig.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

static int running = 0;
static int delay = 1;
static int counter = 0;
static char *conf_file_name = NULL;
static char *pid_file_name = NULL;
static int pid_fd = -1;
static char *app_name = NULL;
static FILE *log_stream;
static char *sizefilename = NULL;

/**
 * \brief Read configuration from config file
 */
int read_conf_file(int reload)
{
	const char * tmpstr;
	config_t cfg;
	int ret = -1;
	config_init(&cfg); /* обязательная инициализация */

	
	if (conf_file_name == NULL) return 0;
	/* Читаем файл. Если ошибка, то завершаем работу */
  	if(!config_read_file(&cfg, conf_file_name)) //"daemon.conf"
  		{
    		syslog(LOG_ERR, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    		config_destroy(&cfg);
    		return(EXIT_FAILURE);
  		}
	ret = config_lookup_string(&cfg, "application.filename", &tmpstr);
	if(ret)
    {	
		sizefilename = strdup(tmpstr);
        syslog(LOG_INFO, "%s=%s\n", "application.filename", sizefilename);
    }
    else
    {
        syslog(LOG_ERR, "filename in conf file not found!\n");
    }
	ret = config_lookup_string(&cfg, "application.pid_file", &tmpstr);
	if(ret)
    {
		pid_file_name = strdup(tmpstr);
        syslog(LOG_INFO, "%s=%s\n", "application.pid_file", pid_file_name);
    }
    else
    {
        syslog(LOG_ERR, "pid_file in conf file not found!\n");
    }

	if (ret > 0) {
		if (reload == 1) {
			syslog(LOG_INFO, "Reloaded configuration file %s of %s",
				conf_file_name,
				app_name);
		} else {
			syslog(LOG_INFO, "Configuration of %s read from file %s",
				app_name,
				conf_file_name);
		}
	}

	config_destroy(&cfg);
	return ret;
}

/**
 * \brief This function tries to test config file
 */
int test_conf_file(char *_conf_file_name)
{
	const char * tmpstr;
	config_t cfg;
	int ret = -1;
	config_init(&cfg); /* обязательная инициализация */

	/* Читаем файл. Если ошибка, то завершаем работу */
  	if(!config_read_file(&cfg, _conf_file_name)) //"daemon.conf"
  		{
    		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    		config_destroy(&cfg);
    		return(EXIT_FAILURE);
  		}
	ret = config_lookup_string(&cfg, "application.filename", &tmpstr);

	if(ret)
    	{	
			sizefilename = strdup(tmpstr);
        	syslog(LOG_INFO, "%s=%s\n", "application.filename", sizefilename);
    	} else {
        	syslog(LOG_ERR, "filename in conf file not found!\n");
			return EXIT_FAILURE;
    	}
	ret = config_lookup_string(&cfg, "application.pid_file", &tmpstr);
	if(ret)
    	{
			pid_file_name = strdup(tmpstr);
        	syslog(LOG_INFO, "%s=%s\n", "application.pid_file", pid_file_name);
    	} else {
        	syslog(LOG_ERR, "pid_file in conf file not found!\n");
			return EXIT_FAILURE;
    	}

	if (ret > 0)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

/**
 * \brief Callback function for handling signals.
 * \param	sig	identifier of signal
 */
void handle_signal(int sig)
{
	if (sig == SIGINT) {
		fprintf(log_stream, "Debug: stopping daemon ...\n");
		/* Unlock and close lockfile */
		if (pid_fd != -1) {
			lockf(pid_fd, F_ULOCK, 0);
			close(pid_fd);
		}
		/* Try to delete lockfile */
		if (pid_file_name != NULL) {
			unlink(pid_file_name);
		}
		running = 0;
		/* Reset signal handling to default behavior */
		signal(SIGINT, SIG_DFL);
	} else if (sig == SIGHUP) {
		fprintf(log_stream, "Debug: reloading daemon config file ...\n");
		read_conf_file(1);
	} else if (sig == SIGCHLD) {
		fprintf(log_stream, "Debug: received SIGCHLD signal\n");
	}
}

int lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return(fcntl(fd, F_SETLK, &fl));
}

/**
 * \brief This function will daemonize this app
 */
static void daemonize()
{
	pid_t pid = 0;
	int fd;

	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* On success: The child process becomes session leader */
	if (setsid() < 0) {
		exit(EXIT_FAILURE);
	}

	/* Ignore signal sent from child to parent process */
	signal(SIGCHLD, SIG_IGN);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Set new file permissions */
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/");

	/* Close all open file descriptors */
	for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
		close(fd);
	}

	/* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
	stdin = fopen("/dev/null", "r");
	stdout = fopen("/dev/null", "w+");
	stderr = fopen("/dev/null", "w+");
	syslog(LOG_INFO, "pid_file_name is: %s", pid_file_name);
	
	/* Try to write PID of daemon to lockfile */
	if (pid_file_name != NULL)
	{
		char str[256];
		syslog(LOG_ERR, "BEFORE OPEN");
		pid_fd = open(pid_file_name, LOCKMODE);
		syslog(LOG_INFO, "pid_fd: %d", pid_fd);
		if (pid_fd < 0) {
			syslog(LOG_ERR, "Can not open pid file: %s, error: %s",
				pid_file_name, strerror(errno));
			/* Can't open lockfile */
			exit(EXIT_FAILURE);
		}
		if (lockfile(pid_fd)<0) {
			syslog(LOG_ERR, "Can't lock file %s, error: %s. Allready running ?",
				pid_file_name, strerror(errno));
			/* Can't lock file */
			exit(EXIT_FAILURE);
		}
		/* Get current PID */
		sprintf(str, "%d\n", getpid());
		/* Write PID to lockfile */
		ssize_t res;
		res = write(pid_fd, str, strlen(str));
		syslog(LOG_INFO, "Write %ld bytes", res);
	}
}

/**
 * \brief Print help for this application
 */
void print_help(void)
{
	printf("\n Usage: %s [OPTIONS]\n\n", app_name);
	printf("  Options:\n");
	printf("   -h --help                 Print this help\n");
	printf("   -c --conf_file filename   Read configuration from the file\n");
	printf("   -t --test_conf filename   Test configuration file\n");
	printf("   -l --log_file  filename   Write logs to the file\n");
	printf("   -d --daemon               Daemonize this application\n");
	printf("   -p --pid_file  filename   PID file used by daemonized app\n");
	printf("\n");
}

/* Main function */
int main(int argc, char *argv[])
{
	static struct option long_options[] = {
		{"conf_file", required_argument, 0, 'c'},
		{"test_conf", required_argument, 0, 't'},
		{"log_file", required_argument, 0, 'l'},
		{"help", no_argument, 0, 'h'},
		{"daemon", no_argument, 0, 'd'},
		{"pid_file", required_argument, 0, 'p'},
		{NULL, 0, 0, 0}
	};
	int value, option_index = 0, ret;
	char *log_file_name = NULL;
	int start_daemonized = 0;

	app_name = argv[0];

	/* Try to process all command line arguments */
	while ((value = getopt_long(argc, argv, "c:l:t:p:dh", long_options, &option_index)) != -1) {
		switch (value) {
			case 'c':
				conf_file_name = strdup(optarg);
				break;
			case 'l':
				log_file_name = strdup(optarg);
				break;
			case 'p':
				pid_file_name = strdup(optarg);
				break;
			case 't':
				return test_conf_file(optarg);
			case 'd':
				start_daemonized = 1;
				break;
			case 'h':
				print_help();
				return EXIT_SUCCESS;
			case '?':
				print_help();
				return EXIT_FAILURE;
			default:
				break;
		}
	}

	
	/* Open system log and write message to it */
	openlog("my_daemon", LOG_PID|LOG_CONS, LOG_DAEMON);
	syslog(LOG_INFO, "Started %s", app_name);

	/* Read configuration from config file */
	read_conf_file(0);

	/* Daemon will handle two signals */
	signal(SIGINT, handle_signal);
	signal(SIGHUP, handle_signal);

	/* When daemonizing is requested at command line. */
	if (start_daemonized == 1) {
		/* It is also possible to use glibc function deamon()
		 * at this point, but it is useful to customize your daemon. */
		daemonize();
	}

	/* Try to open log file to this daemon */
	if (log_file_name != NULL) {
		log_stream = fopen(log_file_name, "a+");
		if (log_stream == NULL) {
			syslog(LOG_ERR, "Can't lock file: %s, error: %s",
				log_file_name, strerror(errno));
			log_stream = stdout;
		}
	} else {
		log_stream = stdout;
	}

	/* This global variable can be changed in function handling signal */
	running = 1;

	/* Never ending loop of server */
	while (running == 1) {
		/* Debug print */
		ret = fprintf(log_stream, "Debug: %d\n", counter++);
		if (ret < 0) {
			syslog(LOG_ERR, "Can not write to log stream: %s, error: %s",
				(log_stream == stdout) ? "stdout" : log_file_name, strerror(errno));
			break;
		}
		ret = fflush(log_stream);
		if (ret != 0) {
			syslog(LOG_ERR, "Can not fflush() log stream: %s, error: %s",
				(log_stream == stdout) ? "stdout" : log_file_name, strerror(errno));
			break;
		}

		/* TODO: dome something useful here */

		/* Real server should use select() or poll() for waiting at
		 * asynchronous event. Note: sleep() is interrupted, when
		 * signal is received. */
		sleep(delay);
	}

	/* Close log file, when it is used. */
	if (log_stream != stdout) {
		fclose(log_stream);
	}

	/* Write system log and close it. */
	syslog(LOG_INFO, "Stopped %s", app_name);
	closelog();

	/* Free allocated memory */
	if (conf_file_name != NULL) free(conf_file_name);
	if (log_file_name != NULL) free(log_file_name);
	if (pid_file_name != NULL) free(pid_file_name);

	return EXIT_SUCCESS;
}