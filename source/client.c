#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#define FILE_NAME_SIZE 1024
#define S_PATH "/tmp/socket"

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
    read_file(file_name);
    if(get_file_size(file_name) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if(get_file_size(file_name) == 0)
    {
        printf("ERR: File is empty\n");
        exit(EXIT_FAILURE);
    }
    if(get_file_size(file_name) > 0)
    {
    printf("File content: %s", read_file(file_name));
    printf("File size: %d\n\n", get_file_size(file_name));
    }


    struct sockaddr_un addr;
    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    sun_path_max_length = sizeof(addr.sun_path);
    strncpy(addr.sun_path, S_PATH, sun_path_max_length);

    if (connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        printf("ERR: Connection error. Server is not running\n");
        exit(EXIT_FAILURE);
    }

     printf("Connected to Server.............\n");

    if (send(client_socket, read_file(file_name), get_file_size(file_name), 0) == -1)
    {
        perror("Sending error");
        exit(EXIT_FAILURE);
    }

    printf("Message Sent to the Server......\n\n");


    char buffer[1025];
    ssize_t bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("Encrypted Message Received: %s\n", buffer);
    }

    close(client_socket); 

}

char* read_file(char* filename)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen(filename,"r");
    if (file==NULL){
        printf("ERR: File not found\n");
        return NULL;
    }

    file_size = get_file_size(filename);

    buffer = (char*) malloc(file_size * sizeof(char));
    if (buffer == NULL){
        printf("Error: Memory allocation failed\n");
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

    file = fopen(filename,"r");
    if (file==NULL){
        return -1;
    }

    fseek(file, 0 , SEEK_END);
    file_size = ftell(file);
    fclose(file);
    return file_size;
}







