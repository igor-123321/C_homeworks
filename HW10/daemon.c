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

#define LOCKFILE "/tmp/my-daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define SOCKETNAME "/tmp/server.socket"
#define MAXCLIENTS 2


static int running = 0;
static char *conf_file_name = NULL;
static int pid_fd = -1;
static char *app_name = NULL;
static char *sizefilename = NULL;

int read_conf_file(int reload)
{
	const char * tmpstr;
	config_t cfg;
	int ret = -1;
	if (conf_file_name == NULL) {
		syslog(LOG_ERR, "Config file not set!");
		return(EXIT_FAILURE);
		}
	if (access(conf_file_name, F_OK) < 0) {
    	syslog(LOG_ERR, "Config file %s not found", conf_file_name);
		return(EXIT_FAILURE);
	} 
	config_init(&cfg);
	/* Read file.*/
  	if(!config_read_file(&cfg, conf_file_name)) {
    		syslog(LOG_ERR, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    		config_destroy(&cfg);
    		return(EXIT_FAILURE);
  		}
	ret = config_lookup_string(&cfg, "settings.size_file", &tmpstr);
	if(ret) {	
		sizefilename = strdup(tmpstr);
        syslog(LOG_INFO, "%s=%s\n", "size_file", sizefilename);
    }
    else {
        syslog(LOG_ERR, "Param size_file in conf file not found!\n");
    }

	if (ret > 0) {
		if (reload == 1) {
			syslog(LOG_INFO, "Reloaded configuration file %s of %s",
				conf_file_name,
				app_name);
		} else {
			syslog(LOG_INFO, "Configuration has been read successfully from file %s",
				conf_file_name);
		}
	}
	config_destroy(&cfg);
	return ret;
}

/**
 * \brief Callback function for handling signals.
 * \param sig identifier of signal
 */
void handle_signal(int sig)
{
	if (sig == SIGINT) {
		syslog(LOG_INFO, "Stopping daemon ...\n");
		/* Unlock and close lockfile */
		if (pid_fd != -1) {
			lockf(pid_fd, F_ULOCK, 0);
			close(pid_fd);
		}
		/* delete lockfile */
		unlink(LOCKFILE);
		running = 0;
		/* Reset signal handling to default behavior */
		signal(SIGINT, SIG_DFL);
	} else if (sig == SIGHUP) {
		syslog(LOG_INFO, "Reloading daemon config file ...\n");
		read_conf_file(1);
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
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	/* Set new file permissions */
	umask(0);
	/* Change the working directory to the root directory */
	chdir("/");
	/* Close all open file descriptors */
	for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
		close(fd);
	}
	/* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
	stdin = fopen("/dev/null", "r");
	stdout = fopen("/dev/null", "w+");
	stderr = fopen("/dev/null", "w+");
	/* write PID of daemon to lockfile */
	char str[256];
	pid_fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
	if (pid_fd < 0) {
		syslog(LOG_ERR, "Can not open pid file: %s, error: %s",
			LOCKFILE, strerror(errno));
		/* Can't open lockfile */
		exit(EXIT_FAILURE);
	}
	if (lockfile(pid_fd)<0) {
		syslog(LOG_ERR, "Can't lock file %s, error: %s. Allready running ?",
			LOCKFILE, strerror(errno));
		/* Can't lock file */
		exit(EXIT_FAILURE);
	}
	/* Get current PID */
	sprintf(str, "%d\n", getpid());
	/* Write PID to lockfile */
	write(pid_fd, str, strlen(str));
	
}

void print_help(void)
{
	printf("\n Usage: %s [OPTIONS]\n\n", app_name);
	printf("  Options:\n");
	printf("   -h --help                 Print this help\n");
	printf("   -c --conf_file filename   Read configuration from the file\n");
	printf("   -d --daemon               Daemonize this application\n");
	printf("   -f --size_file  filename  The file you want to get the size of \n");
	printf("\n");
}

void run_server(char * sizefilename){
	int sock, msgsock;
    struct sockaddr_un server;
    struct stat st;
    char buf[1024];
    //const char * filename;
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
		syslog(LOG_ERR, "Can`t create new socket, terminating...");
        exit(EXIT_FAILURE);
    }
    server.sun_family = AF_UNIX;
    strncpy(server.sun_path, SOCKETNAME, strlen(SOCKETNAME)+1);
    if (bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un))) {
		syslog(LOG_ERR, "Can`t bind socket, terminating...");
        exit(EXIT_FAILURE);
    }
	syslog(LOG_INFO, "Starting server with socket: %s\n", server.sun_path);
    listen(sock, MAXCLIENTS);
    for (;;) {
        msgsock = accept(sock, 0, 0);
        if (msgsock == -1)
        {
			syslog(LOG_ERR, "Exception while accept");
            break;
        }
        else {
            bzero(buf, sizeof(buf));

            int ret = stat(sizefilename, &st);
            if (ret < 0) {
                sprintf(buf, "Can`t read file size. %s\n", strerror(errno));
            } 
            else {
                off_t size = st.st_size;
                sprintf(buf, "Size of file %s is %ld bytes\n", sizefilename, size);
                }
            send(msgsock, buf, strlen(buf), 0);
        } 
        close(msgsock);
    }
    close(sock);
    unlink(SOCKETNAME);
}

int main(int argc, char *argv[])
{
	static struct option long_options[] = {
		{"conf_file", required_argument, 0, 'c'},
		{"help", no_argument, 0, 'h'},
		{"daemon", no_argument, 0, 'd'},
		{"size_file", required_argument, 0, 'f'},
		{NULL, 0, 0, 0}
	};
	int value, option_index = 0;
	int start_daemonized = 0;

	app_name = argv[0];

	/* Try to process all command line arguments */
	while ((value = getopt_long(argc, argv, "c:f:dh", long_options, &option_index)) != -1) {
		switch (value) {
			case 'c':
				conf_file_name = strdup(optarg);
				break;
			case 'f':
				sizefilename = strdup(optarg);
				break;
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
	openlog(app_name, LOG_PID|LOG_CONS, LOG_DAEMON);
	syslog(LOG_INFO, "Started %s", app_name);

	/* Read configuration from config file */
	read_conf_file(0);

	/* Daemon will handle two signals */
	struct sigaction a;
	a.sa_handler = handle_signal;
	sigemptyset( &a.sa_mask );
	a.sa_flags = 0;
	sigaction( SIGINT, &a, NULL );
	sigaction( SIGHUP, &a, NULL );

	/* When daemonizing is requested at command line. */
	if (start_daemonized == 1) {
		daemonize();
	}

	/* This global variable can be changed in function handling signal */
	running = 1;
	if (sizefilename == NULL) {
		syslog(LOG_ERR, "Size_file not set! Set it in conf file or use -f option");
		print_help();
        exit(EXIT_FAILURE);
	}

	while (running == 1) {
		run_server(sizefilename);
	}

	/* Write system log and close it. */
	syslog(LOG_INFO, "Stopped %s", app_name);
	closelog();

	/* Free allocated memory */
	if (conf_file_name != NULL) free(conf_file_name);
	free(sizefilename);

	return EXIT_SUCCESS;
}