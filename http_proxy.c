#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

int client_fd, server_fd;
int new_server_fd;

void shutdown_proxy() {
    printf("\nShutting down...\n");
    if (client_fd != -1) {
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }
    if (server_fd != -1) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
    }
    if (new_server_fd != -1) {
        shutdown(new_server_fd, SHUT_RDWR);
        close(new_server_fd);
    }
    exit(0);
}

void handle_sigint(int signum) {
    shutdown_proxy();
}

void handle_sigterm(int signum) {
    shutdown_proxy();
}

void handle_signals() {
    signal(SIGPIPE, SIG_IGN);

    struct sigaction action_int = {
            .sa_handler = handle_sigint,
            .sa_flags = SA_RESTART,
    };
    sigaction(SIGINT, &action_int, NULL);

    struct sigaction action_term = {
            .sa_handler = handle_sigterm,
            .sa_flags = SA_RESTART
    };
    sigaction(SIGTERM, &action_term, NULL);
}

int main(int argc, char *argv[]) {
    handle_signals();
    printf("pid: %d\n", getpid());
    printf("Press ^C to exit\n\n");
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Error in opening socket\n");
        shutdown_proxy();
    }
    struct sockaddr_in server_address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = inet_addr("127.0.0.1"),
            .sin_port = htons(atoi(argv[2]))
    };
    if (-1 == connect(client_fd, (struct sockaddr *) (&server_address), sizeof(server_address))) {
        perror("Error in connect");
        shutdown_proxy();
    } else {
        printf("Connected to the server\n");
    }
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Error in opening socket\n");
        shutdown_proxy();
    }
    int setsockopt_value = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &setsockopt_value, sizeof(setsockopt_value));
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &setsockopt_value, sizeof(setsockopt_value));
    struct sockaddr_in client_address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons(atoi(argv[1]))
    };
    if (bind(server_fd, (struct sockaddr *) &client_address, sizeof(client_address)) == -1) {
        perror("Error in binding\n");
        shutdown_proxy();
    }
    listen(server_fd, 1);
    struct sockaddr_in client_address_len;
    socklen_t client_length;
    new_server_fd = accept(server_fd, (struct sockaddr *) &client_address_len, &client_length);
    if (new_server_fd == -1) {
        perror("Error in accepting\n");
        shutdown_proxy();
    }
    printf("Client connected\n");
    while (true) {
        char receive_mes_from_client[4096];
        memset(receive_mes_from_client, '\0', sizeof(receive_mes_from_client));
        if (read(new_server_fd, receive_mes_from_client, sizeof(receive_mes_from_client)) <= 0)
            break;
        printf("received %zu bytes from client\n", strlen(receive_mes_from_client));
        fflush(stdout);
        if (write(client_fd, receive_mes_from_client, sizeof(receive_mes_from_client)) <= 0)
            break;

        char receive_mes_from_server[4096];
        memset(receive_mes_from_server, '\0', sizeof(receive_mes_from_server));
        if (read(client_fd, receive_mes_from_server, sizeof(receive_mes_from_server)) <= 0)
            break;
        printf("received %zu bytes from server\n", strlen(receive_mes_from_server));
        fflush(stdout);
        if (write(new_server_fd, receive_mes_from_server, sizeof(receive_mes_from_server)) <= 0)
            break;
    }
    shutdown_proxy();
    return 0;
}
