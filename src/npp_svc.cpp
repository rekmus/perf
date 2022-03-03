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
int sendreqs()
{
    INF("sendreqs");

    INF("batch = %d", SESSION_DATA.batch);
    INF("URL [%s]", SESSION_DATA.url);
    INF("times = %d", SESSION_DATA.times);

    struct timespec start;
    clock_gettime(MONOTONIC_CLOCK_NAME, &start);

    str1k tmp;

    for ( int i=0; i<SESSION_DATA.times; ++i )
    {
        DBG_LINE_LONG;
        DBG_LINE_LONG;

        sprintf(tmp, "%02d%02d%02d%04d%04d", G_ptm->tm_hour, G_ptm->tm_min, G_ptm->tm_sec, SESSION_DATA.batch, i);
        DBG("perfreqid: %s", tmp);
        REST_HEADER_SET("perfreqid", tmp);

        if ( !CALL_HTTP(NULL, NULL, "GET", SESSION_DATA.url) )
        {
            ERR("Remote call failed\n");
            SESSION_DATA.elapsed = lib_elapsed(&start);
            return ERR_REMOTE_CALL;
        }

        DBG("Remote call OK\n");
    }

    SESSION_DATA.elapsed = lib_elapsed(&start);

    INF("elapsed: %.3lf ms\n", SESSION_DATA.elapsed);

    return OK;
}




/* ======================================================================= */
/* ========================== END OF SERVICES ============================ */
/* ======================================================================= */



/* --------------------------------------------------------------------------
   Entry point
-------------------------------------------------------------------------- */
void npp_svc_main()
{
    if ( SVC("sendreqs") )
        ASYNC_ERR_CODE = sendreqs();

    OUT("%d|", ASYNC_ERR_CODE);

    if ( ASYNC_ERR_CODE == ERR_REMOTE_CALL )
        OUT("Call failed");
    else if ( ASYNC_ERR_CODE == ERR_REMOTE_CALL_STATUS )
        OUT("Call response status wasn't successful");
    else if ( ASYNC_ERR_CODE == OK )
        OUT("OK");

    OUT("|%s", AMT(G_call_http_average));
    OUT("|%f", SESSION_DATA.elapsed);

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
