/* --------------------------------------------------------------------------
   Silgy Web App Engine
   Jurek Muszynski
   silgy.com
   Started: August 2015
-------------------------------------------------------------------------- */

#ifndef SILGY_H
#define SILGY_H

#ifdef _WIN32   /* Windows */
#ifdef _MSC_VER /* Microsoft compiler */
/* libraries */
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")   /* GetProcessMemoryInfo */
/* __VA_ARGS__ issue */
#define EXPAND_VA(x) x
/* access function */
#define	F_OK    0       /* test for existence of file */
#define	X_OK    0x01    /* test for execute or search permission */
#define	W_OK    0x02    /* test for write permission */
#define	R_OK    0x04    /* test for read permission */
#endif  /* _MSC_VER */
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 /* Windows XP or higher required */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <psapi.h>
#define CLOCK_MONOTONIC 0   /* dummy */
#undef OUT
#endif  /* _WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <netdb.h>
#include <sys/shm.h>
#include <mqueue.h>
#endif  /* _WIN32 */

#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>

#ifdef __cplusplus
#include <iostream>
#include <cctype>
#else   /* C */
#include <ctype.h>
typedef char                        bool;
#define false                       ((char)0)
#define true                        ((char)1)
#endif  /* __cplusplus */


#define WEB_SERVER_VERSION          "4.1"
/* alias */
#define SILGY_VERSION               WEB_SERVER_VERSION


#ifndef FALSE
#define FALSE                       false
#endif
#ifndef TRUE
#define TRUE                        true
#endif


/* pure C string type */

typedef char str1k[1024];
typedef char str2k[1024*2];
typedef char str4k[1024*4];
typedef char str8k[1024*8];
typedef char str16k[1024*16];
typedef char str32k[1024*32];
typedef char str64k[1024*64];


#include "silgy_app.h"


#ifdef SILGY_WATCHER
#ifdef DBMYSQL
#undef DBMYSQL
#endif
#ifdef USERS
#undef USERS
#endif
#ifdef HTTPS
#undef HTTPS
#endif
#endif  /* SILGY_WATCHER */


#ifdef DBMYSQL
#include <mysql.h>
#include <mysqld_error.h>
#endif

#ifdef HTTPS
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif


#define OK                          0
#define SUCCEED                     OK
#define FAIL                        -1
#define EOS                         ((char)0)       /* End Of String */

/* log levels */

#define LOG_ALWAYS                  0               /* print always */
#define LOG_ERR                     1               /* print errors only */
#define LOG_WAR                     2               /* print errors and warnings */
#define LOG_INF                     3               /* print errors and warnings and info */
#define LOG_DBG                     4               /* for debug mode -- most detailed */

/* request macros */

#define REQ_METHOD                  conn[ci].method
#define REQ_GET                     (0==strcmp(conn[ci].method, "GET"))
#define REQ_POST                    (0==strcmp(conn[ci].method, "POST"))
#define REQ_PUT                     (0==strcmp(conn[ci].method, "PUT"))
#define REQ_DELETE                  (0==strcmp(conn[ci].method, "DELETE"))
#define REQ_URI                     conn[ci].uri
#define REQ_CONTENT_TYPE            conn[ci].in_ctypestr
#define REQ_DSK                     !conn[ci].mobile
#define REQ_MOB                     conn[ci].mobile
#define REQ_BOT                     conn[ci].bot
#define REQ_LANG                    conn[ci].lang

#define PROTOCOL                    (conn[ci].secure?"https":"http")
#define COLON_POSITION              (conn[ci].secure?5:4)

/* defaults */

#ifndef MEM_MEDIUM
#ifndef MEM_BIG
#ifndef MEM_HUGE
#ifndef MEM_SMALL
#define MEM_SMALL   /* default memory model */
#endif
#endif
#endif
#endif

#ifndef OUTFAST
#ifndef OUTCHECK
#define OUTCHECKREALLOC   /* default output type */
#endif
#endif

#ifndef QS_DEF_SQL_ESCAPE
#ifndef QS_DEF_DONT_ESCAPE
#define QS_DEF_HTML_ESCAPE   /* default query string security */
#endif
#endif

/* generate output as fast as possible */

#ifdef SILGY_SVC

    #define OUTSS(str)                  (G_res = stpcpy(G_res, str))
    #define OUT_BIN(data, len)          (len=(len>OUT_BUFSIZE?OUT_BUFSIZE:len), memcpy(res, data, len), res += len)

#else

    #define HOUT(str)                   (conn[ci].p_curr_h = stpcpy(conn[ci].p_curr_h, str))

    #ifdef OUTFAST
        #define OUTSS(str)                  (conn[ci].p_curr_c = stpcpy(conn[ci].p_curr_c, str))
        #define OUT_BIN(data, len)          (len=(len>OUT_BUFSIZE?OUT_BUFSIZE:len), memcpy(conn[ci].p_curr_c, data, len), conn[ci].p_curr_c += len)
    #else
        #ifdef OUTCHECK
            #define OUTSS(str)                  eng_out_check(ci, str)
            #define OUT_BIN(data, len)          (len=(len>OUT_BUFSIZE?OUT_BUFSIZE:len), memcpy(conn[ci].p_curr_c, data, len), conn[ci].p_curr_c += len)
        #else   /* OUTCHECKREALLOC */
            #define OUTSS(str)                  eng_out_check_realloc(ci, str)
            #define OUT_BIN(data, len)          eng_out_check_realloc_bin(ci, data, len)
        #endif
    #endif  /* OUTFAST */

#endif  /* SILGY_SVC */

#ifdef _MSC_VER /* Microsoft compiler */
    #define OUT(...)                        (sprintf(G_tmp, EXPAND_VA(__VA_ARGS__)), OUTSS(G_tmp))
#else   /* GCC */
    #define OUTM(str, ...)                  (sprintf(G_tmp, str, __VA_ARGS__), OUTSS(G_tmp))   /* OUT with multiple args */
    #define CHOOSE_OUT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, NAME, ...) NAME          /* single or multiple? */
    #define OUT(...)                        CHOOSE_OUT(__VA_ARGS__, OUTM, OUTM, OUTM, OUTM, OUTM, OUTM, OUTM, OUTM, OUTM, OUTM, OUTM, OUTM, OUTSS)(__VA_ARGS__)
#endif  /* _MSC_VER */

/* HTTP header -- resets respbuf! */
#define PRINT_HTTP_STATUS(val)          (sprintf(G_tmp, "HTTP/1.1 %d %s\r\n", val, get_http_descr(val)), HOUT(G_tmp))

/* date */
#define PRINT_HTTP_DATE                 (sprintf(G_tmp, "Date: %s\r\n", M_resp_date), HOUT(G_tmp))

/* cache control */
#define PRINT_HTTP_NO_CACHE             HOUT("Cache-Control: private, must-revalidate, no-store, no-cache, max-age=0\r\n")
#define PRINT_HTTP_EXPIRES              (sprintf(G_tmp, "Expires: %s\r\n", M_expires), HOUT(G_tmp))
#define PRINT_HTTP_LAST_MODIFIED(str)   (sprintf(G_tmp, "Last-Modified: %s\r\n", str), HOUT(G_tmp))

/* connection */
#define PRINT_HTTP_CONNECTION(ci)       (sprintf(G_tmp, "Connection: %s\r\n", conn[ci].keep_alive?"Keep-Alive":"close"), HOUT(G_tmp))

/* vary */
#define PRINT_HTTP_VARY_DYN             HOUT("Vary: Accept-Encoding, User-Agent\r\n")
#define PRINT_HTTP_VARY_STAT            HOUT("Vary: Accept-Encoding\r\n")
#define PRINT_HTTP_VARY_UIR             HOUT("Vary: Upgrade-Insecure-Requests\r\n")

/* content language */
#define PRINT_HTTP_LANGUAGE             HOUT("Content-Language: en-us\r\n")

/* framing */
#define PRINT_HTTP_FRAME_OPTIONS        HOUT("X-Frame-Options: SAMEORIGIN\r\n")

/* cookie */
#define PRINT_HTTP_COOKIE_A(ci)         (sprintf(G_tmp, "Set-Cookie: as=%s; HttpOnly\r\n", conn[ci].cookie_out_a), HOUT(G_tmp))
#define PRINT_HTTP_COOKIE_L(ci)         (sprintf(G_tmp, "Set-Cookie: ls=%s; HttpOnly\r\n", conn[ci].cookie_out_l), HOUT(G_tmp))
#define PRINT_HTTP_COOKIE_A_EXP(ci)     (sprintf(G_tmp, "Set-Cookie: as=%s; Expires=%s; HttpOnly\r\n", conn[ci].cookie_out_a, conn[ci].cookie_out_a_exp), HOUT(G_tmp))
#define PRINT_HTTP_COOKIE_L_EXP(ci)     (sprintf(G_tmp, "Set-Cookie: ls=%s; Expires=%s; HttpOnly\r\n", conn[ci].cookie_out_l, conn[ci].cookie_out_l_exp), HOUT(G_tmp))

/* content length */
#define PRINT_HTTP_CONTENT_LEN(len)     (sprintf(G_tmp, "Content-Length: %d\r\n", len), HOUT(G_tmp))

/* identity */
#define PRINT_HTTP_SERVER               HOUT("Server: Silgy\r\n")

/* must be last! */
#define PRINT_HTTP_END_OF_HEADER        HOUT("\r\n")


#define IN_BUFSIZE                      8192            /* incoming request buffer length (8 kB) */
#define OUT_HEADER_BUFSIZE              4096            /* response header buffer length */
#define OUT_BUFSIZE                     262144          /* initial HTTP response buffer length (256 kB) */
#define TMP_BUFSIZE                     1048576         /* temporary string buffer size (1 MB) */
#define MAX_POST_DATA_BUFSIZE           16777216+1048576    /* max incoming POST data length (16+1 MB) */
#define MAX_LOG_STR_LEN                 4095            /* max log string length */
#define MAX_METHOD_LEN                  7               /* method length */
#define MAX_URI_LEN                     2047            /* max request URI length */
#define MAX_LABEL_LEN                   255             /* max request label length */
#define MAX_VALUE_LEN                   255             /* max request value length */
#define MAX_RESOURCE_LEN                127             /* max resource's name length -- as a first part of URI */
#define MAX_RESOURCES                   10000           /* for M_auth_levels */

/* mainly memory usage */

#ifdef SILGY_SVC
#define MAX_CONNECTIONS                 1               /* dummy */
#else
#ifdef MEM_MEDIUM
#define MAX_CONNECTIONS                 200             /* max TCP connections (2 per user session) */
#define MAX_SESSIONS                    100             /* max user sessions */
#elif defined(MEM_BIG)
#define MAX_CONNECTIONS                 1000            /* max TCP connections */
#define MAX_SESSIONS                    500             /* max user sessions */
#elif defined(MEM_HUGE)
#define MAX_CONNECTIONS                 5000            /* max TCP connections */
#define MAX_SESSIONS                    2500            /* max user sessions */
#else   /* MEM_SMALL -- default */
#define MAX_CONNECTIONS                 20              /* max TCP connections */
#define MAX_SESSIONS                    10              /* max user sessions */
#endif
#endif  /* SILGY_SVC */

/* select() vs poll() vs epoll() */

#ifdef _WIN32
#define FD_MON_SELECT   /* WSAPoll doesn't seem to be reliable alternative */
#else
#ifndef FD_MON_POLL
#define FD_MON_SELECT
#endif
#endif  /* _WIN32 */

#ifdef FD_MON_SELECT
#if MAX_CONNECTIONS > FD_SETSIZE-2
#undef MAX_CONNECTIONS
#define MAX_CONNECTIONS FD_SETSIZE-2
#endif
#endif  /* FD_MON_SELECT */

#define CLOSING_SESSION_CI              MAX_CONNECTIONS

#ifndef CONN_TIMEOUT
#define CONN_TIMEOUT                    180             /* idle connection timeout in seconds */
#endif

#ifndef USES_TIMEOUT
#define USES_TIMEOUT                    300             /* anonymous user session timeout in seconds */
#endif

#define NOT_CONNECTED                   -1

#define CONN_STATE_DISCONNECTED         '0'
#define CONN_STATE_ACCEPTING            'a'
#define CONN_STATE_CONNECTED            '1'
#define CONN_STATE_READY_FOR_PARSE      'p'
#define CONN_STATE_READY_FOR_PROCESS    'P'
#define CONN_STATE_READING_DATA         'd'
#define CONN_STATE_WAITING_FOR_ASYNC    'A'
#define CONN_STATE_READY_TO_SEND_HEADER 'H'
#define CONN_STATE_READY_TO_SEND_BODY   'B'
#define CONN_STATE_SENDING_BODY         'S'

#ifdef __linux__
#define MONOTONIC_CLOCK_NAME            CLOCK_MONOTONIC_RAW
#else
#define MONOTONIC_CLOCK_NAME            CLOCK_MONOTONIC
#endif

#define MAX_BLACKLIST                   10000


#ifndef APP_WEBSITE
#define APP_WEBSITE                     "Silgy Web Application"
#endif
#ifndef APP_DOMAIN
#define APP_DOMAIN                      ""
#endif
#ifndef APP_LOGIN_URI
#define APP_LOGIN_URI                   "login"
#endif
#ifndef APP_VERSION
#define APP_VERSION                     "1.0"
#endif


#define SQLBUF                          4096            /* SQL query buffer size */


/* UTF-8 */

#define CHAR_POUND                      "&#163;"
#define CHAR_COPYRIGHT                  "&#169;"
#define CHAR_N_ACUTE                    "&#324;"
#define CHAR_DOWN_ARROWHEAD1            "&#709;"
#define CHAR_LONG_DASH                  "&#8212;"
#define CHAR_EURO                       "&#8364;"
#define CHAR_UP                         "&#8593;"
#define CHAR_DOWN                       "&#8595;"
#define CHAR_MINUS                      "&#8722;"
#define CHAR_VEL                        "&#8744;"
#define CHAR_VERTICAL_ELLIPSIS          "&#8942;"
#define CHAR_COUNTERSINK                "&#9013;"
#define CHAR_DOUBLE_TRIANGLE_U          "&#9195;"
#define CHAR_DOUBLE_TRIANGLE_D          "&#9196;"
#define CHAR_DOWN_TRIANGLE_B            "&#9660;"
#define CHAR_DOWN_TRIANGLE_W            "&#9661;"
#define CHAR_CLOSE                      "&#10005;"
#define CHAR_HEAVY_PLUS                 "&#10133;"
#define CHAR_HEAVY_MINUS                "&#10134;"
#define CHAR_DOWN_ARROWHEAD2            "&#65088;"
#define CHAR_FULLW_PLUS                 "&#65291;"

#define LOGIN_LEN                       30
#define EMAIL_LEN                       120
#define UNAME_LEN                       60
#define PHONE_LEN                       30
#define ABOUT_LEN                       250

#define VIEW_DEFAULT                    '0'
#define VIEW_DESKTOP                    '1'
#define VIEW_MOBILE                     '2'

#ifdef APP_SESID_LEN
#define SESID_LEN                       APP_SESID_LEN
#else
#define SESID_LEN                       15
#endif

#define LANG_LEN                        7

#define EXPIRES_IN_DAYS                 30              /* from app start for Expires HTTP reponse header for static resources */


/* authorization levels */

#define AUTH_LEVEL_NONE                 '0'
#define AUTH_LEVEL_ANONYMOUS            '1'
#define AUTH_LEVEL_LOGGEDIN             '2'
#define AUTH_LEVEL_LOGGED               '2'
#define AUTH_LEVEL_ADMIN                '3'

#ifndef APP_DEF_AUTH_LEVEL
#define APP_DEF_AUTH_LEVEL              AUTH_LEVEL_NONE /* default authorization level */
#endif


/* errors */

/* 0 always means OK */
#define ERR_INVALID_REQUEST             1
#define ERR_UNAUTHORIZED                2
#define ERR_FORBIDDEN                   3
#define ERR_NOT_FOUND                   4
#define ERR_INT_SERVER_ERROR            5
#define ERR_SERVER_TOOBUSY              6
#define ERR_FILE_TOO_BIG                7
#define ERR_REDIRECTION                 8
#define ERR_ASYNC_NO_SUCH_SERVICE       9
#define ERR_ASYNC_TIMEOUT               10
#define ERR_REMOTE_CALL                 11
#define ERR_REMOTE_CALL_STATUS          12
#define ERR_REMOTE_CALL_DATA            13
/* ------------------------------------- */
#define ERR_MAX_ENGINE_ERROR            99
/* ------------------------------------- */


#define MSG_CAT_OK                      "OK"
#define MSG_CAT_ERROR                   "err"
#define MSG_CAT_WARNING                 "war"
#define MSG_CAT_MESSAGE                 "msg"


/* statics */

#define NOT_STATIC                      -1
#ifdef APP_MAX_STATICS                                          /* max static resources */
#define MAX_STATICS                     APP_MAX_STATICS
#else
#define MAX_STATICS                     1000
#endif

#define STATIC_PATH_LEN                 1024

#define STATIC_SOURCE_INTERNAL          0
#define STATIC_SOURCE_RES               1
#define STATIC_SOURCE_RESMIN            2


/* asynchronous calls */

#define ASYNC_STATE_FREE                '0'
#define ASYNC_STATE_SENT                '1'
#define ASYNC_STATE_RECEIVED            '2'
#define ASYNC_STATE_TIMEOUTED           '3'
#ifdef APP_ASYNC_MQ_MAXMSG                                      /* max messages in a message queue */
#define ASYNC_MQ_MAXMSG                 APP_ASYNC_MQ_MAXMSG
#else
#define ASYNC_MQ_MAXMSG                 10
#endif
#define MAX_ASYNC                       ASYNC_MQ_MAXMSG*2       /* max queued async responses */
#ifdef APP_ASYNC_REQ_MSG_SIZE                                   /* request message size */
#define ASYNC_REQ_MSG_SIZE              APP_ASYNC_REQ_MSG_SIZE
#else
#define ASYNC_REQ_MSG_SIZE              8192
#endif
#ifdef APP_ASYNC_RES_MSG_SIZE                                   /* response message size */
#define ASYNC_RES_MSG_SIZE              APP_ASYNC_RES_MSG_SIZE
#else
#define ASYNC_RES_MSG_SIZE              8192
#endif
#define ASYNC_REQ_QUEUE                 "/silgy_req"            /* request queue name */
#define ASYNC_RES_QUEUE                 "/silgy_res"            /* response queue name */
#define ASYNC_DEF_TIMEOUT               30                      /* in seconds */
#define ASYNC_MAX_TIMEOUT               1800                    /* in seconds ==> 30 minutes */
#ifdef SILGY_SVC
#define SVC(svc)                        (0==strcmp(G_service, svc))
#define ASYNC_ERR_CODE                  G_error_code
#else
#define SVC(svc)                        (0==strcmp(conn[ci].service, svc))
#define ASYNC_ERR_CODE                  conn[ci].async_err_code
#endif
#define CALL_ASYNC(svc, data)           eng_async_req(ci, svc, data, TRUE, G_ASYNCDefTimeout, 0)
#define CALL_ASYNC_TM(svc, data, tmout) eng_async_req(ci, svc, data, TRUE, tmout, 0)
#define CALL_ASYNC_NR(svc, data)        eng_async_req(ci, svc, data, FALSE, 0, 0)
#define CALL_ASYNC_BIN(svc, data, size) eng_async_req(ci, svc, data, TRUE, G_ASYNCDefTimeout, size)


/* resource / content types */

/* incoming */

#define CONTENT_TYPE_URLENCODED         'U'
#define CONTENT_TYPE_MULTIPART          'L'

/* outgoing */

#define CONTENT_TYPE_UNSET              '-'
#define CONTENT_TYPE_USER               '+'
#define RES_TEXT                        'T'
#define RES_HTML                        'H'
#define RES_CSS                         'C'
#define RES_JS                          'S'
#define RES_GIF                         'G'
#define RES_JPG                         'J'
#define RES_ICO                         'I'
#define RES_PNG                         'P'
#define RES_BMP                         'B'
#define RES_SVG                         'V'
#define RES_JSON                        'O'
#define RES_PDF                         'A'
#define RES_AMPEG                       'M'
#define RES_EXE                         'X'
#define RES_ZIP                         'Z'


#define REQ0                            conn[ci].resource
#define REQ1                            conn[ci].req1
#define REQ2                            conn[ci].req2
#define REQ3                            conn[ci].req3
#define URI(uri_)                       (0==strcmp(conn[ci].uri, uri_))
#define REQ(res)                        (0==strcmp(conn[ci].resource, res))
#define ID(id)                          (0==strcmp(conn[ci].id, id))
#ifndef SILGY_SVC
#define US                              uses[conn[ci].usi]
#define AUS                             auses[conn[ci].usi]
#else
#define US                              uses
#define AUS                             auses
#endif  /* SILGY_SVC */
#define HOST(str)                       eng_host(ci, str)
#define REQ_GET_HEADER(header)          eng_get_header(ci, header)

#ifdef SILGY_SVC
#define REQ_DATA                        G_req
#else
#define REQ_DATA                        conn[ci].data
#endif

#define REST_HEADER_PASS(header)        eng_rest_header_pass(ci, header)


/* response macros */

#ifdef SILGY_SVC
#define RES_STATUS(val)                 (G_status=val)
#else
#define RES_STATUS(val)                 eng_set_res_status(ci, val)
#endif
#define RES_CONTENT_TYPE(str)           eng_set_res_content_type(ci, str)
#define RES_LOCATION(str, ...)          eng_set_res_location(ci, str, ##__VA_ARGS__)
#define RES_REDIRECT(str, ...)          RES_LOCATION(str, ##__VA_ARGS__)
#define RES_DONT_CACHE                  conn[ci].dont_cache=TRUE
#define RES_KEEP_CONTENT                conn[ci].keep_content=TRUE
#define RES_CONTENT_DISPOSITION(str, ...) eng_set_res_content_disposition(ci, str, ##__VA_ARGS__)

#define REDIRECT_TO_LANDING             sprintf(conn[ci].location, "%s://%s", PROTOCOL, conn[ci].host)

#define OUT_MSG_DESCRIPTION(code)       eng_send_msg_description(ci, code)
#define OUT_HTML_HEADER                 eng_out_html_header(ci)
#define OUT_HTML_FOOTER                 eng_out_html_footer(ci)
#define APPEND_CSS(name, first)         eng_append_css(ci, name, first)
#define APPEND_SCRIPT(name, first)      eng_append_script(ci, name, first)

#define MAX_URI_VAL_LEN                 255             /* max value length received in URI -- sufficient for 99% cases */
#define MAX_LONG_URI_VAL_LEN            65535           /* max long value length received in URI -- 64 kB - 1 B */

#define QSBUF                           MAX_URI_VAL_LEN+1
#define QS_BUF                          QSBUF

#define QS_HTML_ESCAPE(param, val)      get_qs_param_html_esc(ci, param, val)
#define QS_SQL_ESCAPE(param, val)       get_qs_param_sql_esc(ci, param, val)
#define QS_DONT_ESCAPE(param, val)      get_qs_param(ci, param, val)
#define QS_RAW(param, val)              get_qs_param_raw(ci, param, val, MAX_URI_VAL_LEN)

#ifdef QS_DEF_HTML_ESCAPE
#define QS(param, val)                  QS_HTML_ESCAPE(param, val)
#endif
#ifdef QS_DEF_SQL_ESCAPE
#define QS(param, val)                  QS_SQL_ESCAPE(param, val)
#endif
#ifdef QS_DEF_DONT_ESCAPE
#define QS(param, val)                  QS_DONT_ESCAPE(param, val)
#endif

#define SVC_NAME_LEN                    63      /* async service name length */



/* Date-Time */

#define DT_NULL                         "2000-01-01 00:00:00"
#define DT_NOW                          G_dt



/* HTTP status */

typedef struct {
    int     status;
    char    description[128];
} http_status_t;


/* date */

typedef struct {
    short   year;
    char    month;
    char    day;
} date_t;


/* connection */

#ifdef SILGY_SVC
typedef struct {                            /* dummy */
    char    ip[INET_ADDRSTRLEN];
    char    uagent[MAX_VALUE_LEN+1];
    char    host[MAX_VALUE_LEN+1];
    char    website[256];
    char    lang[LANG_LEN+1];
    char    cookie_out_l[SESID_LEN+1];
    char    cookie_out_l_exp[32];
    int     usi;
} conn_t;
#else   /* not SILGY_SVC */
typedef struct {
    /* what comes in */
#ifdef _WIN32   /* Windows */
    SOCKET  fd;                             /* file descriptor */
#else
    int     fd;                             /* file descriptor */
#endif  /* _WIN32 */
    bool    secure;                         /* https? */
    char    ip[INET_ADDRSTRLEN];            /* client IP */
    char    pip[INET_ADDRSTRLEN];           /* proxy IP */
    char    in[IN_BUFSIZE];                 /* the whole incoming request */
    char    method[MAX_METHOD_LEN+1];       /* HTTP method */
    long    was_read;                       /* request bytes read so far */
    bool    upgrade2https;                  /* Upgrade-Insecure-Requests = 1 */
    /* parsed HTTP request starts here */
    bool    head_only;                      /* request method = HEAD */
    bool    post;                           /* request method = POST */
    char    uri[MAX_URI_LEN+1];             /* requested URI string */
    char    resource[MAX_RESOURCE_LEN+1];   /* from URI (REQ0) */
    char    req1[MAX_RESOURCE_LEN+1];       /* from URI -- level 1 */
    char    req2[MAX_RESOURCE_LEN+1];       /* from URI -- level 2 */
    char    req3[MAX_RESOURCE_LEN+1];       /* from URI -- level 3 */
    char    proto[16];                      /* HTTP request protocol */
    char    uagent[MAX_VALUE_LEN+1];        /* user agent string */
    bool    mobile;
    bool    keep_alive;
    char    referer[MAX_VALUE_LEN+1];
    long    clen;                           /* incoming & outgoing content length */
    char    *data;                          /* POST data */
    char    cookie_in_a[SESID_LEN+1];       /* anonymous */
    char    cookie_in_l[SESID_LEN+1];       /* logged in */
    char    host[MAX_VALUE_LEN+1];
    char    website[256];
    char    lang[LANG_LEN+1];
    time_t  if_mod_since;
    char    in_ctypestr[MAX_VALUE_LEN+1];   /* content type as an original string */
    char    in_ctype;                       /* content type */
    char    boundary[MAX_VALUE_LEN+1];      /* for POST multipart/form-data type */
    char    authorization[MAX_VALUE_LEN+1]; /* Authorization */
    /* what goes out */
    char    header[OUT_HEADER_BUFSIZE];     /* outgoing HTTP header */
#ifdef OUTCHECKREALLOC
    char    *out_data;                      /* body */
#else
    char    out_data[OUT_BUFSIZE];
#endif
    long    out_data_allocated;
    int     status;                         /* HTTP status */
    long    data_sent;                      /* how many body bytes has been sent */
    char    ctype;                          /* content type */
    char    ctypestr[256];                  /* user (custom) content type */
    char    cdisp[256];                     /* content disposition */
    time_t  modified;
    char    cookie_out_a[SESID_LEN+1];
    char    cookie_out_a_exp[32];           /* cookie expires */
    char    cookie_out_l[SESID_LEN+1];
    char    cookie_out_l_exp[32];           /* cookie expires */
    char    location[MAX_URI_LEN+1];        /* redirection */
    /* internal stuff */
    long    req;                            /* request count */
    struct timespec proc_start;
    char    conn_state;                     /* connection state (STATE_XXX) */
    char    *p_curr_h;                      /* current header pointer */
    char    *p_curr_c;                      /* current content pointer */
#ifdef HTTPS
    SSL     *ssl;
#endif
    int     ssl_err;
    char    auth_level;                     /* required authorization level */
    int     usi;                            /* user session index */
    int     static_res;                     /* static resource index in M_stat */
    time_t  last_activity;
    bool    bot;
    bool    expect100;
    bool    dont_cache;
    bool    keep_content;                   /* don't reset already rendered content on error */
#ifdef FD_MON_POLL
    int     pi;                             /* pollfds array index */
#endif
#ifdef ASYNC
    char    service[SVC_NAME_LEN+1];
    int     async_err_code;
    int     ai;                             /* async responses array index */
#endif
} conn_t;
#endif  /* SILGY_SVC */


/* user session */

typedef struct {
    char    sesid[SESID_LEN+1];
    /* connection data */
    char    ip[INET_ADDRSTRLEN];
    char    uagent[MAX_VALUE_LEN+1];
    char    referer[MAX_VALUE_LEN+1];
    char    lang[LANG_LEN+1];
    bool    logged;
    /* users table record */
    long    uid;
    char    login[LOGIN_LEN+1];
    char    email[EMAIL_LEN+1];
    char    name[UNAME_LEN+1];
    char    phone[PHONE_LEN+1];
    char    about[ABOUT_LEN+1];
    char    login_tmp[LOGIN_LEN+1];     /* while My Profile isn't saved */
    char    email_tmp[EMAIL_LEN+1];
    char    name_tmp[UNAME_LEN+1];
    char    phone_tmp[PHONE_LEN+1];
    char    about_tmp[ABOUT_LEN+1];
    time_t  last_activity;
} usession_t;


/* static resources */

typedef struct {
    char    name[STATIC_PATH_LEN];
    char    type;
    char    *data;
    long    len;
    time_t  modified;
    char    source;
} stat_res_t;


/* counters */

typedef struct {
    long    req;        /* all parsed requests */
    long    req_dsk;    /* all requests with desktop UA */
    long    req_mob;    /* all requests with mobile UA */
    long    req_bot;    /* all requests with HTTP header indicating well-known search-engine bots */
    long    visits;     /* all visits to domain (Host=APP_DOMAIN) landing page (no action/resource), excl. bots that got 200 */
    long    visits_dsk; /* like visits -- desktop only */
    long    visits_mob; /* like visits -- mobile only */
    long    blocked;    /* attempts from blocked IP */
    double  elapsed;    /* sum of elapsed time of all requests for calculating average */
    double  average;    /* average request elapsed */
} counters_t;


#define MAX_MSG_LEN     255
#define MAX_MESSAGES    1000

typedef struct {
    int  code;
    char lang[8];
    char message[MAX_MSG_LEN+1];
} messages_t;


/* asynchorous processing */

/* request */

typedef struct {
    long    call_id;
    int     ci;
    char    service[SVC_NAME_LEN+1];
    char    ip[INET_ADDRSTRLEN];
    char    uagent[MAX_VALUE_LEN+1];
    char    host[MAX_VALUE_LEN+1];
    char    website[256];
    char    lang[LANG_LEN+1];
    char    response;
    usession_t uses;
#ifdef ASYNC_AUSES
    ausession_t auses;
#endif
    counters_t cnts_today;
    counters_t cnts_yesterday;
    counters_t cnts_day_before;
} async_req_hdr_t;

typedef struct {
    async_req_hdr_t hdr;
    char            data[ASYNC_REQ_MSG_SIZE-sizeof(async_req_hdr_t)];
} async_req_t;

/* response */

typedef struct {
    long    call_id;
    int     ci;
    char    service[SVC_NAME_LEN+1];
    char    state;
    time_t  sent;
    int     timeout;
    int     err_code;
    int     status;
    int     rest_status;
    long    rest_req;
    double  rest_elapsed;
    usession_t uses;
#ifdef ASYNC_AUSES
    ausession_t auses;
#endif
} async_res_hdr_t;

typedef struct {
    async_res_hdr_t hdr;
    char            data[ASYNC_RES_MSG_SIZE-sizeof(async_res_hdr_t)];
} async_res_t;


#ifdef __cplusplus
extern "C" {
#endif

/* read from the config file */

extern int      G_logLevel;
extern int      G_logToStdout;
extern int      G_logCombined;
extern int      G_httpPort;
extern int      G_httpsPort;
extern char     G_cipherList[256];
extern char     G_certFile[256];
extern char     G_certChainFile[256];
extern char     G_keyFile[256];
extern char     G_dbHost[128];
extern int      G_dbPort;
extern char     G_dbName[128];
extern char     G_dbUser[128];
extern char     G_dbPassword[128];
extern int      G_usersRequireAccountActivation;
extern char     G_blockedIPList[256];
extern int      G_ASYNCId;
extern int      G_ASYNCDefTimeout;
extern int      G_RESTTimeout;
extern int      G_test;
/* end of config params */
extern int      G_pid;                      /* pid */
extern char     G_appdir[256];              /* application root dir */
extern long     G_days_up;                  /* web server's days up */
extern conn_t   conn[MAX_CONNECTIONS+1];    /* HTTP connections & requests -- by far the most important structure around */
extern int      G_open_conn;                /* number of open connections */
extern int      G_open_conn_hwm;            /* highest number of open connections (high water mark) */
extern char     G_tmp[TMP_BUFSIZE];         /* temporary string buffer */
#ifndef SILGY_SVC
extern usession_t uses[MAX_SESSIONS+1];     /* engine user sessions -- they start from 1 */
extern ausession_t auses[MAX_SESSIONS+1];   /* app user sessions, using the same index (usi) */
#else
extern usession_t uses;
extern ausession_t auses;
#endif  /* SILGY_SVC */
extern int      G_sessions;                 /* number of active user sessions */
extern int      G_sessions_hwm;             /* highest number of active user sessions (high water mark) */
extern time_t   G_now;                      /* current time */
extern struct tm *G_ptm;                    /* human readable current time */
extern char     G_last_modified[32];        /* response header field with server's start time */
extern messages_t G_messages[MAX_MESSAGES];
extern int      G_next_message;
#ifdef HTTPS
extern bool     G_ssl_lib_initialized;
#endif
#ifdef DBMYSQL
extern MYSQL    *G_dbconn;                  /* database connection */
#endif
#ifndef _WIN32
/* asynchorous processing */
extern char     G_req_queue_name[256];
extern char     G_res_queue_name[256];
extern mqd_t    G_queue_req;                /* request queue */
extern mqd_t    G_queue_res;                /* response queue */
#ifdef ASYNC
extern async_res_t ares[MAX_ASYNC];         /* async response array */
extern long     G_last_call_id;             /* counter */
#endif  /* ASYNC */
#endif  /* _WIN32 */
extern char     G_dt[20];                   /* datetime for database or log (YYYY-MM-DD hh:mm:ss) */
extern bool     G_index_present;            /* index.html present in res? */
#ifdef SILGY_SVC
extern char     *G_req;
extern char     *G_res;
extern char     G_service[SVC_NAME_LEN+1];
extern int      G_error_code;
extern int      G_status;
extern int      ci;
#endif  /* SILGY_SVC */

extern char     G_blacklist[MAX_BLACKLIST+1][INET_ADDRSTRLEN];
extern int      G_blacklist_cnt;            /* M_blacklist length */
/* counters */
extern counters_t G_cnts_today;             /* today's counters */
extern counters_t G_cnts_yesterday;         /* yesterday's counters */
extern counters_t G_cnts_day_before;        /* day before's counters */
/* SHM */
extern char     *G_shm_segptr;              /* SHM pointer */
/* REST */
extern int      G_rest_status;              /* last REST call response status */
extern long     G_rest_req;                 /* REST calls counter */
extern double   G_rest_elapsed;             /* REST calls elapsed for calculating average */
extern double   G_rest_average;             /* REST calls average elapsed */
extern char     G_rest_content_type[MAX_VALUE_LEN+1];
extern bool     G_dont_use_current_session;
extern long     G_new_user_id;

#ifdef __cplusplus
}   /* extern "C" */
#endif


#include <silgy_lib.h>

#ifdef USERS
#include <silgy_usr.h>
#endif



/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

    /* public engine functions */

    void silgy_set_auth_level(const char *resource, char level);
    int  eng_uses_start(int ci, const char *sesid);
    void eng_uses_close(int usi);
    void eng_uses_reset(int usi);
    void eng_async_req(int ci, const char *service, const char *data, char response, int timeout, int size);
    void silgy_add_to_static_res(const char *name, const char *src);
    void eng_send_msg_description(int ci, int code);
    void eng_block_ip(const char *value, bool autoblocked);
    bool eng_host(int ci, const char *host);
    void eng_set_res_status(int ci, int status);
    void eng_set_res_content_type(int ci, const char *str);
    void eng_set_res_location(int ci, const char *str, ...);
    void eng_set_res_content_disposition(int ci, const char *str, ...);
    void eng_out_check(int ci, const char *str);
    void eng_out_check_realloc(int ci, const char *str);
    void eng_out_check_realloc_bin(int ci, const char *data, long len);
    void eng_out_html_header(int ci);
    void eng_out_html_footer(int ci);
    void eng_append_css(int ci, const char *fname, bool first);
    void eng_append_script(int ci, const char *fname, bool first);
    void eng_send_msg_description(int ci, int errcode);
    bool get_qs_param_html_esc(int ci, const char *fieldname, char *retbuf);
    bool get_qs_param_sql_esc(int ci, const char *fieldname, char *retbuf);
    bool get_qs_param(int ci, const char *fieldname, char *retbuf);
    bool get_qs_param_raw(int ci, const char *fieldname, char *retbuf, int maxlen);
    bool get_qs_param_long(int ci, const char *fieldname, char *retbuf);
    bool get_qs_param_multipart_txt(int ci, const char *fieldname, char *retbuf);
    char *get_qs_param_multipart(int ci, const char *fieldname, long *retlen, char *retfname);
    char *eng_get_header(int ci, const char *header);
    void eng_rest_header_pass(int ci, const char *header);

    /* public app functions */

#ifdef SILGY_SVC
    bool silgy_svc_init(void);
    void silgy_svc_main(void);
    void silgy_svc_done(void);
#else /* not SILGY_SVC */
    bool silgy_app_init(int argc, char *argv[]);
    void silgy_app_done(void);
    void silgy_app_main(int ci);
    bool silgy_app_session_init(int ci);
#ifdef USERS
    bool silgy_app_user_login(int ci);
    void silgy_app_user_logout(int ci);
#endif
    void silgy_app_session_done(int ci);
#ifdef ASYNC
    void silgy_app_continue(int ci, const char *data);
#endif
#ifdef APP_ERROR_PAGE
    void silgy_app_error_page(int ci, int code);
#endif
#ifdef EVERY_SECOND
    void app_every_second(void);
#endif
#endif  /* SILGY_SVC */

#ifdef __cplusplus
}   /* extern "C" */
#endif



#endif  /* SILGY_H */
