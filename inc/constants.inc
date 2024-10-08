%ifndef CONSTANTS_INC
%define CONSTANTS_INC

STDOUT          equ 1
LONG_SIZE       equ 10

; time constants
CLOCK_REALTIME  equ 0
HOURS_IN_DAY    equ 24
EPOCH_YEAR      equ 1970

SECONDS_IN_MINUTE equ 60
SECONDS_IN_HOUR   equ 3600
SECONDS_IN_DAY    equ 86400
DAYS_IN_YEAR      equ 365
DAYS_IN_LEAP_YEAR equ 366

; server constants
PORT_MIN                equ 0x1
DEFAULT_MAX_CONNECTIONS equ 0x1
PORT_MAX                equ 0xffff

; request struct constants
METHOD_MAX_LEN        equ 0x8
PROTO_MAX_LEN         equ 0x9
HOST_MAX_LEN          equ 0xFF
QUERY_MAX_LEN         equ 0x20
QUERY_MAX_VALUE_LEN   equ 0x10
URL_MAX_LEN           equ 0x30
HEADERS_MAX_LEN       equ 0x2000
BODY_MAX_LEN          equ 0x2000

; response struct constants
STATUS_MAX_LEN      equ 0x40
RESPONSE_MAX_LEN    equ 0x2000

; socket constants
INADDR_ANY          equ 0
SOCK_STREAM         equ 1
SOL_SOCKET          equ 1
SO_REUSEADDR        equ 2
AF_INET             equ 2
STATUS_CODE_LEN     equ 3
MAX_ROUTES_COUNT    equ 10
SO_REUSEPORT        equ 15
MSG_TRUNC           equ 32
REQUEST_MAX_LEN     equ 0x2000
STRUCT_ROUTE_LEN    equ URL_MAX_LEN + METHOD_MAX_LEN

; syscall values
SYS_READ          equ 0
SYS_WRITE         equ 1
SYS_OPEN          equ 2
SYS_CLOSE         equ 3
SYS_SIGACTION     equ 13
SYS_SOCKET        equ 41
SYS_CONNECT       equ 42
SYS_ACCEPT        equ 43
SYS_SENDTO        equ 44
SYS_RECVFROM      equ 45
SYS_BIND          equ 49
SYS_LISTEN        equ 50
SYS_SETSOCKOPT    equ 54
SYS_EXIT          equ 60
SYS_GETDENTS      equ 78
SYS_CLOCK_GETTIME equ 228

; ascii
NULL_CHAR       equ 0
END_OF_TEXT     equ 3
LINE_FEED       equ 10
CARRIAGE_RETURN equ 13
SPACE           equ 32
NUMBER_SIGN     equ 35
AMPERSAND       equ 38
HYPHEN          equ 45
DOT             equ 46
SLASH           equ 47
ZERO_CHAR       equ 48
COLON           equ 58
EQUAL_SIGN      equ 61
QUESTION_MARK   equ 63
LOW_N_CHAR      equ 110
LOW_S_CHAR      equ 115

; extended ascii
MICRO_FIRST   equ 0xCE
MICRO_SECOND  equ 0xBC

; file constants
O_RDONLY        equ 0
O_DIRECTORY     equ 65536
FILE_PATH_LEN   equ 32
FILE_MAX_LEN    equ 0x2000
DIR_ENTRIES_LEN equ 0x400
DT_REG          equ 0x8

; file extension
TEXT_EXT  db ".txt", NULL_CHAR
CSS_EXT   db ".css", NULL_CHAR
HTML_EXT  db ".html", NULL_CHAR
JS_EXT    db ".js", NULL_CHAR

; exit code
SUCCESS_CODE equ 0
FAILURE_CODE equ -1

; methods
GET     db "GET", NULL_CHAR
POST    db "POST", NULL_CHAR
PUT     db "PUT", NULL_CHAR
DELETE  db "DELETE", NULL_CHAR
HEAD    db "HEAD", NULL_CHAR
OPTIONS db "OPTIONS", NULL_CHAR
PATCH   db "PATCH", NULL_CHAR
TRACE   db "TRACE", NULL_CHAR
CONNECT db "CONNECT", NULL_CHAR

; status code
OK                    equ 200
BAD_REQUEST           equ 400
UNAUTHORIZED          equ 401
NOT_FOUND             equ 404
METHOD_NOT_ALLOWED    equ 405
URI_TOO_LONG          equ 414
INTERNAL_SERVER_ERROR equ 500
SERVICE_UNAVAILABLE   equ 503

; status code string
STR_OK                    db "200 OK", NULL_CHAR
STR_BAD_REQUEST           db "400 Bad Request", NULL_CHAR
STR_UNAUTHORIZED          db "401 Unauthorized", NULL_CHAR
STR_NOT_FOUND             db "404 Not Found", NULL_CHAR
STR_METHOD_NOT_ALLOWED    db "405 Method Not Allowed", NULL_CHAR
STR_URI_TOO_LONG          db "414 URI Too Long", NULL_CHAR
STR_INTERNAL_SERVER_ERROR db "500 Internal Server Error", NULL_CHAR
STR_SERVICE_UNAVAILABLE    db "503 Service Unavailable", NULL_CHAR

; protocol
HTTP_1_1 db "HTTP/1.1", NULL_CHAR

; content type
CONTENT_HTML  db "Content-Type: text/html; charset=UTF-8", NULL_CHAR
CONTENT_CSS   db "Content-Type: text/css; charset=UTF-8", NULL_CHAR
CONTENT_JS    db "Content-Type: text/javascript; charset=UTF-8", NULL_CHAR
CONTENT_PLAIN db "Content-Type: text/plain; charset=UTF-8", NULL_CHAR

; connection
CONNECTION_CLOSE db "Connection: close", NULL_CHAR

; content_length
CONTENT_LENGTH db "Content-Length: ", NULL_CHAR

; logs
LOG_PORT_LEN    equ 5 ; port + new line char
LOG_TIME_SIZE   equ 25

; error
ERR_MAX_LEN equ 128

; utils
TO_STRING_MAX equ 7

%endif

