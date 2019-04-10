/* --------------------------------------------------------------------------
   Silgy Web App
   Jurek Muszynski
-----------------------------------------------------------------------------
   Sample service module
-------------------------------------------------------------------------- */


#include <silgy.h>


ausession_t auses;


/* ======================================================================= */
/* =============================== SERVICES ============================== */
/* ======================================================================= */


/* --------------------------------------------------------------------------
   Service
-------------------------------------------------------------------------- */
int sendreqs()
{
    INF("endreqs");

    INF("URL [%s]", AUS.url);
    INF("times = %d", AUS.times);

    struct timespec start;
    clock_gettime(MONOTONIC_CLOCK_NAME, &start);

    for ( int i=0; i<AUS.times; ++i )
    {
        DBG("Request #%d", i);

        if ( !CALL_HTTP(NULL, NULL, "GET", AUS.url) )
        {
            ERR("Remote call failed");
            return ERR_REMOTE_CALL;
        }
        else if ( !CALL_HTTP_STATUS_OK )
        {
            WAR("Remote call status %d", CALL_HTTP_STATUS);
            return ERR_REMOTE_CALL_STATUS;
        }

        DBG("Remote call OK");
    }

    AUS.elapsed = lib_elapsed(&start);

    INF("elapsed: %.3lf ms\n", AUS.elapsed);

    return OK;
}




/* ======================================================================= */
/* ========================== END OF SERVICES ============================ */
/* ======================================================================= */



/* --------------------------------------------------------------------------
   Entry point
-------------------------------------------------------------------------- */
void silgy_svc_main()
{
    if ( SVC("sendreqs") )
        ASYNC_ERR_CODE = sendreqs();
}


/* --------------------------------------------------------------------------
   Server start
   Return true if successful
-------------------------------------------------------------------------- */
bool silgy_svc_init()
{
    return true;
}


/* --------------------------------------------------------------------------
   Server stop
-------------------------------------------------------------------------- */
void silgy_svc_done()
{
}
