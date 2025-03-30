#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define DOCUMENT_ROOT "./www"

const char* get_mime_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream"; 
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    return "application/octet-stream";
}

void serve_file(int client_socket, char *filename) {
    char filepath[BUFFER_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/%s", DOCUMENT_ROOT, filename);

    int file_fd = open(filepath, O_RDONLY);
    if (file_fd == -1) {
        char response[] = "HTTP/1.1 404 Not Found\r\n"
                          "Content-Length: 25\r\n"
                          "Content-Type: text/html\r\n"
                          "\r\n"
                          "<h1>404 Not Found</h1>";
        write(client_socket, response, sizeof(response) - 1);
        return;
    }

    struct stat file_stat;
    fstat(file_fd, &file_stat);
    size_t file_size = file_stat.st_size;

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Length: %zu\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             file_size, get_mime_type(filename));

    write(client_socket, header, strlen(header));

    char file_buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
        write(client_socket, file_buffer, bytes_read);
    }

    close(file_fd);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    read(client_socket, buffer, sizeof(buffer) - 1);

    char method[10], path[BUFFER_SIZE];
    sscanf(buffer, "%s /%s", method, path);

    if (strlen(path) == 0 || strcmp(path, "/") == 0) {
        strcpy(path, "index.html");
    }

    serve_file(client_socket, path);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

        server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

        if (listen(server_socket, 5) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Server running on http://localhost:%d\n", PORT);
    printf("Serving files from %s\n", DOCUMENT_ROOT);

        while (1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Connection failed");
            continue;
        }

        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}



