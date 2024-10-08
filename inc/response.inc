%ifndef RESPONSE_INC
%define RESPONSE_INC

send_response:
  ; rdi -> status code str
  ; rsi -> content type
  ; rdx -> body
  sub   rsp, 0x28

  mov   rax, qword [request_client]
  mov   qword [rsp], rax
  mov   [rsp+0x8], rdi
  mov   [rsp+0x10], rsi
  mov   [rsp+0x18], rdx

  ; load protocol
  lea   rdi, [HTTP_1_1]
  call  strlen

  mov   rcx, rax
  lea   rdi, [response]
  lea   rsi, [HTTP_1_1]
  rep   movsb

  ; load space
  xor   rax, rax
  mov   al, SPACE
  stosb

  mov   [rsp+0x20], rdi

  ; load status code
  lea   rax, [rsp+0x8]
  mov   rdi, [rax]
  call  strlen

  mov   rcx, rax
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb

  mov   al, LINE_FEED
  stosb

  mov   rax, [rsp+0x10]
  cmp   rax, 0
  je    .skip_body

  mov   [rsp+0x20], rdi

  ; load content type
  lea   rax, [rsp+0x10]
  mov   rdi, [rax]
  call  strlen
  mov   rcx, rax
  lea   rsi, [rdi]
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb

  mov   al, LINE_FEED
  stosb

  mov   [rsp+0x20], rdi

  ; load connection
  lea   rdi, [CONNECTION_CLOSE]
  call  strlen
  mov   rcx, rax
  lea   rsi, [rdi]
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb
  mov   al, LINE_FEED
  stosb

  mov [rsp+0x20], rdi

  lea   rdi, [CONTENT_LENGTH]
  call  strlen
  mov   rcx, rax
  lea   rsi, [rdi]
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  rep   movsb

  mov [rsp+0x20], rdi

  ; get body length
  lea   rax, [rsp+0x18]
  mov   rdi, [rax]
  call  strlen
  
  mov   rdi, rax
  call  to_string

  lea   rdi, [rax]
  call  strlen
  mov   rcx, rax
  lea   rsi, [rdi]
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  rep   movsb 

  ; load new lines
  lea   rsi, [post_headers]
  mov   rcx, post_headers_len
  rep   movsb
  
  mov   [rsp+0x20], rdi

  ; load body
  lea   rax, [rsp+0x18]
  mov   rdi, [rax]
  call  strlen
  mov   rcx, rax
  lea   rsi, [rdi]
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  rep   movsb

.skip_body:
  ; get response length
  lea   rdi, [response]
  call  strlen
  mov   rdx, rax

  ; send response
  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp]
  lea   rsi, [response]
  syscall

  ; clean stack
  add   rsp, 0x28

  ret

send_error_response:
  ; rdi -> sockfd
  ; rsi -> status code str
  sub   rsp, 0x18

  mov   qword [rsp], rdi
  mov   [rsp+0x8], rsi

  ; load protocol
  lea   rdi, [HTTP_1_1]
  call  strlen

  mov   rcx, rax
  lea   rdi, [response]
  lea   rsi, [HTTP_1_1]
  rep   movsb

  ; load space
  xor   rax, rax
  mov   al, SPACE
  stosb

  mov   [rsp+0x10], rdi

  ; load status code
  lea   rax, [rsp+0x8]
  mov   rdi, [rax]
  call  strlen

  mov   rcx, rax
  lea   rax, [rsp+0x10]
  mov   rdi, [rax]
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb

  mov   al, LINE_FEED
  stosb

  mov   [rsp+0x10], rdi

  ; load connection header
  lea   rdi, [CONNECTION_CLOSE]
  call  strlen
  mov   rcx, rax

  lea   rsi, [rdi]
  lea   rax, [rsp+0x10]
  mov   rdi, [rax]
  rep   movsb

  ; load new lines
  lea   rsi, [post_headers]
  mov   rcx, post_headers_len
  rep   movsb

  lea   rdi, [response]
  call  strlen
  mov   rdx, rax

  ; send response
  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp]
  lea   rsi, [response]
  syscall

  ; clean stack
  add   rsp, 0x18
  ret

serve_string:
  ; rdi -> string
  sub   rsp, 0x8 ; string

  mov   [rsp], rdi
  
  lea   rdi, [STR_OK]
  lea   rsi, [CONTENT_PLAIN]
  lea   rax, [rsp]
  mov   rdx, [rax]
  call  send_response

  add   rsp, 0x8
  ret

serve_static_file:
  ; rdi -> file path
  sub   rsp, 0x20 ; path, length, fd and content-type

  mov   [rsp], rdi

  ; get file ext
  call  get_ext

  cmp   rax, 0
  jl    .error_open

  mov   [rsp+0x18], rax

  ; load file path
  lea   rax, [rsp]
  mov   rdi, [rax]

  ; open file
  mov   rax, SYS_OPEN
  mov   rsi, O_RDONLY
  mov   rdx, 0
  syscall

  cmp   rax, 0
  jl    .error_open

  mov   qword [rsp+0x10], rax

  ; read file
  mov   rax, SYS_READ
  mov   rdi, qword [rsp+0x10]
  lea   rsi, [file]
  mov   rdx, FILE_MAX_LEN
  syscall

  cmp   rax, 0
  jl    .error_read

  mov   qword [rsp+0x8], rax

  ; send response
  lea   rdi, [STR_OK]
  lea   rax, [rsp+0x18]
  mov   rsi, [rax]
  lea   rdx, [file]
  call  send_response

  ; close file
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+0x10]
  syscall

  ; clear file buffer
  xor   rax, rax
  mov   al, NULL_CHAR
  lea   rdi, [file]
  mov   rcx, FILE_MAX_LEN
  rep   stosb

  mov   rax, 0
  ; clean stack
  add   rsp, 0x20
  ret

.error_open:
  ; load 404
  mov   qword [response_status_code], NOT_FOUND
  mov   rdi, qword [request_client]
  lea   rsi, [STR_NOT_FOUND]
  call  send_error_response
  jmp   .error

.error_read:
  ; load 500
  mov   qword [response_status_code], INTERNAL_SERVER_ERROR
  mov   rdi, qword [request_client]
  lea   rsi, [STR_INTERNAL_SERVER_ERROR]
  call  send_error_response

.error:
  ; clean stack
  add   rsp, 0x20

  ; return
  mov   rax, -1
  ret

; serve file from directory
; @param rdi: request
serve_dir:
  lea   rdi, [request_url]  
  ; add one to remove '/'
  inc   rdi
  call  serve_static_file

  ret

; Sets the value of a specific HTTP header.
; @param  rdi: key
; @param  rsi: value
; @return rax: success/failure code
set_headers:
  sub   rsp, 0x10

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  mov   rax, SUCCESS_CODE

  add   rsp, 0x10
  ret

%endif

