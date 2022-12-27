#include "routes.h"

void send_response(SOCKET clientSocket, char* status_code, struct Header* headers, int header_count, char* body);
void run_server();
int init_socket(char* port);
