/* --------------------------------------------------------------------------
   Node++ Web App
   Jurek Muszynski
-----------------------------------------------------------------------------
   Web App Performance Tester
-------------------------------------------------------------------------- */


#include <npp.h>


/* ======================================================================= */
/* =============================== SERVICES ============================== */
/* ======================================================================= */


/* --------------------------------------------------------------------------
   Service
-------------------------------------------------------------------------- */
int sendbatch(int ci)
{
    INF("sendbatch");

    INF("batch = %d", SESSION_DATA.batch);
    INF("URL [%s]", SESSION_DATA.url);
    INF("times = %d", SESSION_DATA.times);

    struct timespec start;
    clock_gettime(MONOTONIC_CLOCK_NAME, &start);

    char perfreqid[256];
    bool success;

    for ( int i=0; i<SESSION_DATA.times; ++i )
    {
        DBG_LINE_LONG;
        DBG_LINE_LONG;

        sprintf(perfreqid, "%02d%02d%02d%04d%06d", G_ptm->tm_hour, G_ptm->tm_min, G_ptm->tm_sec, SESSION_DATA.batch, i);
        INF("perfreqid [%s]", perfreqid);
        CALL_HTTP_HEADER_SET("perfreqid", perfreqid);

//        if ( SESSION_DATA.keep )
//            success = npp_call_http(NULL, NULL, "GET", SESSION_DATA.url, FALSE, TRUE);
//        else
            success = CALL_HTTP(NULL, NULL, "GET", SESSION_DATA.url, SESSION_DATA.keep);

        if ( !success )
        {
            ERR("Remote call failed\n");
            SESSION_DATA.elapsed = npp_elapsed(&start);
            return ERR_REMOTE_CALL;
        }

//        INF("Remote call status: %d", CALL_HTTP_STATUS);
    }

    SESSION_DATA.elapsed = npp_elapsed(&start);

    INF("elapsed: %.3lf ms\n", SESSION_DATA.elapsed);

    return OK;
}




/* ======================================================================= */
/* ========================== END OF SERVICES ============================ */
/* ======================================================================= */



/* --------------------------------------------------------------------------
   Entry point
-------------------------------------------------------------------------- */
void npp_svc_main(int ci)
{
    if ( SVC("sendbatch") )
        ASYNC_ERR_CODE = sendbatch(ci);

    OUT("%d|", ASYNC_ERR_CODE);

    if ( ASYNC_ERR_CODE == ERR_REMOTE_CALL )
        OUT("Call failed");
    else if ( ASYNC_ERR_CODE == ERR_REMOTE_CALL_STATUS )
        OUT("Call response status wasn't successful");
    else if ( ASYNC_ERR_CODE == OK )
        OUT("OK");

    OUT("|%s", AMT(G_call_http_average));
    OUT("|%0.3lf", SESSION_DATA.elapsed);

    RES_DONT_CACHE;
}


/* --------------------------------------------------------------------------
   Server start
   Return true if successful
-------------------------------------------------------------------------- */
bool npp_svc_init()
{
    return true;
}


/* --------------------------------------------------------------------------
   Server stop
-------------------------------------------------------------------------- */
void npp_svc_done()
{
}
