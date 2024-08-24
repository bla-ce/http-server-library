%ifndef REQUEST_INC
%define REQUEST_INC

%macro parse_request 0
  lea   rsi, [request]
  mov   rcx, qword [request_len]

  ; extract method
  lea   rdi, [request_method]

  find_next_char rsi, SPACE
  cmp   rax, 0x0
  jl    %%error

  mov   rcx, rax
  rep   movsb

  ; mov rsi to go to the url
  inc   rsi

  ; extract url
  lea   rdi, [request_url]
  find_next_char rsi, SPACE
  cmp   rax, 0x0
  jl    %%error

  mov   rcx, rax
  rep   movsb

  ; inc rsi to go to the protocol 
  inc   rsi

  ; extract proto
  lea   rdi, [request_proto]
  find_next_char rsi, CARRIAGE_RETURN
  cmp   rax, 0x0
  jl    %%error

  mov   rcx, rax
  rep   movsb

  push  rsi

  ; parse proto
  lea   rsi, [request_proto]
  find_next_char rsi, SLASH
  cmp   rax, 0x0
  jl    %%error

  pop   rsi

  ; mov to proto major
  inc   rsi
  sub   rsi, rax

  xor   rax, rax
  lodsb
  sub   al, 0x30  ; ascii to int

  ; check proto major is digit
  cmp   eax, 0x0
  jl    %%error

  cmp   eax, 0x9
  jg    %%error

  mov   dword [request_proto_major], eax

  ; mov to proto minor
  inc   rsi

  xor   rax, rax
  lodsb
  sub   al, 0x30  ; ascii to int

  ; check proto minor is digit
  cmp   eax, 0x0
  jl    %%error

  cmp   eax, 0x9
  jg    %%error

  mov   dword [request_proto_minor], eax

  ; mov to new line (skip 2 char \r\n)
  add   rsi, 0x2

  push  rsi
  ; get headers
  strpos rsi, [request_len], post_headers, post_headers_len
  pop rsi

  lea   rdi, [request_headers]
  mov   rcx, rax
  rep   movsb

  ; get body
  ; move to new line (skip 4 char \r\n\r\n)
  add   rsi, 0x4

  lea   rdi, [request_body]
  find_next_char rsi, NULL_CHAR
  cmp   rax, 0x0
  jl    %%error

  mov   rcx, rax
  rep   movsb

  jmp   %%return

%%error:
  mov   qword [response_status_code], BAD_REQUEST

%%return:
%endmacro

%macro verify_request 0
  get_length request_method
  mov   r10, rax

  check_method request_method, r10  
  cmp   rax, 0
  jl    %%method_not_allowed

  get_length request_url
  mov   r10, rax
  check_route request_url, r10
  cmp   rax, 0
  jl    %%not_found

  mov   qword [response_status_code], OK
  jmp   %%return

%%method_not_allowed:
  mov   qword [response_status_code], METHOD_NOT_ALLOWED
  jmp   %%return

%%not_found:
  mov   qword [response_status_code], NOT_FOUND
  jmp   %%return

%%return:
%endmacro

%macro send_response 1
  ; %1 -> client
  mov   rdi, %1

  cmp   qword [response_status_code], OK 
  je    %%send_200

  cmp   qword [response_status_code], BAD_REQUEST
  je    %%send_400

  cmp   qword [response_status_code], NOT_FOUND
  je    %%send_404

  cmp   qword [response_status_code], METHOD_NOT_ALLOWED
  je    %%send_405

  jmp   %%return

%%send_200:
  mov   rax, SYS_WRITE
  mov   rsi, response_200
  mov   rdx, response_200_len
  syscall

  jmp   %%return

%%send_400:
  mov   rax, SYS_WRITE
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  jmp   %%return

%%send_404:
  mov   rax, SYS_WRITE
  mov   rsi, response_404
  mov   rdx, response_404_len
  syscall

  jmp   %%return

%%send_405:
  mov   rax, SYS_WRITE
  mov   rsi, response_405
  mov   rdx, response_405_len
  syscall

  jmp   %%return

%%send_500:
  mov   rax, SYS_WRITE
  mov   rsi, response_500
  mov   rdx, response_500_len
  syscall

  jmp   %%return

%%return:
%endmacro

%endif