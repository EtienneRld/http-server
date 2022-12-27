#include "routes.h"

struct Route* routes;
int count;

void add_route(void* func, Method method, char* path)
{
    if (!count) {
        routes = malloc(sizeof(struct Route));
    }
    else {
        struct Route* tmp_routes = (struct Route*)realloc(routes, (size_t)(count + 1) * sizeof(struct Route));

        routes = tmp_routes;
    }

    if (routes == NULL) {
        return;
    }

    routes[count].func = func;
    routes[count].method = method;
    routes[count].path = path;

    ++count;
}

int route_callback(struct HttpRequest* request)
{
    int found = 0;

    for (int i = 0; i < count; ++i) {
        if (!strcmp(request->header.path, routes[i].path) && request->header.method == routes[i].method) {
            found = 1;
            routes[i].func(request);
        }
    }

    return found;
}
