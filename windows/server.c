#include "server.h"

#define BUFFER_SIZE 1024

SOCKET serverSocket;

void send_response(SOCKET clientSocket, char* status_code, struct Header* headers, int header_count, char* body)
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
    }
    else {
        strcat(response, "\r\n\r\n");
    }

    if (body) {
        strcat(response, body);
    }

    send(clientSocket, response, (int)strlen(response), 0);
    closesocket(clientSocket);
}

DWORD WINAPI handle_request(void* args)
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
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);

        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) > 0) {
            size_t length = strlen(buffer);

            if (!buffer) {
                send_response(clientSocket, "400", NULL, 0, NULL);
            }
            else {
                struct HttpRequest request = { clientSocket, malloc(length), length };
                strcpy(request.buffer, buffer);
                memset(buffer, 0, sizeof(buffer));

                CreateThread(NULL, 0, handle_request, &request, 0, NULL);
            }
        }
    }

    closesocket(serverSocket);
    WSACleanup();
}

int init_socket(char* port)
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 0;
    }

    struct addrinfo* addrInfo = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if (getaddrinfo(NULL, port, &hints, &addrInfo) != 0) {
        WSACleanup();

        return 0;
    }

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        freeaddrinfo(addrInfo);
        WSACleanup();

        return 0;
    }

    if (bind(serverSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR) {
        freeaddrinfo(addrInfo);
        closesocket(serverSocket);
        WSACleanup();

        return 0;
    }

    freeaddrinfo(addrInfo);

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();

        return 0;
    }

    return 1;
}
