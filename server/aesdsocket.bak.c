#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>

#define FILE_PATH /var/tmp/aesdsocketdata
#define PORT 9000
#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("testing: " msg "\n", ##__VA_ARGS__)



//syslog
//signal handling
//socket
//malloc
//uinstd

int main(int argc, char** argv){
	int server_fd, new_socket;
	ssize_tt valread;
	struct sockaddr_in address
	int opt = 1;
	socklen_t addrlen = sizeof(address);
	char buffer[1024] = {0};
	const char* hello = "Hello from server"

	//creating socket file descriptor
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		//syslog and exit
	}

	
	//Allows reuse of the port and address. 
	//Prevents errors like “address already in use” when restarting the server.
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));



	if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1){
		//syslog
	}
	if(listen(server_fd, 3) == -1){
		//syslog
	}
	if((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) == -1){
		//syslog
	}


	syslog(,"Accepted connection from %d", address.ipv4);

	valread = read(new_socket, buffer, strlen(buffer)-2);
	buffer[strlen(buffer)-1] = '\n';
	ERROR_LOG("%s\n", buffer);
	int fd = open(FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(fd == -1){
		syslog(LOG_ERR, "Failed to open file: %s", FILE_PATH);
		closelog();
		close(fd);
		exit(1);
	}
	
	ssize_t bytes_written = write(fd, buffer, strlen(buffer));

	if(bytes_written == -1){
		syslog(LOG_ERR, "Failed to write to file: %s", buffer);
		closelog();
		close(fd);
		exit(2);
	}
	
	
	syslog(LOG_SUCCEED, "Complete writting msg to file: %s", FILE_PATH);
	ERROR_LOG("%complete writing\n");
	//close(fd);
	//i should return the file save in aesdsocketdata so maybe do not have to close fd afte rwritng
	int bytes_read = read(fd, buffer2, strlen(buffer2));
	if(byts_read == -1){
		syslog(LOG_ERR, "Failed to read from file: %s", FILE_PATH);
		closelog();
		close(fd);
		exit(3);
	}

	send(new_socket, buffer2, strlen(buffer2), 0);

	syslog(,"Closed connection from %d", address.ipv4);
	close(new_socket);
	close(server_fd);

	return 0;
}
