#include "http.h"

char* methods[] = {
    "GET", "POST", "HEAD", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"
};

void get_method(struct HttpRequest* request)
{
    char c;
    char* method = malloc(request->length);

    if (!method) {
        return;
    }

    memset(method, 0, request->length);

    for (size_t i = 0; i < request->length; ++i) {
        c = request->buffer[i];

        if (c == ' ') {
            break;
        }

        strncat(method, &c, 1);
    }

    for (int i = 0; i < sizeof(methods) / sizeof(methods[0]); ++i) {
        if (strcmp(method, methods[i]) == 0) {
            request->header.method = i;
        }
    }

    free(method);
}

void get_path(struct HttpRequest* request)
{
    char c;
    short in_path = 0;
    char* path = malloc(request->length);

    if (!path) {
        return;
    }

    memset(path, 0, request->length);

    for (size_t i = 0; i < request->length; ++i) {
        c = request->buffer[i];

        if (c == '/') {
            in_path = 1;
        }

        if (!in_path) {
            continue;
        }

        if (c == ' ' || c == '?') {
            break;
        }

        strncat(path, &c, 1);
    }

    request->header.path = malloc(strlen(path));

    if (!request->header.path) {
        return;
    }

    strcpy(request->header.path, path);
    free(path);
}

void get_queries(struct HttpRequest* request)
{
    char c;
    short status = 0;
    int count = 0;

    char* item = malloc(request->length);

    if (!item) {
        return;
    }

    memset(item, 0, request->length);

    request->header.queries = malloc(sizeof(struct Query));

    for (size_t i = 0; i < request->length; ++i) {
        c = request->buffer[i];

        if (c == '/') {
            status = 1;
            continue;
        }

        if (!status) {
            continue;
        }

        if (c == ' ' && status != 2) {
            break;
        }

        if (c == '?') {
            status = 2;
            continue;
        }

        if (status != 2) {
            continue;
        }

        if (c == '=') {
            request->header.queries[count].key = malloc(strlen(item) + 1);
            strcpy(request->header.queries[count].key, item);
            memset(item, 0, request->length);

            continue;
        }

        if (c == '&') {
            request->header.queries[count].value = malloc(strlen(item) + 1);
            strcpy(request->header.queries[count].value, item);
            memset(item, 0, request->length);

            ++count;
            struct Query* tmp_queries = (struct Query*)realloc(request->header.queries, (size_t)(count + 1) * sizeof(struct Query));
            request->header.queries = tmp_queries;


            continue;
        }

        if (c == ' ') {
            request->header.queries[count].value = malloc(strlen(item) + 1);
            strcpy(request->header.queries[count].value, item);
            memset(item, 0, request->length);

            ++count;
            struct Query* tmp_queries = (struct Query*)realloc(request->header.queries, (size_t)(count + 1) * sizeof(struct Query));
            request->header.queries = tmp_queries;

            break;
        }

        strncat(item, &c, 1);
    }

    free(item);

    request->header.query_count = count;
}

void get_headers(struct HttpRequest* request)
{
    char c;
    short header_section, body_section;
    int count;
    header_section = body_section = count = 0;

    char* item = malloc(request->length);

    if (!item) {
        return;
    }

    memset(item, 0, request->length);
    request->header.headers = malloc(sizeof(struct Header));

    for (size_t i = 0; i < request->length; ++i) {
        c = request->buffer[i];

        if (c == '\n' || c == '\r') {
            if (!header_section && c == '\n') {
                memset(item, 0, request->length);
                ++header_section;

                continue;
            }

            if (i > 2 && c == '\n' && request->buffer[i - 1] == '\r' && request->buffer[i - 2] == '\n' && request->buffer[i - 3] == '\r') {
                break;
            }

            if (c == '\n') {
                request->header.headers[count].value = malloc(strlen(item) + 1);
                strcpy(request->header.headers[count].value, item);
                memset(item, 0, request->length);

                ++count;

                struct Header* tmp_headers = (struct Header*)realloc(request->header.headers, (size_t)(count + 1) * sizeof(struct Header));
                request->header.headers = tmp_headers;
            }

            continue;
        }

        if ((c == ' ' && i > 1 && request->buffer[i - 1] == ':')) {
            item[strlen(item) - 1] = '\0';
            request->header.headers[count].key = malloc(strlen(item) + 1);
            strcpy(request->header.headers[count].key, item);
            memset(item, 0, request->length);

            continue;
        }

        strncat(item, &c, 1);
    }

    free(item);
    request->header.header_count = count;
}

void get_body(struct HttpRequest* request)
{
    char c;
    short body_section = 0;

    request->body = malloc(request->length);

    if (!request->body) {
        return;
    }

    memset(request->body, 0, request->length);

    for (size_t i = 0; i < request->length; ++i) {
        c = request->buffer[i];

        if (body_section) {
            strncat(request->body, &c, 1);
        }

        if (c == '\n' && i > 2 && request->buffer[i - 1] == '\r' && request->buffer[i - 2] == '\n' && request->buffer[i - 3] == '\r') {
            ++body_section;
        }

        continue;
    }
}
