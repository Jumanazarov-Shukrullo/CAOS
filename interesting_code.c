#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
 
void error(const char* msg) { 
        perror(msg); 
        exit(1); 
}
int main(int argc, char* argv[]) { 
        if (argc < 3) { 
                fprintf(stderr, "usage %s hostname port \n", argv[0]); 
                exit(1); 
        } 
        int sockfd,  port_number, fd; 
        char buffer[255]; 
        struct sockaddr_in server_address; 
        struct hostent *server; 
        sockfd = socket(AF_INET,SOCK_STREAM, 0); 
        if (sockfd < 0) 
                error("Error occured in opening socket\n"); 
        port_number = atoi(argv[2]); 
 
        server = gethostbyname(argv[1]); 
        if (server == NULL) 
                fprintf(stderr, "Error\n"); 
        bzero((char*) &server_address, sizeof(server_address)); 
        server_address.sin_family = AF_INET; 
        bcopy((char*) server->h_addr, (char*) &server_address.sin_addr.s_addr, server -> h_length); 
        server_address.sin_port = htons(port_number); 
        while(1) { 
                bzero(buffer, 255); 
                fgets(buffer, 255, stdin); 
                fd = write(sockfd, buffer, 255); 
                if (fd < 0) 
                        error("Error in writing\n"); 
                bzero(buffer, 255); 
                fd = read(sockfd, buffer, 255); 
                if (fd < 0) 
                        error("Error in reading\n"); 
                printf("Server: %s", buffer); 
                int cmp = strncmp("Bye", buffer, 3); 
                if (cmp == 0) 
                        break; 
        } 
        close(sockfd); 
        return 0; 
}
