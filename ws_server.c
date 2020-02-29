#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include "ws.h"
#include "ws_http.h"
#include "ws_socket.h"
#include "ws_string.h"

#define MAX_WS_CLIENTS 32

struct ws_client {
  bool is_connected;
  ws_socket_t socket;
  struct sockaddr_in address;
};

static ws_socket_t ws_server_socket = -1;
static struct ws_client ws_clients[MAX_WS_CLIENTS];

static void start_server(unsigned short port,
                         ws_socket_t *listen_sock,
                         void (*handler)(ws_socket_t, struct sockaddr_in *))
{
  ws_socket_t server_sock;
  ws_socket_t client_sock;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int opt_reuseaddr;

  server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_sock < 0) {
    printf("Could not open socket: %d\n", ws_socket_last_error());
    return;
  }

  opt_reuseaddr = 1;
  setsockopt(server_sock,
             SOL_SOCKET,
             SO_REUSEADDR,
             (const void *)&opt_reuseaddr,
             sizeof(opt_reuseaddr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);
  if (bind(server_sock,
           (struct sockaddr *)&server_addr,
           sizeof(server_addr)) != 0) {
    ws_close_socket(server_sock);
    printf("Could not bind to port %u: %d\n", port, ws_socket_last_error());
    return;
  }

  if (listen(server_sock, 1) != 0) {
    ws_close_socket(server_sock);
    printf("Could not start listening for connections: %d\n",
           ws_socket_last_error());
    return;
  }

  assert(listen_sock != NULL);
  *listen_sock = server_sock;

  printf("Listening on port %u\n", port);

  for (;;) {
    client_sock = accept(server_sock,
                         (struct sockaddr *)&client_addr,
                         &client_addr_len);
    if (client_sock < 0) {
      printf("Could not accept connection: %d\n", ws_socket_last_error());
      continue;
    }
    handler(client_sock, &client_addr);
  }
}

static void process_ws_request(ws_socket_t sock, struct sockaddr_in *addr)
{
  int error = ws_perform_handshake(sock);
  if (error != 0) {
    printf("WebSocket handshake error: %s\n", ws_error_message(error));
    ws_close_socket(sock);
    return;
  }

  char ip_str[INET_ADDRSTRLEN] = { 0 };
  if (inet_ntop(addr->sin_family,
                &addr->sin_addr,
                ip_str,
                sizeof(ip_str)) != NULL) {
    printf("WebSocket client connected: %s\n", ip_str);
  } else {
    printf("WebSocket client connected (unknown address)\n");
  }

  struct ws_client *client = NULL;
  for (int i = 0; i < MAX_WS_CLIENTS; i++) {
    if (!ws_clients[i].is_connected) {
      client = &ws_clients[i];
      client->socket = sock;
      client->address = *addr;
      client->is_connected = true;
      break;
    }
  }
  if (client == NULL) {
    printf("Client limit reached, closing connection\n");
    ws_send_close(sock, 0, 0);
    ws_close_socket(sock);
    return;
  }

  ws_send_text(client->socket,
    "Hello! Thanks for connecting!\n", WS_FLAG_FINAL, 0);
}

int main(int argc, char **argv)
{
#ifdef _WIN32
  WSADATA wsa_data;
  DWORD result = WSAStartup(MAKEWORD(2,2), &wsa_data);
  if (result != 0) {
      printf("WSAStartup failed: %d\n", result);
      return EXIT_FAILURE;
  }
#endif

  start_server(8000, &ws_server_socket, process_ws_request);

#if 0
  for (int i = 0; i < MAX_WS_CLIENTS; i++) {
    ws_socket_t sock = ws_clients[i].socket;
    ws_send_close(sock, 0, 0);
    ws_close_socket(sock);
  }
#endif

  return EXIT_SUCCESS;
}
