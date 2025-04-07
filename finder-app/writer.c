#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <stdlib.h>
int main(int argc, char *argv[]){
	if(argc != 3){
		fprintf(stderr, "Usage: %s <writefile> <writestr>\n", argv[0]);
			exit(1);
	}

	const char *writefile = argv[1];
	const char *writestr = argv[2];
	
	//open syslog with LOG_USER facility
	openlog("write", LOG_PID, LOG_USER);



	int fd = open(writefile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	
	if(fd == -1){
		syslog(LOG_ERR, "Failed to open file: %s", writefile);
		perror("open");
		closelog();
		close(fd);
		exit(1);
	}

	ssize_t bytes_written = write(fd, writestr, strlen(writestr));
	
	if(bytes_written == -1){
		syslog(LOG_ERR, "Failed to write to file: %s", writefile);
		perror("write");
		closelog();
		close(fd);
		exit(1);
	}
	
	syslog(LOG_ERR, "Failed to open file: %s", writefile);
	close(fd);
	closelog();

    return 0;
}
