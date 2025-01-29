#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>

#define BASE 10
#define TOTALALPHA 26
#define S_PATH "/tmp/socket"
#define BACKLOG 5
#define BUFFER_SIZE 1024

// Global server socket
int server_socket = -1;

// Function Prototypes
void validate_arguments(int argc, char* argv[], int* shift);
int setup_server_socket();
void handle_signal(int signal);
void cleanup();
void accept_client_connections(int server_socket, int shift);
void process_client_message(int client_socket, int shift);
void caesar_encrypt(char *message, int shift);

int main(int argc, char* argv[]) {
    int shift;
    validate_arguments(argc, argv, &shift);

    signal(SIGINT, handle_signal);

    server_socket = setup_server_socket();
    accept_client_connections(server_socket, shift);

    cleanup();
    return 0;
}

// Validate command-line arguments
void validate_arguments(int argc, char* argv[], int* shift) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Shift>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *endptr;
    *shift = strtol(argv[1], &endptr, 10);

    if (*endptr != '\0') {
        fprintf(stderr, "ERR: Shift must be an integer\n");
        exit(EXIT_FAILURE);
    }
}

// Setup and return server socket
int setup_server_socket() {
    struct sockaddr_un addr;
    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("ERR: Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, S_PATH, sizeof(addr.sun_path) - 1);

    if (access(S_PATH, F_OK) == 0) {
        unlink(S_PATH);
    }

    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("ERR: Binding failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, BACKLOG) == -1) {
        perror("ERR: Listen failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server is listening...\nPress CTRL+C to terminate.\n\n");
    return server_socket;
}

// Handle SIGINT for graceful shutdown
void handle_signal(int signal) {
    if (signal == SIGINT) {
        printf("\nCaught SIGINT (Ctrl+C). Shutting down...\n");
        cleanup();
        exit(EXIT_SUCCESS);
    }
}

// Cleanup server resources
void cleanup() {
    if (server_socket != -1) {
        close(server_socket);
        printf("Server socket closed.\n");
    }
    if (unlink(S_PATH) == 0) {
        printf("Socket file unlinked.\n");
    } else {
        perror("ERR: Unlinking socket file failed");
    }
}

// Accept and process client connections
void accept_client_connections(int server_socket, int shift) {
    while (1) {
        printf("Waiting for a client...\n");

        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("ERR: Accept failed");
            continue;
        }

        printf("Client connected.\n");
        process_client_message(client_socket, shift);

        close(client_socket);
        printf("Client disconnected.\n\n");
    }
}

// Process client message: receive, encrypt, and send back
void process_client_message(int client_socket, int shift) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    bool done_receiving = false;

    while (!done_receiving) {
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            caesar_encrypt(buffer, shift);
            send(client_socket, buffer, strlen(buffer), 0);

            if (bytes_read < BUFFER_SIZE) {
                done_receiving = true;
            }
        } else if (bytes_read == 0) {
            printf("Client closed the connection.\n");
            done_receiving = true;
        } else {
            perror("ERR: Receive failed");
            done_receiving = true;
        }
    }

    printf("Finished sending encrypted data.\n");
}

// Perform Caesar cipher encryption
void caesar_encrypt(char *message, int shift) {
    for (int i = 0; message[i] != '\0'; i++) {
        if (message[i] >= 'a' && message[i] <= 'z') {
            message[i] = (char)((((message[i] - 'a' + shift) % TOTALALPHA + TOTALALPHA) % TOTALALPHA) + 'a');
        } else if (message[i] >= 'A' && message[i] <= 'Z') {
            message[i] = (char)((((message[i] - 'A' + shift) % TOTALALPHA + TOTALALPHA) % TOTALALPHA) + 'A');
        }
    }
}
