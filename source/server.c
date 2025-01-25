#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#define BASE 10
#define TOTALAPLHA 26
#define S_PATH "/tmp/socket"
#define BACKLOG 5
#define BUFFER_SIZE 1026

void caesar_encrypt(char *message, int shift);

void main(int argc, char* argv[])
{
    int shift;
    size_t sun_path_max_length;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Shift> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *endptr;
    shift = strtol(argv[1], &endptr, 10);

    if (*endptr != '\0') 
    {
        fprintf(stderr, "Usage: %s <Shift Must be an integer> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
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


    if (access(S_PATH,F_OK ) == 0) 
    {
        unlink(S_PATH);
    }

    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("Binding error");
        exit(EXIT_FAILURE);
    }


    if (listen(server_socket, BACKLOG) == -1)
    {
        perror("Listen error");
        exit(EXIT_FAILURE);
    }

    printf("Listening...\n");

    int s_socket = accept(server_socket, NULL, NULL);
    if (s_socket == -1) {
        perror("Accept Error");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    bytes_read = recv(s_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_read == BUFFER_SIZE) {
        printf("Content of received message is too large\n");
        char* err_msg = "Content size too large - Server will only process up to 1024 bytes at a time.\n";
        strncpy(buffer, err_msg, BUFFER_SIZE - 1);
        buffer[BUFFER_SIZE - 1] = '\0';
    }else
    {
    buffer[bytes_read] = '\0';

        printf("Received Message: %s\n", buffer);
        printf("Bytes Read: %ld\n", bytes_read);

        caesar_encrypt(buffer, shift);
        printf("Encrypted Message: %s\n", buffer);
    }

    if (send(s_socket, buffer, strlen(buffer), 0) == -1) 
    {
        perror("Server Response Error");
        exit(EXIT_FAILURE);
    }

    //printf("Sending Encrypted Content: %s",buffer);

    close(s_socket);
    close(server_socket);
    if (unlink(S_PATH) == -1) 
    {
        perror("Unlink Err");
        exit(EXIT_FAILURE);
    }

}
void caesar_encrypt(char *message, int shift)
{
    int i;
    for(i = 0; message[i] != '\0'; i++)
    {
        if(message[i] >= 'a' && message[i] <= 'z')
        {
            message[i] = (char)(((message[i] - 'a' + shift) % TOTALAPLHA + TOTALAPLHA) % TOTALAPLHA + 'a');
        }
        else if(message[i] >= 'A' && message[i] <= 'Z')
        {
            message[i] = (char)(((message[i] - 'A' + shift) % TOTALAPLHA + TOTALAPLHA) % TOTALAPLHA + 'A');
        }
    }
}