#include "server.h"
#include <stdio.h>

void print(struct HttpRequest* request)
{
    printf("Client -> %llu\n", request->clientSocket);
    printf("Buffer -> %s\n\n", request->buffer);
    printf("Length -> %llu\n", request->length);
    printf("Method -> %d\n", request->header.method);
    printf("Path -> %s\n\n", request->header.path);
    printf("Query count -> %d\n\n", request->header.query_count);

    for (int i = 0; i < request->header.query_count; ++i) {
        printf("[%d] Key -> %s | Value -> %s\n", i + 1, request->header.queries[i].key, request->header.queries[i].value);
    }

    printf("\nHeader count -> %d\n\n", request->header.header_count);

    for (int i = 0; i < request->header.header_count; ++i) {
        printf("[%d] Key -> %s | Value -> %s\n", i + 1, request->header.headers[i].key, request->header.headers[i].value);
    }

    printf("\nBody -> %s\n\n", request->body);

    send_response(request->clientSocket, "200", NULL, 0, request->buffer);
}

void header(struct HttpRequest* request)
{
    struct Header* headers = malloc(3 * sizeof(struct Header));

    if (!headers) {
        send_response(request->clientSocket, "500", NULL, 0, NULL);
        return;
    }

    for (int i = 0; i < 3; ++i) {
        headers[i].key = malloc(20);
        headers[i].value = malloc(20);
    }

    headers[0] = (struct Header){ "Accept", "*/*" };
    headers[1] = (struct Header){ "x-powered-by", "test" };
    headers[2] = (struct Header){ "Connection", "close" };

    send_response(request->clientSocket, "200", headers, 3, NULL);

    free(headers);
}

int main()
{
    add_route(&print, GET, "/print");
    add_route(&header, PUT, "/header");

    if (!init_socket("3000")) {
        return 1;
    }
    else {
        run_server();
    }

    return 0;
}
