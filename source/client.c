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

// Function Prototypes
void validate_arguments(int argc);
void prepare_filename(char* dest, const char* src);
FILE* open_file(const char* filename);
long get_file_size(FILE* file);
char* read_file_content(FILE* file, long file_size);
void close_file(FILE* file);
int create_client_socket();
void connect_to_server(int client_socket);
void send_message_to_server(int client_socket, const char* message, long size);
void receive_server_response(int client_socket);
void close_socket(int client_socket);

int main(int argc, char* argv[]) {
    validate_arguments(argc);

    char file_name[FILE_NAME_SIZE];
    prepare_filename(file_name, argv[1]);

    FILE* file = open_file(file_name);
    long file_size = get_file_size(file);
    if (file_size <= 0) {
        fprintf(stderr, "ERR: File is empty or unreadable.\n");
        close_file(file);
        exit(EXIT_FAILURE);
    }

    char* file_content = read_file_content(file, file_size);
    close_file(file);

    int client_socket = create_client_socket();
    connect_to_server(client_socket);
    send_message_to_server(client_socket, file_content, file_size);
    free(file_content);

    receive_server_response(client_socket);
    close_socket(client_socket);

    return 0;
}

// Validate command-line arguments
void validate_arguments(int argc) {
    if (argc != 2) {
        fprintf(stderr, "Usage: <filename>\n");
        exit(EXIT_FAILURE);
    }
}

// Copy filename safely
void prepare_filename(char* dest, const char* src) {
    strncpy(dest, src, FILE_NAME_SIZE - 1);
    dest[FILE_NAME_SIZE - 1] = '\0';
}

// Open file safely
FILE* open_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("ERR: File not found");
        exit(EXIT_FAILURE);
    }
    return file;
}

// Get file size
long get_file_size(FILE* file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    return size;
}

// Read file content into memory
char* read_file_content(FILE* file, long file_size) {
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        perror("ERR: Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';  // Null-terminate the string
    return buffer;
}

// Close file safely
void close_file(FILE* file) {
    if (file) {
        fclose(file);
    }
}

// Create client socket
int create_client_socket() {
    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("ERR: Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

// Connect client to server
void connect_to_server(int client_socket) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, S_PATH, sizeof(addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("ERR: Unable to connect to server");
        close_socket(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server...\n");
}

// Send message to the server
void send_message_to_server(int client_socket, const char* message, long size) {
    if (send(client_socket, message, size, 0) == -1) {
        perror("ERR: Failed to send message");
        close_socket(client_socket);
        exit(EXIT_FAILURE);
    }
    printf("Message sent to the server.\n\n");
}

// Receive server response
void receive_server_response(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    bool done_receiving = false;

    printf("Encrypted message received from the server:\n");

    while (!done_receiving) {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
            if (bytes_received < BUFFER_SIZE - 1) {
                done_receiving = true;
            }
        } else if (bytes_received == 0) {
            done_receiving = true;
            printf("\nServer closed the connection.\n");
        } else {
            perror("ERR: Receiving error");
            done_receiving = true;
        }
    }
    printf("\nDisconnected from the server.\n");
}

// Close socket safely
void close_socket(int client_socket) {
    close(client_socket);
}
