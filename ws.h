#ifndef WS_H
#define WS_H

#include <stddef.h>
#include "ws_socket.h"

#ifdef _MSC_VER
  typedef unsigned __int8 ws_uint8_t;
  typedef unsigned __int16 ws_uint16_t;
  typedef unsigned __int32 ws_uint32_t;
  typedef unsigned __int64 ws_uint64_t;
#else
  #include <stdint.h>
  typedef uint8_t ws_uint8_t;
  typedef uint16_t ws_uint16_t;
  typedef uint32_t ws_uint32_t;
  typedef uint64_t ws_uint64_t;
#endif

#define WS_PROTOCOL_VERSION 13

enum {
  WS_ERROR_MEMORY = 1,
  WS_ERROR_RECV_HTTP_HEADERS,
  WS_ERROR_HTTP_REQUEST,
  WS_ERROR_HTTP_METHOD,
  WS_ERROR_HTTP_VERSION,
  WS_ERROR_WEBSOCKET_VERSION,
  WS_ERROR_NO_UPGRADE,
  WS_ERROR_NO_KEY
};

enum {
  WS_OP_CONTINUATION = 0,
  WS_OP_TEXT = 1,
  WS_OP_BINARY = 2,
  WS_OP_CLOSE = 8,
  WS_OP_PING = 9,
  WS_OP_PONG = 10
};

enum {
  WS_FLAG_FINAL = 1u << 15,
  WS_FLAG_RSV1 = 1u << 14,
  WS_FLAG_RSV2 = 1u << 13,
  WS_FLAG_RSV3 = 1u << 12,
  WS_FLAG_MASK = 1u << 7
};

const char *ws_error_message(int error);
int ws_perform_handshake(ws_socket_t sock);
int ws_parse_connect_request(
  const char *buf,
  size_t len,
  const char **key,
  size_t *key_len);
int ws_send_handshake_accept(ws_socket_t sock, const char *key);
int ws_send(
  ws_socket_t sock,
  int opcode,
  const char *data,
  size_t len,
  ws_uint32_t flags,
  ws_uint32_t masking_key);
int ws_send_text(
  ws_socket_t sock,
  const char *text,
  ws_uint16_t flags,
  ws_uint32_t masking_key);
int ws_send_close(ws_socket_t sock,
  ws_uint16_t flags,
  ws_uint32_t masking_key);

#endif /* WS_H */
