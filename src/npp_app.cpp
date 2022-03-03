/* --------------------------------------------------------------------------
   Node++ Web App
   Jurek Muszynski
-----------------------------------------------------------------------------
   Web App Performance Tester
-------------------------------------------------------------------------- */


#include <npp.h>


/* --------------------------------------------------------------------------
   Output HTML & page header
-------------------------------------------------------------------------- */
void gen_header(int ci)
{
    OUT("<!DOCTYPE html>");
    OUT("<html>");
    OUT("<head>");
    OUT("<meta charset=\"UTF-8\">");
    OUT("<title>%s</title>", NPP_APP_NAME);
    OUT("<link rel=\"stylesheet\" type=\"text/css\" href=\"npp.css\">");
    OUT("<script src=\"npp.js\"></script>");
    OUT("<link rel=\"stylesheet\" type=\"text/css\" href=\"dsk.css\">");
    OUT("<script src=\"dsk.js\"></script>");
    OUT("</head>");

    OUT("<body>");

    if ( REQ("") || REQ("dashboard") )
        OUT("<h1>%s</h1>", NPP_APP_NAME);
    else
        OUT("<h1><a href=\"/\" %s>%s</a></h1>", WAIT, NPP_APP_NAME);

    char lnk_home[256]="<a href=\"/\" onClick=\"wait();\">Home</a>";

    if ( REQ("") )
        strcpy(lnk_home, "Home");

    OUT("<div class=mm>");
    if ( !REQ("") ) OUT(lnk_home);
    OUT("</div>");
}


/* --------------------------------------------------------------------------
   Output footer; body & html tags close here
-------------------------------------------------------------------------- */
void gen_footer(int ci)
{
    OUT("</body></html>");
}


/* --------------------------------------------------------------------------
   Show main page
-------------------------------------------------------------------------- */
void gen_page_main(int ci)
{
    gen_header(ci);

    OUT("<table class=m10>");
    OUT("<tr><td class=\"gr rt\">URL:</td><td><input id=\"url\" style=\"width:40em;\" value=\"127.0.0.1:1234\" autofocus %s></td></tr>", ONKEYDOWN);
    OUT("<tr><td class=\"gr rt\">Batches:</td><td><input id=\"batches\" value=\"1\" %s></td></tr>", ONKEYDOWN);
    OUT("<tr><td class=\"gr rt\">Times:</td><td><input id=\"times\" value=\"10\" %s></td></tr>", ONKEYDOWN);
    OUT("<tr><td></td><td><label><input type=\"checkbox\" id=\"keep\" %s> Keep connections open</label></td></tr>", ONKEYDOWN);
    OUT("<tr><td></td><td><button id=\"sbm\" onClick=\"sendreqs();\" style=\"width:7em;height:2.2em;\">Go!</button></td></tr>");
    OUT("</table>");

    gen_footer(ci);
}


/* --------------------------------------------------------------------------
   Send requests (AJAX)
-------------------------------------------------------------------------- */
void sendreqs(int ci)
{
    if ( !QSI("batch", &SESSION_DATA.batch) ) return;
    if ( !QS("url", SESSION_DATA.url) ) return;
    if ( !QSI("times", &SESSION_DATA.times) ) return;

    QSB("keep", &SESSION_DATA.keep);

    if ( SESSION_DATA.times < 1 ) SESSION_DATA.times = 1;
    if ( SESSION_DATA.times > 100000 ) SESSION_DATA.times = 100000;

    INF("batch = %d", SESSION_DATA.batch);
    INF("URL [%s]", SESSION_DATA.url);
    INF("times = %d", SESSION_DATA.times);
    INF("keep = %s", SESSION_DATA.keep?"true":"false");

    CALL_ASYNC_TM("sendreqs", 600);   // 10 minutes timeout
}


/* --------------------------------------------------------------------------------
   This is the main entry point for a request
   ------------------------------
   Called after parsing HTTP request headers
   ------------------------------
   If required (NPP_REQUIRED_AUTH_LEVEL >= AUTH_LEVEL_ANONYMOUS),
   the session is already created

   If valid ls cookie is present in the request or
   it's over existing connection that has already been authenticated,
   the session is already authenticated
   ------------------------------
   Response status is 200 by default
   Use RES_STATUS() if you want to change it

   Response content type is text/html by default
   Use RES_CONTENT_TYPE() if you want to change it
-------------------------------------------------------------------------------- */
void npp_app_main(int ci)
{
    if ( REQ("sendreqs") )
        sendreqs(ci);
    else
        gen_page_main(ci);
}


/* --------------------------------------------------------------------------------
   Called when application starts
   ------------------------------
   Return true if everything OK
   ------------------------------
   Returning false will stop booting process,
   npp_app_done() will be called and application will be terminated
-------------------------------------------------------------------------------- */
bool npp_app_init(int argc, char *argv[])
{
    return true;
}


/* --------------------------------------------------------------------------------
   Called when new anonymous user session starts
   ------------------------------
   Return true if everything OK
   ------------------------------
   Returning false will cause the session to be closed
   and npp_app_session_done() will be called
   Response status will be set to 500
-------------------------------------------------------------------------------- */
bool npp_app_session_init(int ci)
{
    return true;
}


/* --------------------------------------------------------------------------------
   ******* Only for USERS *******
   ------------------------------
   Called after successful authentication (using password or cookie)
   when user session is upgraded from anonymous to logged in
   ------------------------------
   Return true if everything OK
   ------------------------------
   Returning false will cause the session to be downgraded back to anonymous
   and npp_app_user_logout() will be called
-------------------------------------------------------------------------------- */
bool npp_app_user_login(int ci)
{
    return true;
}


/* --------------------------------------------------------------------------------
   ******* Only for USERS *******
   ------------------------------
   Called when downgrading logged in user session to anonymous
-------------------------------------------------------------------------------- */
void npp_app_user_logout(int ci)
{
}


/* --------------------------------------------------------------------------------
   Called when closing anonymous user session
   After calling this the session memory will be zero-ed
-------------------------------------------------------------------------------- */
void npp_app_session_done(int ci)
{
}


/* --------------------------------------------------------------------------------
   Called when application shuts down
-------------------------------------------------------------------------------- */
void npp_app_done()
{
}
