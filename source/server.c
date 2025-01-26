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
#define TOTALAPLHA 26
#define S_PATH "/tmp/socket"
#define BACKLOG 5
#define BUFFER_SIZE 1024

void caesar_encrypt(char *message, int shift);
void handle_signal(int signal);
void cleanup();

int server_socket;

void main(int argc, char* argv[])
{
    int shift;
    size_t sun_path_max_length;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Shift>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *endptr;
    shift = strtol(argv[1], &endptr, 10);

    if (*endptr != '\0')
    {
        fprintf(stderr, "Usage: %s <Shift Must be an integer>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;

    signal(SIGINT, handle_signal);

    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    sun_path_max_length = sizeof(addr.sun_path);
    strncpy(addr.sun_path, S_PATH, sun_path_max_length);

    if (access(S_PATH, F_OK) == 0)
    {
        unlink(S_PATH);
    }

    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("Binding error");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, BACKLOG) == -1)
    {
        perror("Listen error");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Listening...\n\n");

    while (1)
    {
        printf("Waiting for a connection...\n\n");
        int s_socket = accept(server_socket, NULL, NULL);
        if (s_socket == -1)
        {
            perror("Accept Error");
            continue;
        }

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read;
        bool done_receiving = false;  
        printf("Client connected.\n");
        while (!done_receiving)
        {
            bytes_read = recv(s_socket, buffer, BUFFER_SIZE, 0);

            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0';  

                caesar_encrypt(buffer, shift);

                send(s_socket, buffer, strlen(buffer), 0); 

                
                if (bytes_read < BUFFER_SIZE)
                {
                    done_receiving = true;
                }
            }
            else if (bytes_read == 0)
            {
                done_receiving = true;
                printf("Client closed the connection.\n");
            }
            else
            {
                perror("Receive error");
                done_receiving = true;
            }
        }

        printf("Finished sending data.\n");
        close(s_socket);
        printf("Client Disconnected.\n\n");
    }

    cleanup();
}

void caesar_encrypt(char *message, int shift)
{
    int i;
    for (i = 0; message[i] != '\0'; i++)
    {
        if (message[i] >= 'a' && message[i] <= 'z')
        {
            message[i] = (char)(((message[i] - 'a' + shift) % TOTALAPLHA + TOTALAPLHA) % TOTALAPLHA + 'a');
        }
        else if (message[i] >= 'A' && message[i] <= 'Z')
        {
            message[i] = (char)(((message[i] - 'A' + shift) % TOTALAPLHA + TOTALAPLHA) % TOTALAPLHA + 'A');
        }
    }
}

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        printf("\nCaught SIGINT (Ctrl+C). Shutting down gracefully...\n");
        cleanup();
        exit(EXIT_SUCCESS);
    }
}

void cleanup()
{
    if (server_socket != -1)
    {
        close(server_socket);
        printf("Server socket closed gracefully\n");
    }
    if (unlink(S_PATH) == -1)
    {
        perror("Error unlinking socket file");
    }
}
