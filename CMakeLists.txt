project(ws)

cmake_minimum_required(VERSION 3.0)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_library(ws STATIC
    ws.c
    ws.h
    ws_base64.c
    ws_base64.h
    ws_http.c
    ws_http.h
    ws_sha1.c
    ws_sha1.h
    ws_socket.c
    ws_socket.h
    ws_string.c
    ws_string.h
)

if(WIN32)
  target_link_libraries(ws ws2_32)
endif()

add_executable(ws_server ws_server.c)
target_link_libraries(ws_server ws)

add_executable(ws_client ws_client.c)
target_link_libraries(ws_client ws)
