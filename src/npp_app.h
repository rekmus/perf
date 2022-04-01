/* --------------------------------------------------------------------------
   Silgy Web App
   Jurek Muszynski
-----------------------------------------------------------------------------
   Web App Performance Tester
-------------------------------------------------------------------------- */

#ifndef NPP_APP_H
#define NPP_APP_H


#define NPP_APP_NAME                    "Web App Performance Tester"
#define NPP_REQUIRED_AUTH_LEVEL         AUTH_LEVEL_ANONYMOUS


/* List of additional C/C++ modules to compile. They have to be one-liners */

#define NPP_APP_MODULES                 ""
#define NPP_SVC_MODULES                 NPP_APP_MODULES


#define NPP_ASYNC
#define NPP_ASYNC_INCLUDE_SESSION_DATA

#define NPP_MEM_MEDIUM


#define NPP_HTTPS
#define NPP_NO_HSTS

//#define NPP_FD_MON_LINUX_POLL
//#define NPP_DEBUG


#define WAIT                        "onClick=\"wait();\""
#define ONKEYDOWN                   "onkeydown=\"ent(event);\""


/* app session data */
/* accessible via SESSION_DATA macro */

typedef struct {
    char url[256];
    int  batch;
    int  times;
    bool keep;
    double elapsed;
} app_session_data_t;


#endif  /* NPP_APP_H */
