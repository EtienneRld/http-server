#include "http.h"

struct Route {
    void (*func)(struct HttpRequest* request);
    Method method;
    char* path;
};

void add_route(void* func, Method method, char* path);
int route_callback(struct HttpRequest* request);