%ifndef SERVER_INC
%define SERVER_INC

htons:
  ; rdi -> port
  ; return value: rax -> network port
  mov   rax, rdi
  xchg  al, ah ; bswap 16-bit registers
  ret

disallow_method:
; rdi -> method
  sub   rsp, 0x10

  mov   [rsp+0x8], rdi

  ; get length of the method
  call  strlen

  ; store the length of the method onto the stack
  mov   qword [rsp], rax

  ; check method arg, variable should be between GET and CONNECT
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  lea   rax, [GET]
  lea   rbx, [CONNECT]

  lea   rdi, [ERR_METHOD_NOT_EXIST]
  cmp   rsi, rax
  jl    .error

  cmp   rsi, rbx
  jg    .error

  ; look if method is inside the list
  ; load request into rdx
  lea   rax, [rsp+0x8]
  mov   rdx, [rax]
  lea   rdi, [methods_list]
  mov   rsi, methods_list_len
  mov   rcx, qword [rsp]
  call  strpos

  ; add method to not allowed list
  lea   rdi, [not_allowed_methods]
  ; rdi points to the position of the method inside allowed list
  add   rdi, rax

  ; put method inside not allowed lists
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  mov   rcx, qword [rsp]
  rep   movsb

  xor   rax, rax
  jmp   .return

.error:
  mov   [errno], rdi
  mov   rax, -1

.return:
  ; clean stack
  add   rsp, 0x10
  ret

; create a route serving files located in a specified dir 
; @param  rdi: path
add_dir_route:
  sub   rsp, 0x18

  mov   [rsp+0x8], rdi

  ; save dir_path into file var
  lea   rsi, [rdi]
  lea   rdi, [file_path]

  ; add slash before dir path
  mov   rax, SLASH
  stosb
  call  strcpy

  lea   rax, [rsp+0x8]
  mov   rdi, [rax]
  
  ; open directory
  mov   rax, SYS_OPEN
  mov   rsi, O_RDONLY | O_DIRECTORY
  mov   rdx, 0
  syscall

  cmp   rax, 0
  jl    .error

  ; get dirent struct
  mov   rdi, rax
  mov   rax, SYS_GETDENTS
  lea   rsi, [dir_entries]
  mov   rdx, DIR_ENTRIES_LEN
  syscall

  cmp   rax, 0
  jl    .error

  ; store number of bytes read
  mov   qword [rsp], rax

  ; save file_path_len
  lea   rdi, [file_path]
  call  strlen
  mov   qword [rsp+0x10], rax

  xor   r9, r9
  lea   rdi, [dir_entries]
  
.loop:
  ; d_reclen
  xor   r10, r10

  ; mov to d_reclen (offset 16)
  mov   r10w, word [rdi + 16]

  ; get d_type (offset d_reclen - 1)
  xor   rax, rax
  mov   al, byte [rdi + r10 - 1]

  ; check if it's a file (DT_REG = 0x8)
  cmp   rax, DT_REG
  jne   .skip_not_file

  ; save rdi pos
  mov   [rsp+0x8], rdi

  ; clear file path after dir
  lea   rdi, [file_path]
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  mov   rax, SLASH
  stosb
  call  strlen

  mov   rcx, rax
  xor   rax, rax
  rep   stosb

  lea   rdi, [file_path]
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  inc   rdi

  lea   rax, [rsp+0x8]
  mov   rsi, [rax]

  ; go to file name (offset 18)
  add   rsi, 18

  call  strcpy
  
  lea   rsi, [file_path]
  lea   rdi, [GET]
  mov   rdx, serve_dir
  call  add_route

  lea   rax, [rsp+0x8]
  mov   rdi, [rax]

.skip_not_file:
  add   r9, r10
  add   rdi, r10

  cmp   r9, qword [rsp]
  jge   .return
  jmp   .loop

.return:
  ; clear file var
  lea   rdi, [file_path]
  mov   rax, NULL_CHAR
  mov   rcx, FILE_PATH_LEN
  rep   stosb

  mov   rax, SUCCESS_CODE
  add   rsp, 0x18
  ret

.error:
  ; clear file var
  lea   rdi, [file_path]
  mov   rax, NULL_CHAR
  mov   rcx, FILE_PATH_LEN
  rep   stosb

  mov   rcx, rax
  mov   rax, NULL_CHAR
  rep   stosb

  mov   rax, FAILURE_CODE
  add   rsp, 0x10
  ret

add_route:
  ; rdi -> method
  ; rsi -> route
  ; rdx -> callback
  sub   rsp, 0x20

  mov   qword [rsp], 0 ; method_len
  mov   qword [rsp+0x8], 0 ; route_len

  mov   [rsp+0x10], rdi 
  mov   [rsp+0x18], rsi

  ; store callback
  mov   qword [rel callback], rdx

  ; get method_length
  lea   rax, [rsp+0x10]
  mov   rdi, [rax]
  call  strlen
  mov   qword [rsp], rax

  ; get route_length
  lea   rax, [rsp+0x18]
  mov   rdi, [rax]
  call  strlen
  mov   qword [rsp+0x8], rax

  ; check if the length of the route is equal to 1
  lea   rax, [rsp+0x18]
  mov   rsi, [rax]
  cmp   qword [rsp+0x8], 1
  je    .skip

  ; if not equal, extract last char to see if it's a slash
  add   rsi, qword [rsp+0x8]
  dec   rsi
  lodsb

  cmp   al, SLASH
  jne   .skip_trim
  
  ; if last char is a '/', remove it and add null char
  mov   byte [rsi], NULL_CHAR
  ; decrease route_length
  dec   qword [rsp+0x8]
  
.skip_trim:
  ; go back to beginning of the route
  sub   rsi, qword [rsp+0x8]

.skip:
  ; go to next available space inside route_list
  xor   rdx, rdx
  mov   rax, qword [routes_count]
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  lea   rdi, [routes_list] 
  add   rdi, rax
  ; move route inside the list
  mov   rcx, qword [rsp+0x8]
  rep   movsb

  ; move the method after the route
  add   rdi, URL_MAX_LEN
  sub   rdi, qword [rsp+0x8]

  ; load method inside rsi
  lea   rax, [rsp+0x10]
  mov   rsi, [rax]
  mov   rcx, qword [rsp]
  rep   movsb

  ; store callback in the right index
  xor   rdx, rdx
  lea   rdi, [callback_list]

  ; compute callback index
  mov   rax, qword [routes_count]
  mov   rbx, 8 ; size of qword
  mul   rbx
  add   rdi, rax

  mov   rax, [rel callback]
  mov   qword [rdi], rax

  ; clear callback
  mov   qword [callback], 0

  ; increase number of routes
  inc   qword [routes_count]

  add   rsp, 0x20
  ret

server_init:
  ; rdi -> port
  ; return value: rax -> socket fd | error
  sub   rsp, 0x8

  mov   qword [port], rdi
  mov   qword [rsp], 0  ; sockfd

  ; check if port is in bounds
  lea   rdi, [ERR_PORT_BOUNDS]
  cmp   qword [port], PORT_MIN
  jl    .error

  cmp   qword [port], PORT_MAX
  jg    .error

  ; create socket
  mov   rax, SYS_SOCKET
  mov   edi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall

  lea   rdi, [ERR_CREAT_SOCKET]
  cmp   rax, 0
  jl    .error

  ; store server fd
  mov   qword [rsp], rax

  ; set socket options
  mov   rax, SYS_SETSOCKOPT
  mov   rdi, qword [rsp]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  mov   r10, enable
  mov   r8, 4 
  syscall

  lea   rdi, [ERR_SET_SOCKOPT]
  cmp   rax, 0
  jl    .error

  mov   rax, SYS_SETSOCKOPT
  mov   rdi, qword [rsp]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4 
  syscall

  lea   rdi, [ERR_SET_SOCKOPT]
  cmp   rax, 0
  jl    .error

  ; convert host port to network
  xor   rax, rax
  mov   rdi, qword [port]
  call  htons

  ; bind socket
  mov   dword [server_sin_port], eax
  mov   rax, SYS_BIND
  mov   rdi, qword [rsp]
  lea   rsi, [server_sin_family]
  mov   rdx, server_addrlen
  syscall

  lea   rdi, [ERR_BIND_SOCKET]
  cmp   rax, 0
  jl    .error

  ; listen socket
  mov   rax, SYS_LISTEN
  mov   rdi, qword [rsp]
  xor   rsi, rsi
  syscall

  lea   rdi, [ERR_LISTEN_SOCKET]
  cmp   rax, 0
  jl    .error

  mov   rax, qword [rsp]
  jmp   .return

.error:
  mov   [errno], rdi

  ; check if socket have been opened
  cmp   qword [rsp], 0
  jle   .skip_close_socket

  ; if yes close it
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall
  ; don't need to check for error, we exit anyway

.skip_close_socket:
  mov   rax, -1

.return:
  add   rsp, 0x8
  ret

run_server:
  ; rdi -> sockfd
  sub   rsp, 0x8

  mov   qword [rsp], rdi

  call  log_server_init

.loop:
  ; set end timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [end_tv_sec]
  syscall

  ; check if we have set start timer
  cmp   qword [start_tv_sec], 0
  je    .timer_not_defined

  ; if yes, compute the difference
  mov   rax, qword [end_tv_nsec]
  sub   rax, qword [start_tv_nsec]

  mov   qword [duration], rax

  ; clear callback
  mov   qword [callback], 0

  ; log previous request
  call  log_request

.timer_not_defined:
  ; clear response, route and method and query
  mov   rcx, RESPONSE_MAX_LEN
  xor   rax, rax
  lea   rdi, [response]
  rep   stosb

  mov   rcx, URL_MAX_LEN
  xor   rax, rax
  lea   rdi, [request_url]
  rep   stosb

  mov   rcx, METHOD_MAX_LEN
  xor   rax, rax
  lea   rdi, [request_method]
  rep   stosb

  mov   rcx, QUERY_MAX_LEN
  xor   rax, rax
  lea   rdi, [query_params]
  rep   stosb

  ; clear body
  mov   rcx, BODY_MAX_LEN
  xor   rax, rax
  lea   rdi, [request_body]
  rep   stosb

  ; clear request
  mov   rcx, REQUEST_MAX_LEN
  xor   rax, rax
  lea   rdi, [request]
  rep   stosb

  ; accept connection
  mov   rax, SYS_ACCEPT
  mov   rdi, qword [rsp]
  lea   rsi, [client_sin_family]
  lea   rdx, [client_addrlen]
  syscall

  mov   qword [request_client], rax

  inc   qword [active_connections]
  mov   rax, qword [active_connections]
  cmp   rax, qword [max_connections]
  jg    .server_unavailable

  ; set start timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [start_tv_sec]
  syscall

  ; receive client request
  mov   rax, SYS_RECVFROM
  mov   rdi, qword [request_client]
  lea   rsi, [request]
  mov   rdx, REQUEST_MAX_LEN
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  mov   qword [request_len], rax

  ; parse_request
  call  parse_request
  cmp   rax, 0
  jl    .error

  ; verify request
  call  verify_request

  ; callback
  cmp   qword [callback], 0
  je    .skip_callback

  lea   rdi, [request]
  call  [rel callback]

.skip_callback:
  mov   rax, SYS_CLOSE
  mov   rdi, qword [request_client] 
  syscall
  
  dec   qword [active_connections]

  jmp   .loop

.error:
  mov   rax, SYS_CLOSE
  mov   rdi, qword [request_client] 
  syscall
  
  dec   qword [active_connections]

  ; jump to next request, skipping logs
  jmp   .timer_not_defined

  ; shutdown server
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

  add   rsp, 0x8
  ret

.server_unavailable:
  mov   qword [response_status_code], SERVICE_UNAVAILABLE
  mov   rdi, qword [request_client]
  lea   rsi, STR_SERVICE_UNAVAILABLE
  call  send_error_response

  jmp   .error

trim_query_parameters:
  ; rdi -> request_url
  ; rsi -> route_length
  ; return value: rax -> url length
  sub   rsp, 0x10 ; url length and query start length

  mov   qword [rsp], rsi

  mov   rsi, QUESTION_MARK
  call  find_next_char
  cmp   rax, 0
  jle   .no_query

  mov   qword [rsp+0x8], rax

  add   rdi, rax
  mov   al, NULL_CHAR
  stosb

  lea   rsi, [rdi]
  lea   rdi, [query_params]
  mov   rcx, qword [rsp]
  sub   rcx, qword [rsp+0x8]
  rep   movsb

  mov   rax, qword [rsp+0x8]
  mov   qword [rsp], rax

.no_query:
  mov   rax, qword [rsp]

  add   rsp, 0x10
  ret

set_max_connections:
  ; rdi -> max connections
  mov   qword [max_connections], rdi

  ret

%endif

