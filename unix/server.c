#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int serverSocket;

void send_response(int clientSocket, char* status_code, struct Header* headers, int header_count, char* body)
{
    char response[1024] = "HTTP/1.1 ";
    strcat(response, status_code);

    if (header_count) {
        strcat(response, "\r\n");

        for (int i = 0; i < header_count; ++i) {
            strcat(response, headers[i].key);
            strcat(response, ": ");
            strcat(response, headers[i].value);
            strcat(response, "\r\n");
        }

        strcat(response, "\r\n");
    } else {
        strcat(response, "\r\n\r\n");
    }

    if (body) {
        strcat(response, body);
    }

    send(clientSocket, response, (int)strlen(response), 0);
    close(clientSocket);
}

void* handle_request(void* args)
{
    struct HttpRequest* request = args;

    get_method(request);
    get_path(request);
    get_queries(request);
    get_headers(request);
    get_body(request);

    if (!route_callback(request)) {
        send_response(request->clientSocket, "404", NULL, 0, "<h1>404 Not Found</h1>");
    }

    free(request->buffer);
    free(request->header.path);
    free(request->header.queries);
    free(request->header.headers);
    free(request->body);

    return 0;
}

void run_server()
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    for (;;) {
        int clientSocket = accept(serverSocket, NULL, NULL);

        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) > 0) {
            size_t length = strlen(buffer);

            if (!length) {
                send_response(clientSocket, "400", NULL, 0, NULL);
            } else {
                struct HttpRequest request = { clientSocket, malloc(length), length };
                strcpy(request.buffer, buffer);
                memset(buffer, 0, sizeof(buffer));

                pthread_t thread;
                pthread_create(&thread, NULL, &handle_request, &request);
            }
        }
    }

    close(serverSocket);
}

int init_socket(int port)
{
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return 0;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        return 0;
    }

    if (listen(serverSocket, SOMAXCONN) < 0) {
        return 0;
    }

    return 1;
}
