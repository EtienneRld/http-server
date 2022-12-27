#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    GET,
    POST,
    HEAD,
    PUT,
    DEL,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH
} Method;

struct Header {
    char* key;
    char* value;
};

struct Query {
    char* key;
    char* value;
};

struct HttpHeader {
    Method method;
    char* path;
    struct Query* queries;
    struct Header* headers;
    int query_count;
    int header_count;
};

struct HttpRequest {
    int clientSocket;
    char* buffer;
    size_t length;
    struct HttpHeader header;
    char* body;
};

void get_method(struct HttpRequest* request);
void get_path(struct HttpRequest* request);
void get_queries(struct HttpRequest* request);
void get_headers(struct HttpRequest* request);
void get_body(struct HttpRequest* request);
