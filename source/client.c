#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>

#define FILE_NAME_SIZE 1024
#define S_PATH "/tmp/socket"
#define BUFFER_SIZE 1024

char* read_file(char* filename);
int get_file_size(char* filename);

void main(int argc, char* argv[])
{
    char file_name[FILE_NAME_SIZE];
    size_t sun_path_max_length;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strncpy(file_name, argv[1], FILE_NAME_SIZE - 1);
    file_name[FILE_NAME_SIZE - 1] = '\0';

    char* file_content = read_file(file_name);

    long file_size = get_file_size(file_name);
    if (file_size == -1)
    {
        printf("ERR: File not found\n");
        free(file_content);
        exit(EXIT_FAILURE);
    }

    if (file_size == 0)
    {
        printf("ERR: File is empty\n");
        free(file_content);
        exit(EXIT_FAILURE);
    }

    printf("File content:\n%s\n\n", file_content);
    printf("File size: %ld bytes\n\n", file_size);

    struct sockaddr_un addr;
    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation error");
        free(file_content);
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    sun_path_max_length = sizeof(addr.sun_path);
    strncpy(addr.sun_path, S_PATH, sun_path_max_length);

    if (connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        printf("ERR: Connection error. Server is not running\n");
        free(file_content);
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server...\n");

    if (send(client_socket, file_content, file_size, 0) == -1)
    {
        perror("Sending error");
        free(file_content);
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Message sent to the server.\n\n");
    free(file_content);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received;
    bool done_receiving = false;

    printf("Encrypted message received from the server:\n");

    while (!done_receiving)
    {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0'; 
            printf("%s", buffer);

            
            if (bytes_received < BUFFER_SIZE)
            {
                done_receiving = true; 
            }
        }
        else if (bytes_received == 0)
        {
            done_receiving = true; 
            printf("\nServer closed the connection.\n");
        }
        else
        {
            perror("Receiving error");
            done_receiving = true;
        }
    }

    printf("\nDiscnnected from the server\n");
    close(client_socket);
}

char* read_file(char* filename)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("ERR: File not found\n");
        return NULL;
    }

    file_size = get_file_size(filename);
    if (file_size <= 0)
    {
        fclose(file);
        return NULL;
    }

    buffer = (char*)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, file_size, file);
    fclose(file);
    return buffer;
}

int get_file_size(char* filename)
{
    FILE *file;
    long file_size;

    file = fopen(filename, "r");
    if (file == NULL)
    {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fclose(file);
    return file_size;
}
