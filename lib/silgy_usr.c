/* --------------------------------------------------------------------------
   Silgy Web App Engine
   Jurek Muszynski
   silgy.com
-----------------------------------------------------------------------------
   Logged in users' functions
-------------------------------------------------------------------------- */


#include <silgy.h>


#ifdef USERS


long     G_new_user_id=0;


static bool valid_username(const char *login);
static bool valid_email(const char *email);
static int  upgrade_uses(int ci, long uid, const char *login, const char *email, const char *name, const char *phone, const char *about, short auth_level);
static void downgrade_uses(int usi, int ci, bool usr_logout);
static int  user_exists(const char *login);
static int  email_exists(const char *email);
static int  do_login(int ci, long uid, char *p_login, char *p_email, char *p_name, char *p_phone, char *p_about, short p_auth_level, long visits, short status);
static void get_hashes(char *result1, char *result2, const char *login, const char *email, const char *passwd);
static void doit(char *result1, char *result2, const char *usr, const char *email, const char *src);
static long get_max(int ci, const char *table);


/* --------------------------------------------------------------------------
   Library init
-------------------------------------------------------------------------- */
void libusr_init()
{
    DBG("libusr_init");

    silgy_add_message(ERR_INVALID_LOGIN,            "EN-US", "Invalid login and/or password");
    silgy_add_message(ERR_USERNAME_TOO_SHORT,       "EN-US", "User name must be at least %d characters long", MIN_USERNAME_LEN);
    silgy_add_message(ERR_USERNAME_CHARS,           "EN-US", "User name may only contain letters, digits, dots, hyphens, underscores or apostrophes");
    silgy_add_message(ERR_USERNAME_TAKEN,           "EN-US", "Unfortunately this login has already been taken");
    silgy_add_message(ERR_EMAIL_EMPTY,              "EN-US", "Your email address can't be empty");
    silgy_add_message(ERR_EMAIL_FORMAT,             "EN-US", "Please enter valid email address");
    silgy_add_message(ERR_EMAIL_FORMAT_OR_EMPTY,    "EN-US", "Please enter valid email address or leave this field empty");
    silgy_add_message(ERR_EMAIL_TAKEN,              "EN-US", "This email address has already been registered");
    silgy_add_message(ERR_INVALID_PASSWORD,         "EN-US", "Please enter your existing password");
    silgy_add_message(ERR_PASSWORD_TOO_SHORT,       "EN-US", "Password must be at least %d characters long", MIN_PASSWORD_LEN);
    silgy_add_message(ERR_PASSWORD_DIFFERENT,       "EN-US", "Please retype password exactly like in the previous field");
    silgy_add_message(ERR_OLD_PASSWORD,             "EN-US", "Please enter your existing password");
    silgy_add_message(ERR_SESSION_EXPIRED,          "EN-US", "Your session has expired. Please log in to continue:");
    silgy_add_message(ERR_LINK_BROKEN,              "EN-US", "It looks like this link is broken. If you clicked on the link you've received from us in email, you can try to copy and paste it in your browser's address bar instead.");
    silgy_add_message(ERR_LINK_MAY_BE_EXPIRED,      "EN-US", "Your link is invalid or may be expired");
    silgy_add_message(ERR_LINK_EXPIRED,             "EN-US", "It looks like you entered email that doesn't exist in our database or your link has expired");
    silgy_add_message(ERR_LINK_TOO_MANY_TRIES,      "EN-US", "It looks like you entered email that doesn't exist in our database or your link has expired");
    silgy_add_message(ERR_ROBOT,                    "EN-US", "I'm afraid you are a robot?");
    silgy_add_message(ERR_WEBSITE_FIRST_LETTER,     "EN-US", "The first letter of this website's name should be %c", APP_WEBSITE[0]);
    silgy_add_message(ERR_NOT_ACTIVATED,            "EN-US", "Your account requires activation. Please check your mailbox for a message from %s.", APP_WEBSITE);

    silgy_add_message(WAR_NO_EMAIL,                 "EN-US", "You didn't provide your email address. This is fine, however please remember that in case you forget your password, there's no way for us to send you reset link.");
    silgy_add_message(WAR_BEFORE_DELETE,            "EN-US", "You are about to delete your %s's account. All your details and data will be removed from our database. If you are sure you want this, enter your password and click 'Delete my account'.", APP_WEBSITE);
    silgy_add_message(WAR_ULA_FIRST,                "EN-US", "Someone has tried to log in to this account unsuccessfully more than %d times. To protect your account from brute-force attack, this system requires you to wait for at least a minute before trying again.", MAX_ULA_BEFORE_FIRST_SLOW);
    silgy_add_message(WAR_ULA_SECOND,               "EN-US", "Someone has tried to log in to this account unsuccessfully more than %d times. To protect your account from brute-force attack, this system requires you to wait for at least an hour before trying again.", MAX_ULA_BEFORE_SECOND_SLOW);
    silgy_add_message(WAR_ULA_THIRD,                "EN-US", "Someone has tried to log in to this account unsuccessfully more than %d times. To protect your account from brute-force attack, this system requires you to wait for at least 23 hours before trying again.", MAX_ULA_BEFORE_THIRD_SLOW);
    silgy_add_message(WAR_PASSWORD_CHANGE,          "EN-US", "You have to change your password");

    silgy_add_message(MSG_WELCOME_NO_ACTIVATION,    "EN-US", "Welcome to %s! You can now log in:", APP_WEBSITE);
    silgy_add_message(MSG_WELCOME_NEED_ACTIVATION,  "EN-US", "Welcome to %s! Your account requires activation. Please check your mailbox for a message from %s.", APP_WEBSITE, APP_WEBSITE);
    silgy_add_message(MSG_WELCOME_AFTER_ACTIVATION, "EN-US", "Very well! You can now log in:");
    silgy_add_message(MSG_USER_LOGGED_OUT,          "EN-US", "You've been successfully logged out");
    silgy_add_message(MSG_CHANGES_SAVED,            "EN-US", "Your changes have been saved");
    silgy_add_message(MSG_REQUEST_SENT,             "EN-US", "Your request has been sent. Please check your mailbox for a message from %s.", APP_WEBSITE);
    silgy_add_message(MSG_PASSWORD_CHANGED,         "EN-US", "Your password has been changed. You can now log in:");
    silgy_add_message(MSG_MESSAGE_SENT,             "EN-US", "Your message has been sent");
    silgy_add_message(MSG_PROVIDE_FEEDBACK,         "EN-US", "%s would suit me better if...", APP_WEBSITE);
    silgy_add_message(MSG_FEEDBACK_SENT,            "EN-US", "Thank you for your feedback!");
    silgy_add_message(MSG_USER_ALREADY_ACTIVATED,   "EN-US", "Your account has already been activated");
    silgy_add_message(MSG_ACCOUNT_DELETED,          "EN-US", "Your user account has been deleted. Thank you for trying %s!", APP_WEBSITE);
}


/* --------------------------------------------------------------------------
   Return TRUE if user name contains only valid characters
-------------------------------------------------------------------------- */
static bool valid_username(const char *login)
{
    int i;

    for ( i=0; login[i] != EOS; ++i )
    {
        if ( !isalnum(login[i]) && login[i] != '.' && login[i] != '_' && login[i] != '-' && login[i] != '\'' )
            return FALSE;
    }

    return TRUE;
}


/* --------------------------------------------------------------------------
   Return TRUE if email has valid format
-------------------------------------------------------------------------- */
static bool valid_email(const char *email)
{
    int     len;
    const char *at;
    int     i;

    len = strlen(email);

    if ( len < 3 ) return FALSE;

    at = strchr(email, '@');

    if ( !at ) return FALSE;                /* no @ */
    if ( at==email ) return FALSE;          /* @ is first */
    if ( at==email+len-1 ) return FALSE;    /* @ is last */

    for ( i=0; i<len; ++i )
    {
        if ( !isalnum(email[i]) && email[i] != '@' && email[i] != '.' && email[i] != '_' && email[i] != '-' )
            return FALSE;
    }

    return TRUE;
}


/* --------------------------------------------------------------------------
   Upgrade anonymous user session to logged in
-------------------------------------------------------------------------- */
static int upgrade_uses(int ci, long uid, const char *login, const char *email, const char *name, const char *phone, const char *about, short auth_level)
{
    DBG("upgrade_uses");

    DBG("Upgrading anonymous session to logged in, usi=%d, sesid [%s]", conn[ci].usi, US.sesid);

    US.logged = TRUE;
    strcpy(US.login, login);
    strcpy(US.email, email);
    strcpy(US.name, name);
    strcpy(US.phone, phone);
    strcpy(US.about, about);
    strcpy(US.login_tmp, login);
    strcpy(US.email_tmp, email);
    strcpy(US.name_tmp, name);
    strcpy(US.phone_tmp, phone);
    strcpy(US.about_tmp, about);
    US.auth_level = auth_level;
    US.uid = uid;

#ifndef SILGY_SVC
    if ( !silgy_app_user_login(ci) )
    {
        downgrade_uses(conn[ci].usi, ci, FALSE);
        return ERR_INT_SERVER_ERROR;
    }
#endif

    strcpy(conn[ci].cookie_out_a, "x");                     /* no longer needed */
    strcpy(conn[ci].cookie_out_a_exp, G_last_modified);     /* to be removed by browser */

    return OK;
}


#ifndef SILGY_SVC   /* this is for engine only */
/* --------------------------------------------------------------------------
   Verify IP & User-Agent against uid and sesid in uses (logged in users)
   set user session array index (usi) if all ok
-------------------------------------------------------------------------- */
int libusr_luses_ok(int ci)
{
    int ret=OK;
    int i;

    DBG("libusr_luses_ok");

    /* try in hot sessions first */

    if ( conn[ci].usi )   /* existing connection */
    {
        if ( uses[conn[ci].usi].sesid[0]
                && uses[conn[ci].usi].logged
                && 0==strcmp(conn[ci].cookie_in_l, uses[conn[ci].usi].sesid)
                && 0==strcmp(conn[ci].uagent, uses[conn[ci].usi].uagent) )
        {
            DBG("Logged in session found in cache, usi=%d, sesid [%s]", conn[ci].usi, uses[conn[ci].usi].sesid);
            return OK;
        }
        else    /* session was closed */
        {
            conn[ci].usi = 0;
        }
    }
    else    /* fresh connection */
    {
        for ( i=1; i<=MAX_SESSIONS; ++i )
        {
            if ( uses[i].sesid[0]
                    && uses[i].logged
                    && 0==strcmp(conn[ci].cookie_in_l, uses[i].sesid)
                    && 0==strcmp(conn[ci].uagent, uses[i].uagent) )
            {
                DBG("Logged in session found in cache, usi=%d, sesid [%s]", i, uses[i].sesid);
                conn[ci].usi = i;
                return OK;
            }
        }
    }

    /* not found in memory -- try database */

    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;
    long        uid;
    time_t      created;

    char sanuagent[DB_UAGENT_LEN+1];
    sanitize_sql(sanuagent, conn[ci].uagent, DB_UAGENT_LEN);

    char sanlscookie[SESID_LEN+1];
    sanitize_sql(sanlscookie, conn[ci].cookie_in_l, SESID_LEN);

    sprintf(sql_query, "SELECT uagent, user_id, created FROM users_logins WHERE sesid = BINARY '%s'", sanlscookie);
    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    DBG("users_logins: %lu record(s) found", sql_records);

    if ( 0 == sql_records )     /* no such session in database */
    {
        mysql_free_result(result);
        WAR("No logged in session in database [%s]", sanlscookie);
        strcpy(conn[ci].cookie_out_l, "x");
        strcpy(conn[ci].cookie_out_l_exp, G_last_modified);     /* expire ls cookie */

        /* ---------------------------------------------------------------------------------- */
        /* brute force ls cookie attack prevention */

        /* maintain the list of last n IPs with failed ls cookie authentication with counters */

        static failed_login_cnt_t failed_cnt[FAILED_LOGIN_CNT_SIZE];
        static int failed_cnt_used=0;
        static int failed_cnt_next=0;
        char found=0;

        for ( i=0; i<failed_cnt_used && i<FAILED_LOGIN_CNT_SIZE; ++i )
        {
            if ( 0==strcmp(conn[ci].ip, failed_cnt[i].ip) )
            {
                if ( (failed_cnt[i].cnt > 10 && failed_cnt[i].when > G_now-60)      /* 10 failed attempts within a minute or */
                    || (failed_cnt[i].cnt > 100 && failed_cnt[i].when > G_now-3600) /* 100 failed attempts within an hour or */
                    || failed_cnt[i].cnt > 1000 )                                   /* 1000 failed attempts */
                {
#ifndef SILGY_SVC
                    WAR("Looks like brute-force cookie attack, blocking IP");
                    eng_block_ip(conn[ci].ip, TRUE);
#else
                    WAR("Looks like brute-force cookie attack");
#endif  /* SILGY_SVC */
                }
                else
                {
                    ++failed_cnt[i].cnt;
                }

                found = 1;
                break;
            }
        }

        if ( !found )   /* add record to failed_cnt array */
        {
            strcpy(failed_cnt[failed_cnt_next].ip, conn[ci].ip);
            failed_cnt[failed_cnt_next].cnt = 1;
            failed_cnt[failed_cnt_next].when = G_now;
            
            if ( failed_cnt_next >= FAILED_LOGIN_CNT_SIZE-1 )    /* last slot was just used -- roll over */
                failed_cnt_next = 0;
            else
            {
                ++failed_cnt_next;

                if ( failed_cnt_used < FAILED_LOGIN_CNT_SIZE )   /* before first roll-over */
                    ++failed_cnt_used;
            }
        }

        /* ---------------------------------------------------------------------------------- */

        return ERR_SESSION_EXPIRED;
    }

    /* we've got some user login cookie remembered */

    sql_row = mysql_fetch_row(result);

    /* verify uagent */

    if ( 0 != strcmp(sanuagent, sql_row[0]) )
    {
        mysql_free_result(result);
        DBG("Different uagent in database for sesid [%s]", sanlscookie);
        strcpy(conn[ci].cookie_out_l, "x");
        strcpy(conn[ci].cookie_out_l_exp, G_last_modified);     /* expire ls cookie */
        return ERR_SESSION_EXPIRED;
    }

    /* -------------------------------------- */

    uid = atol(sql_row[1]);

    /* Verify time. If created more than 30 days ago -- refuse */

    created = db2epoch(sql_row[2]);

    if ( created < G_now - 3600*24*30 )
    {
        DBG("Removing old logged in session, usi=%d, sesid [%s], created %s from database", conn[ci].usi, sanlscookie, sql_row[2]);

        mysql_free_result(result);

        sprintf(sql_query, "DELETE FROM users_logins WHERE sesid = BINARY '%s'", sanlscookie);
        DBG("sql_query: %s", sql_query);

        if ( mysql_query(G_dbconn, sql_query) )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            return ERR_INT_SERVER_ERROR;
        }

        /* tell browser we're logging out */

        strcpy(conn[ci].cookie_out_l, "x");
        strcpy(conn[ci].cookie_out_l_exp, G_last_modified);     /* expire ls cookie */

        return ERR_SESSION_EXPIRED;
    }

    mysql_free_result(result);

    /* cookie has not expired -- log user in */

    DBG("Logged in session found in database");

    /* start a fresh session, keep the old sesid */

    ret = eng_uses_start(ci, sanlscookie);

    if ( ret != OK )
        return ret;

    sprintf(sql_query, "UPDATE users_logins SET last_used='%s' WHERE sesid = BINARY '%s'", G_dt, US.sesid);
    DBG("sql_query: %s", sql_query);
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    return do_login(ci, uid, NULL, NULL, NULL, NULL, NULL, 0, 0, 0);
}
#endif  /* SILGY_SVC */


/* --------------------------------------------------------------------------
   Close timeouted logged in user sessions
-------------------------------------------------------------------------- */
void libusr_luses_close_timeouted()
{
    int     i;
    time_t  last_allowed;

    last_allowed = G_now - LUSES_TIMEOUT;

    for ( i=1; G_sessions>0 && i<=MAX_SESSIONS; ++i )
    {
        if ( uses[i].sesid[0] && uses[i].logged && uses[i].last_activity < last_allowed )
            downgrade_uses(i, NOT_CONNECTED, FALSE);
    }
}


/* --------------------------------------------------------------------------
   Invalidate active user sessions belonging to user_id
   Called after password change
-------------------------------------------------------------------------- */
static void downgrade_uses_by_uid(long uid, int ci)
{
    int i;

    if ( ci > -1 )  /* keep the current session */
    {
        for ( i=1; G_sessions>0 && i<=MAX_SESSIONS; ++i )
        {
            if ( uses[i].sesid[0] && uses[i].logged && uses[i].uid==uid && 0!=strcmp(uses[i].sesid, US.sesid) )
                downgrade_uses(i, NOT_CONNECTED, FALSE);
        }
    }
    else    /* all sessions */
    {
        for ( i=1; G_sessions>0 && i<=MAX_SESSIONS; ++i )
        {
            if ( uses[i].sesid[0] && uses[i].logged && uses[i].uid==uid )
                downgrade_uses(i, NOT_CONNECTED, FALSE);
        }
    }
}


/* --------------------------------------------------------------------------
   Downgrade logged in user session to anonymous
-------------------------------------------------------------------------- */
static void downgrade_uses(int usi, int ci, bool usr_logout)
{
    char sql_query[SQLBUF];

    DBG("downgrade_uses");

    DBG("Downgrading logged in session to anonymous, usi=%d, sesid [%s]", usi, uses[usi].sesid);

    uses[usi].logged = FALSE;
    uses[usi].uid = 0;
    uses[usi].login[0] = EOS;
    uses[usi].email[0] = EOS;
    uses[usi].name[0] = EOS;
    uses[usi].phone[0] = EOS;
    uses[usi].about[0] = EOS;
    uses[usi].login_tmp[0] = EOS;
    uses[usi].email_tmp[0] = EOS;
    uses[usi].name_tmp[0] = EOS;
    uses[usi].phone_tmp[0] = EOS;
    uses[usi].about_tmp[0] = EOS;
    uses[usi].auth_level = AUTH_LEVEL_ANONYMOUS;

    if ( ci != NOT_CONNECTED )   /* still connected */
    {
#ifndef SILGY_SVC
        silgy_app_user_logout(ci);
#endif
    }
    else    /* trick to maintain consistency across silgy_app_xxx functions */
    {       /* that use ci for everything -- even to get user session data */
        conn[CLOSING_SESSION_CI].usi = usi;
#ifndef SILGY_SVC
        silgy_app_user_logout(CLOSING_SESSION_CI);
#endif
    }

    if ( usr_logout )   /* explicit user logout */
    {
        sprintf(sql_query, "DELETE FROM users_logins WHERE sesid = BINARY '%s'", uses[usi].sesid);
        DBG("sql_query: %s", sql_query);
        if ( mysql_query(G_dbconn, sql_query) )
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));

        if ( ci != NOT_CONNECTED )   /* still connected */
        {
            strcpy(conn[ci].cookie_out_l, "x");
            strcpy(conn[ci].cookie_out_l_exp, G_last_modified);     /* in the past => to be removed by browser straight away */

            strcpy(conn[ci].cookie_out_a, uses[usi].sesid);
        }
    }
}


/* --------------------------------------------------------------------------
   Check whether user exists in database
-------------------------------------------------------------------------- */
static int user_exists(const char *login)
{
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    long        records;

    DBG("user_exists, login [%s]", login);

//  if ( 0==strcmp(sanlogin, "ADMIN") )
//      return ERR_USERNAME_TAKEN;

    sprintf(sql_query, "SELECT id FROM users WHERE login_u='%s'", upper(login));

    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    records = mysql_num_rows(result);

    DBG("users: %ld record(s) found", records);

    mysql_free_result(result);

    if ( 0 != records )
        return ERR_USERNAME_TAKEN;

    return OK;
}


/* --------------------------------------------------------------------------
   Check whether email exists in database
-------------------------------------------------------------------------- */
static int email_exists(const char *email)
{
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    long        records;

    DBG("email_exists, email [%s]", email);

    sprintf(sql_query, "SELECT id FROM users WHERE email_u='%s'", upper(email));

    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    records = mysql_num_rows(result);

    DBG("users: %ld record(s) found", records);

    mysql_free_result(result);

    if ( 0 != records )
        return ERR_EMAIL_TAKEN;

    return OK;
}


/* --------------------------------------------------------------------------
   Log user in -- called either by l_usession_ok or silgy_usr_login
   Authentication has already been done prior to calling this
-------------------------------------------------------------------------- */
static int do_login(int ci, long uid, char *p_login, char *p_email, char *p_name, char *p_phone, char *p_about, short p_auth_level, long visits, short status)
{
    int         ret=OK;
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;
    char        login[LOGIN_LEN+1];
    char        email[EMAIL_LEN+1];
    char        name[UNAME_LEN+1];
    char        phone[PHONE_LEN+1];
    char        about[ABOUT_LEN+1];
    short       auth_level;

    DBG("do_login");

    /* get user record by id */

    if ( !p_login )   /* login from cookie */
    {
        sprintf(sql_query, "SELECT login,email,name,phone,about,auth_level,visits FROM users WHERE id=%ld", uid);
        DBG("sql_query: %s", sql_query);
        mysql_query(G_dbconn, sql_query);
        result = mysql_store_result(G_dbconn);
        if ( !result )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            return ERR_INT_SERVER_ERROR;
        }

        sql_records = mysql_num_rows(result);

        DBG("users: %lu record(s) found", sql_records);

        if ( 0 == sql_records )
        {
            mysql_free_result(result);
            WAR("Cookie sesid does not match user id");
            return ERR_INVALID_LOGIN;   /* invalid user and/or password */
        }

        /* user found */

        sql_row = mysql_fetch_row(result);

        strcpy(login, sql_row[0]?sql_row[0]:"");
        strcpy(email, sql_row[1]?sql_row[1]:"");
        strcpy(name, sql_row[2]?sql_row[2]:"");
        strcpy(phone, sql_row[3]?sql_row[3]:"");
        strcpy(about, sql_row[4]?sql_row[4]:"");
        auth_level = sql_row[5]?atoi(sql_row[5]):DEF_USER_AUTH_LEVEL;
        visits = atol(sql_row[6]);

        mysql_free_result(result);
    }
    else
    {
        strcpy(login, p_login);
        strcpy(email, p_email);
        strcpy(name, p_name);
        strcpy(phone, p_phone);
        strcpy(about, p_about);
        auth_level = p_auth_level;
    }

    /* admin? */
#ifdef USERSBYEMAIL
#ifdef APP_ADMIN_EMAIL
    if ( 0==strcmp(email, APP_ADMIN_EMAIL) )
        strcpy(login, "admin");
#endif
#endif  /* USERSBYEMAIL */

    /* upgrade anonymous session to logged in */

    ret = upgrade_uses(ci, uid, login, email, name, phone, about, auth_level);
    if ( ret != OK )
        return ret;

    /* update user record */

    sprintf(sql_query, "UPDATE users SET visits=%ld, last_login='%s' WHERE id=%ld", visits+1, G_dt, uid);
    DBG("sql_query: %s", sql_query);
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

#ifdef USERSBYEMAIL
    INF("User [%s] logged in", US.email);
#else
    INF("User [%s] logged in", US.login);
#endif

    /* password change required? */

    if ( status == USER_STATUS_PASSWORD_CHANGE )
    {
        WAR("User is required to change their password");
        return WAR_PASSWORD_CHANGE;
    }

    return ret;
}


/* --------------------------------------------------------------------------
   Send activation link
-------------------------------------------------------------------------- */
static int send_activation_link(int ci, const char *login, const char *email)
{
    char linkkey[PASSWD_RESET_KEY_LEN+1];
    char sql_query[SQLBUF];
    
    /* generate the key */

    silgy_random(linkkey, PASSWD_RESET_KEY_LEN);

    sprintf(sql_query, "INSERT INTO users_activations (linkkey,user_id,created,activated) VALUES ('%s',%ld,'%s','N')", linkkey, US.uid, G_dt);
    DBG("sql_query: %s", sql_query);

    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    /* send an email */

    char subject[256];
    char message[4096];
    char tmp[1024];
    char *p=message;

    sprintf(tmp, "Dear %s,\n\n", silgy_usr_name(NULL, NULL, NULL, UID));
    p = stpcpy(p, tmp);
    sprintf(tmp, "Welcome to %s! Your account requires activation. Please visit this URL to activate your account:\n\n", conn[ci].website);
    p = stpcpy(p, tmp);

#ifdef HTTPS
    if ( G_test )
        sprintf(tmp, "http://%s/activate_acc?k=%s\n\n", conn[ci].host, linkkey);
    else
        sprintf(tmp, "https://%s/activate_acc?k=%s\n\n", conn[ci].host, linkkey);
#else
    sprintf(tmp, "http://%s/activate_acc?k=%s\n\n", conn[ci].host, linkkey);
#endif  /* HTTPS */
    p = stpcpy(p, tmp);

    sprintf(tmp, "Please keep in mind that this link will only be valid for the next %d hours.\n\n", USER_ACTIVATION_HOURS);
    p = stpcpy(p, tmp);
    p = stpcpy(p, "If you did this by mistake or it wasn't you, you can safely ignore this email.\n\n");
#ifdef APP_CONTACT_EMAIL
    sprintf(tmp, "In case you needed any help, please contact us at %s.\n\n", APP_CONTACT_EMAIL);
    p = stpcpy(p, tmp);
#endif
    p = stpcpy(p, "Kind Regards\n");

    sprintf(tmp, "%s\n", conn[ci].website);
    p = stpcpy(p, tmp);

    sprintf(subject, "%s Account Activation", conn[ci].website);

    if ( !silgy_email(email, subject, message) )
        return ERR_INT_SERVER_ERROR;

    return OK;
}


/* --------------------------------------------------------------------------
   Verify activation key
-------------------------------------------------------------------------- */
static int silgy_usr_verify_activation_key(int ci, char *linkkey, long *uid)
{
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;
    QSVAL       esc_linkkey;

    DBG("silgy_usr_verify_activation_key");

    if ( strlen(linkkey) != PASSWD_RESET_KEY_LEN )
        return ERR_LINK_BROKEN;

    strcpy(esc_linkkey, silgy_sql_esc(linkkey));

    sprintf(sql_query, "SELECT user_id, created, activated FROM users_activations WHERE linkkey = BINARY '%s'", esc_linkkey);
    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    DBG("users_activations: %lu row(s) found", sql_records);

    if ( !sql_records )     /* no records with this key in users_activations -- link broken? */
    {
        mysql_free_result(result);
        return ERR_LINK_MAY_BE_EXPIRED;
    }

    sql_row = mysql_fetch_row(result);

    /* already activated? */

    if ( sql_row[2] && sql_row[2][0]=='Y' )
    {
        mysql_free_result(result);
        DBG("User already activated");
        return MSG_USER_ALREADY_ACTIVATED;
    }

    /* validate expiry time */

    if ( db2epoch(sql_row[1]) < G_now-3600*USER_ACTIVATION_HOURS )
    {
        WAR("Key created more than %d hours ago", USER_ACTIVATION_HOURS);
        mysql_free_result(result);
        return ERR_LINK_MAY_BE_EXPIRED;
    }

    /* otherwise everything's OK ----------------------------------------- */

    /* get the user id */

    *uid = atol(sql_row[0]);

    mysql_free_result(result);

    DBG("Key ok, uid = %ld", *uid);

    return OK;
}




/* ------------------------------------------------------------------------------------------------------------
    Public user functions
------------------------------------------------------------------------------------------------------------ */

/* --------------------------------------------------------------------------
   Log user in / explicit from Log In page
   Return OK or:
   ERR_INVALID_REQUEST
   ERR_INT_SERVER_ERROR
   ERR_INVALID_LOGIN
   and through do_login:
   ERR_SERVER_TOOBUSY
-------------------------------------------------------------------------- */
int silgy_usr_login(int ci)
{
    int         ret=OK;
    QSVAL       login;
    QSVAL       email;
    char        name[UNAME_LEN+1];
    char        phone[PHONE_LEN+1];
    char        about[ABOUT_LEN+1];
    short       auth_level;
    short       status;
    QSVAL       passwd;
    QSVAL       keep;
    char        ulogin[MAX_VALUE_LEN*2+1];
    char        sql_query[SQLBUF];
    char        p1[32], p2[32];
    char        str1[32], str2[32];
    int         ula_cnt;
    char        ula_time[32];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;
    long        uid;
    int         new_ula_cnt;
    long        visits;

    DBG("silgy_usr_login");

#ifdef SILGY_SVC

    ERR("silgy_usr_login() is not currently supported in silgy_svc. Move this call do silgy_app.");
    return ERR_INT_SERVER_ERROR;

#else

#ifdef USERSBYEMAIL

    if ( !QS_HTML_ESCAPE("email", email) || !QS_HTML_ESCAPE("passwd", passwd) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }
    stp_right(email);
    sprintf(sql_query, "SELECT id,login,email,name,phone,passwd1,passwd2,about,auth_level,status,ula_time,ula_cnt,visits FROM users WHERE email_u='%s'", upper(email));

#else    /* by login */

    if ( !QS_HTML_ESCAPE("login", login) || !QS_HTML_ESCAPE("passwd", passwd) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }
    stp_right(login);
    strcpy(ulogin, upper(login));
    sprintf(sql_query, "SELECT id,login,email,name,phone,passwd1,passwd2,about,auth_level,status,ula_time,ula_cnt,visits FROM users WHERE (login_u='%s' OR email_u='%s')", ulogin, ulogin);

#endif  /* USERSBYEMAIL */

    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    DBG("users: %lu record(s) found", sql_records);

    if ( 0 == sql_records )     /* no records */
    {
        mysql_free_result(result);
        return ERR_INVALID_LOGIN;   /* invalid user and/or password */
    }

    /* user name found */

    sql_row = mysql_fetch_row(result);

    uid = atol(sql_row[0]);
    strcpy(login, sql_row[1]?sql_row[1]:"");
    strcpy(email, sql_row[2]?sql_row[2]:"");
    strcpy(name, sql_row[3]?sql_row[3]:"");
    strcpy(phone, sql_row[4]?sql_row[4]:"");
    strcpy(p1, sql_row[5]);
    strcpy(p2, sql_row[6]);
    strcpy(about, sql_row[7]?sql_row[7]:"");
    auth_level = sql_row[8]?atoi(sql_row[8]):DEF_USER_AUTH_LEVEL;
    status = sql_row[9]?atoi(sql_row[9]):USER_STATUS_ACTIVE;
    strcpy(ula_time, sql_row[10]?sql_row[10]:"");
    ula_cnt = atoi(sql_row[11]);
    visits = atol(sql_row[12]);

    mysql_free_result(result);

    /* deleted? */

    if ( status == USER_STATUS_DELETED )
    {
        WAR("User deleted");
        return ERR_INVALID_LOGIN;
    }

    /* locked out? */

    if ( status == USER_STATUS_LOCKED )
    {
        WAR("User locked");
        return ERR_INVALID_LOGIN;
    }

    /* check ULA (Unsuccessful Login Attempts) info to prevent brute-force password attacks */

    if ( ula_cnt > MAX_ULA_BEFORE_FIRST_SLOW )
    {
        if ( ula_cnt > MAX_ULA_BEFORE_LOCK )
        {
            WAR("ula_cnt > MAX_ULA_BEFORE_LOCK (%d) => locking user account", MAX_ULA_BEFORE_LOCK);

            sprintf(sql_query, "UPDATE users SET status=%d WHERE id=%ld", USER_STATUS_LOCKED, uid);
            DBG("sql_query: %s", sql_query);
            if ( mysql_query(G_dbconn, sql_query) )
                ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));

            if ( email[0] )   /* notify account owner */
            {
                char subject[256];
                char message[4096];
                char tmp[1024];
                char *p=message;

                sprintf(tmp, "Dear %s,\n\n", silgy_usr_name(login, email, name, 0));
                p = stpcpy(p, tmp);
                sprintf(tmp, "Someone has tried to log in to your %s account unsuccessfully more than %d times. To protect it from brute-force attack your account has been locked.\n\n", conn[ci].website, MAX_ULA_BEFORE_LOCK);
                p = stpcpy(p, tmp);
#ifdef APP_CONTACT_EMAIL
                sprintf(tmp, "Please contact us at %s.\n\n", APP_CONTACT_EMAIL);
                p = stpcpy(p, tmp);
#endif
                p = stpcpy(p, "Kind Regards\n");

                sprintf(tmp, "%s\n", conn[ci].website);
                p = stpcpy(p, tmp);

                sprintf(subject, "%s account locked", conn[ci].website);

                silgy_email(email, subject, message);
            }

            return ERR_INVALID_LOGIN;
        }

        WAR("ula_cnt = %d", ula_cnt);

        time_t last_ula_epoch = db2epoch(ula_time);

        if ( ula_cnt <= MAX_ULA_BEFORE_SECOND_SLOW )
        {
            if ( last_ula_epoch > G_now-60 )    /* less than a minute => wait before the next attempt */
            {
                WAR("Trying again too soon (wait a minute)");
                return WAR_ULA_FIRST;
            }
        }
        else if ( ula_cnt <= MAX_ULA_BEFORE_THIRD_SLOW )
        {
            if ( last_ula_epoch > G_now-3600 )  /* less than an hour => wait before the next attempt */
            {
                WAR("Trying again too soon (wait an hour)");
                return WAR_ULA_SECOND;
            }
        }
        else    /* ula_cnt > MAX_ULA_BEFORE_THIRD_SLOW */
        {
            if ( last_ula_epoch > G_now-3600*23 )   /* less than 23 hours => wait before the next attempt */
            {
                WAR("Trying again too soon (wait a day)");
                return WAR_ULA_THIRD;
            }
        }
    }

    /* now check username/email and password pairs as they should be */

    get_hashes(str1, str2, login, email, passwd);

    /* are these as expected? */

    if ( 0 != strcmp(str1, p1) || (email[0] && 0 != strcmp(str2, p2)) )  /* passwd1, passwd2 */
    {
        DBG("Invalid password");
        new_ula_cnt = ula_cnt + 1;
        sprintf(sql_query, "UPDATE users SET ula_cnt=%d, ula_time='%s' WHERE id=%ld", new_ula_cnt, G_dt, uid);
        DBG("sql_query: %s", sql_query);
        if ( mysql_query(G_dbconn, sql_query) )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            return ERR_INT_SERVER_ERROR;
        }
        return ERR_INVALID_LOGIN;   /* invalid user and/or password */
    }

    DBG("Password OK");

    /* activated? */

    if ( status == USER_STATUS_INACTIVE )
    {
        WAR("User not activated");
        return ERR_NOT_ACTIVATED;
    }

    DBG("User activation status OK");

    /* successful login ------------------------------------------------------------ */

    if ( ula_cnt )   /* clear it */
    {
        DBG("Clearing ula_cnt");
        sprintf(sql_query, "UPDATE users SET ula_cnt=0 WHERE id=%ld", uid);
        DBG("sql_query: %s", sql_query);
        if ( mysql_query(G_dbconn, sql_query) )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            return ERR_INT_SERVER_ERROR;
        }
    }

    /* try to use anonymous sesid if present */

    if ( conn[ci].usi )
    {
        DBG("Using current session usi=%d, sesid [%s]", conn[ci].usi, US.sesid);
    }
    else    /* no session --> start a new one */
    {
        ret = eng_uses_start(ci, NULL);
        if ( ret != OK )
            return ret;
    }

    /* save new session to users_logins and set the cookie */

    DBG("Saving user session [%s] in users_logins...", US.sesid);

    char sanuagent[DB_UAGENT_LEN+1];
    sanitize_sql(sanuagent, conn[ci].uagent, DB_UAGENT_LEN);

    sprintf(sql_query, "INSERT INTO users_logins (sesid,uagent,ip,user_id,created,last_used) VALUES ('%s','%s','%s',%ld,'%s','%s')", US.sesid, sanuagent, conn[ci].ip, uid, G_dt, G_dt);
    DBG("sql_query: %s", sql_query);
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        if ( mysql_errno(G_dbconn) != 1062 )    /* duplicate entry */
            return ERR_INT_SERVER_ERROR;
    }

    DBG("User session saved OK");

    /* set cookie */

    strcpy(conn[ci].cookie_out_l, US.sesid);

    /* Keep me logged in -- set cookie expiry date */

    if ( QS_HTML_ESCAPE("keep", keep) && 0==strcmp(keep, "on") )
    {
        DBG("keep is ON");
        time_t sometimeahead = G_now + 3600*24*30;  /* 30 days */
        G_ptm = gmtime(&sometimeahead);
        strftime(conn[ci].cookie_out_l_exp, 32, "%a, %d %b %Y %T GMT", G_ptm);
//      DBG("conn[ci].cookie_out_l_exp: [%s]", conn[ci].cookie_out_l_exp);
        G_ptm = gmtime(&G_now);  /* make sure G_ptm is always up to date */
    }

    /* finish logging user in */

    return do_login(ci, uid, login, email, name, phone, about, auth_level, visits, status);

#endif  /* SILGY_SVC */
}


/* --------------------------------------------------------------------------
   Create user account
-------------------------------------------------------------------------- */
static int create_account(int ci, short auth_level, short status, bool current_session)
{
    int     ret=OK;
    QSVAL   login="";
    QSVAL   login_u;
    QSVAL   email="";
    QSVAL   email_u;
    QSVAL   name="";
    QSVAL   phone="";
    QSVAL   about="";
    QSVAL   passwd;
    QSVAL   rpasswd;
    QSVAL   message="";
    char    sql_query[SQLBUF];
    char    str1[32], str2[32];

    DBG("create_account");

    /* get the basics */

    if ( QS_HTML_ESCAPE("login", login) )
    {
        login[LOGIN_LEN] = EOS;
        stp_right(login);
        if ( current_session && conn[ci].usi ) strcpy(US.login, login);
    }

    if ( QS_HTML_ESCAPE("email", email) )
    {
        email[EMAIL_LEN] = EOS;
        stp_right(email);
        if ( current_session && conn[ci].usi ) strcpy(US.email, email);
    }

    /* basic verification */

#ifdef USERSBYEMAIL
    if ( !email[0] )    /* email empty */
    {
        ERR("Invalid request (email missing)");
        return ERR_EMAIL_EMPTY;
    }
#endif  /* USERSBYEMAIL */

    if ( !login[0] )    /* login empty */
    {
#ifdef USERSBYEMAIL
        strcpy(login, email);
#else
        ERR("Invalid request (login missing)");
        return ERR_INVALID_REQUEST;
#endif
    }

    /* regardless of authentication method */

    if ( G_usersRequireAccountActivation && !email[0] )
    {
        ERR("Invalid request (email missing)");
        return ERR_EMAIL_EMPTY;
    }

    if ( !QS_HTML_ESCAPE("passwd", passwd)
            || !QS_HTML_ESCAPE("rpasswd", rpasswd) )
    {
        ERR("Invalid request (passwd or rpasswd missing)");
        return ERR_INVALID_REQUEST;
    }

    /* optional */

    if ( QS_HTML_ESCAPE("name", name) )
    {
        name[UNAME_LEN] = EOS;
        stp_right(name);
        if ( current_session && conn[ci].usi ) strcpy(US.name, name);
    }

    if ( QS_HTML_ESCAPE("phone", phone) )
    {
        phone[PHONE_LEN] = EOS;
        stp_right(phone);
        if ( current_session && conn[ci].usi ) strcpy(US.phone, phone);
    }

    if ( QS_HTML_ESCAPE("about", about) )
    {
        about[ABOUT_LEN] = EOS;
        stp_right(about);
        if ( current_session && conn[ci].usi ) strcpy(US.about, about);
    }

    /* ----------------------------------------------------------------- */

    int plen = strlen(passwd);

    if ( QS_HTML_ESCAPE("message", message) && message[0] )
        return ERR_ROBOT;

#ifdef USERSBYEMAIL
        if ( !email[0] )                                /* email empty */
            return ERR_EMAIL_EMPTY;
        else if ( !valid_email(email) )                 /* invalid email format */
            return ERR_EMAIL_FORMAT;
#else
        if ( strlen(login) < MIN_USERNAME_LEN )         /* user name too short */
            return ERR_USERNAME_TOO_SHORT;
        else if ( !valid_username(login) )              /* only certain chars are allowed in user name */
            return ERR_USERNAME_CHARS;
        else if ( OK != (ret=user_exists(login)) )      /* user name taken */
            return ret;
        else if ( email[0] && !valid_email(email) )     /* invalid email format */
            return ERR_EMAIL_FORMAT_OR_EMPTY;
#endif  /* USERSBYEMAIL */

    if ( email[0] && OK != (ret=email_exists(email)) )  /* email in use */
        return ret;
    else if ( plen < MIN_PASSWORD_LEN )                 /* password too short */
        return ERR_PASSWORD_TOO_SHORT;
    else if ( 0 != strcmp(passwd, rpasswd) )            /* passwords differ */
        return ERR_PASSWORD_DIFFERENT;

    /* welcome! -- and generate password hashes ------------------------------------------------------- */

    get_hashes(str1, str2, login, email, passwd);

    strcpy(login_u, upper(login));
    strcpy(email_u, upper(email));

    sprintf(sql_query, "INSERT INTO users (id,login,login_u,email,email_u,name,phone,passwd1,passwd2,about,auth_level,status,created,visits,ula_cnt) VALUES (0,'%s','%s','%s','%s','%s','%s','%s','%s','%s',%hd,%hd,'%s',0,0)", login, login_u, email, email_u, name, phone, str1, str2, about, auth_level, status, G_dt);

    DBG("sql_query: INSERT INTO users (id,login,email,name,phone,...) VALUES (0,'%s','%s','%s','%s',...)", login, email, name, phone);

    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    G_new_user_id = mysql_insert_id(G_dbconn);

    if ( current_session )
        US.uid = G_new_user_id;

    if ( G_usersRequireAccountActivation )
    {
        if ( (ret=send_activation_link(ci, login, email)) != OK )
            return ret;
    }

#ifdef USERSBYEMAIL
    INF("User [%s] created", email);
#else
    INF("User [%s] created", login);
#endif

    return OK;

}


/* --------------------------------------------------------------------------
   Create user account (wrapper)
-------------------------------------------------------------------------- */
int silgy_usr_create_account(int ci)
{
    DBG("silgy_usr_create_account");

    short status;

    if ( G_usersRequireAccountActivation )
        status = USER_STATUS_INACTIVE;
    else
        status = USER_STATUS_ACTIVE;

    return create_account(ci, DEF_USER_AUTH_LEVEL, status, TRUE);
}


/* --------------------------------------------------------------------------
   Send an email about new account
-------------------------------------------------------------------------- */
static int new_account_notification(int ci, const char *login, const char *email, const char *name, const char *passwd)
{
    char subject[256];
    char message[4096];
    char tmp[1024];
    char *p=message;

    sprintf(tmp, "Dear %s,\n\n", silgy_usr_name(login, email, name, 0));
    p = stpcpy(p, tmp);
    sprintf(tmp, "An account has been created for you at %s.\n\n", conn[ci].website);
    p = stpcpy(p, tmp);

    p = stpcpy(p, "Please visit this address to log in:\n\n");

#ifdef HTTPS
    sprintf(tmp, "https://%s/%s\n\n", conn[ci].host, APP_LOGIN_URI);
#else
    sprintf(tmp, "http://%s/%s\n\n", conn[ci].host, APP_LOGIN_URI);
#endif
    p = stpcpy(p, tmp);

    sprintf(tmp, "Your password is %s and you will have to change it on your first login.\n\n", passwd[0]?passwd:"empty");
    p = stpcpy(p, tmp);

#ifdef APP_CONTACT_EMAIL
    sprintf(tmp, "In case you needed any help, please contact us at %s.\n\n", APP_CONTACT_EMAIL);
    p = stpcpy(p, tmp);
#endif
    p = stpcpy(p, "Kind Regards\n");

    sprintf(tmp, "%s\n", conn[ci].website);
    p = stpcpy(p, tmp);

    sprintf(subject, "Welcome to %s", conn[ci].website);

    if ( !silgy_email(email, subject, message) )
        return ERR_INT_SERVER_ERROR;

    return OK;
}


/* --------------------------------------------------------------------------
   Create user account
-------------------------------------------------------------------------- */
int silgy_usr_add_user(int ci, bool use_qs, const char *login, const char *email, const char *name, const char *passwd, const char *phone, const char *about, short auth_level)
{
    int   ret=OK;
    QSVAL password;

    DBG("silgy_usr_add_user");

    if ( use_qs )   /* use query string / POST payload */
    {
        if ( (ret=create_account(ci, auth_level, USER_STATUS_PASSWORD_CHANGE, FALSE)) != OK )
        {
            ERR("create_account failed");
            return ret;
        }

        QS_HTML_ESCAPE("passwd", password);
    }
    else    /* use function arguments */
    {
        QSVAL login_u;
        QSVAL email_u;
        char sql_query[SQLBUF];
        char str1[32], str2[32];

        /* --------------------------------------------------------------- */

        if ( passwd )   /* use the one supplied */
            strcpy(password, passwd);
        else    /* generate */
            silgy_random(password, MIN_PASSWORD_LEN);

        /* --------------------------------------------------------------- */

#ifdef USERSBYEMAIL
        if ( !email[0] )                                /* email empty */
            return ERR_EMAIL_EMPTY;
        else if ( !valid_email(email) )                 /* invalid email format */
            return ERR_EMAIL_FORMAT;
        else if ( OK != (ret=email_exists(email)) )     /* email not unique */
            return ret;
#else
        if ( strlen(login) < MIN_USERNAME_LEN )         /* user name too short */
            return ERR_USERNAME_TOO_SHORT;
        else if ( !valid_username(login) )              /* only certain chars are allowed in user name */
            return ERR_USERNAME_CHARS;
        else if ( OK != (ret=user_exists(login)) )      /* user name taken */
            return ret;
        else if ( email[0] && !valid_email(email) )     /* invalid email format */
            return ERR_EMAIL_FORMAT_OR_EMPTY;
#endif  /* USERSBYEMAIL */

        /* --------------------------------------------------------------- */

        get_hashes(str1, str2, login, email, password);

        /* --------------------------------------------------------------- */

        strcpy(login_u, upper(login));
        strcpy(email_u, upper(email));

        sprintf(sql_query, "INSERT INTO users (id,login,login_u,email,email_u,name,phone,passwd1,passwd2,about,auth_level,status,created,visits,ula_cnt) VALUES (0,'%s','%s','%s','%s','%s','%s','%s','%s','%s',%hd,%hd,'%s',0,0)", login, login_u, email, email_u, name?name:"", phone?phone:"", str1, str2, about?about:"", auth_level, USER_STATUS_PASSWORD_CHANGE, G_dt);

        DBG("sql_query: INSERT INTO users (id,login,email,name,phone,...) VALUES (0,'%s','%s','%s','%s',...)", login, email, name?name:"", phone?phone:"");

        if ( mysql_query(G_dbconn, sql_query) )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            return ERR_INT_SERVER_ERROR;
        }

        G_new_user_id = mysql_insert_id(G_dbconn);

        /* --------------------------------------------------------------- */

#ifdef USERSBYEMAIL
        INF("User [%s] created", email);
#else
        INF("User [%s] created", login);
#endif
    }

#ifndef DONT_NOTIFY_NEW_USER
    QSVAL email_;
    if ( use_qs )
        QS("email", email_);
    else
        strcpy(email_, email);

    if ( email_[0] )
        new_account_notification(ci, login, email_, name, password);
#endif

    return ret;
}


/* --------------------------------------------------------------------------
   Save user message
-------------------------------------------------------------------------- */
int silgy_usr_send_message(int ci)
{
    char message[MAX_LONG_URI_VAL_LEN+1];

    DBG("silgy_usr_send_message");

    if ( !get_qs_param_long(ci, "msg_box", message) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }

    QSVAL   email;
static char sanmessage[MAX_LONG_URI_VAL_LEN*2];
static char sql_query[MAX_LONG_URI_VAL_LEN*2];

    if ( QS_HTML_ESCAPE("email", email) )
        stp_right(email);

    sprintf(sanmessage, "From %s\n\n", conn[ci].ip);
    strcpy(sanmessage+strlen(sanmessage), silgy_html_esc(message));

    /* remember user details in case of error or warning to correct */

    if ( conn[ci].usi )
        strcpy(US.email_tmp, email);

    sprintf(sql_query, "INSERT INTO users_messages (user_id,msg_id,email,message,created) VALUES (%ld,%ld,'%s','%s','%s')", US.uid, get_max(ci, "messages")+1, email, sanmessage, G_dt);
    DBG("sql_query: INSERT INTO users_messages (user_id,msg_id,email,...) VALUES (%ld,get_max(),'%s',...)", US.uid, email);

    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    /* send an email to admin */

#ifdef APP_CONTACT_EMAIL
    silgy_email(APP_CONTACT_EMAIL, "New message!", sanmessage);
#endif

    return OK;
}


/* --------------------------------------------------------------------------
   Save changes to user account
-------------------------------------------------------------------------- */
int silgy_usr_save_account(int ci)
{
    int         ret=OK;
    QSVAL       login;
    QSVAL       email;
    QSVAL       name;
    QSVAL       phone;
    QSVAL       about;
    QSVAL       passwd;
    QSVAL       rpasswd;
    QSVAL       opasswd;
    QSVAL       uemail_old;
    QSVAL       uemail_new;
    QSVAL       strdelete;
    QSVAL       strdelconf;
    QSVAL       save;
    int         plen;
    char        sql_query[SQLBUF];
    char        str1[32], str2[32];
    MYSQL_RES   *result;
    unsigned long sql_records;
    MYSQL_ROW   sql_row;

    DBG("silgy_usr_save_account");

    if ( !QS_HTML_ESCAPE("opasswd", opasswd)
#ifndef USERSBYEMAIL
            || !QS_HTML_ESCAPE("login", login)
#endif
            || !QS_HTML_ESCAPE("email", email)
            || !QS_HTML_ESCAPE("passwd", passwd)
            || !QS_HTML_ESCAPE("rpasswd", rpasswd) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }

#ifdef USERSBYEMAIL
    if ( QS_HTML_ESCAPE("login", login) )     /* try to get login anyway */
        stp_right(login);
#endif

    stp_right(email);   /* always present but can be empty */

    if ( QS_HTML_ESCAPE("name", name) ) /* optional */
        stp_right(name);

    if ( QS_HTML_ESCAPE("phone", phone) ) /* optional */
        stp_right(phone);

    if ( QS_HTML_ESCAPE("about", about) ) /* optional */
        stp_right(about);

    /* remember form fields */
    /* US.email contains old email */

    strncpy(US.login_tmp, login, LOGIN_LEN);
    US.login_tmp[LOGIN_LEN] = EOS;
    strncpy(US.email_tmp, email, EMAIL_LEN);
    US.email_tmp[EMAIL_LEN] = EOS;
    strncpy(US.name_tmp, name, UNAME_LEN);
    US.name_tmp[UNAME_LEN] = EOS;
    strncpy(US.phone_tmp, phone, PHONE_LEN);
    US.phone_tmp[PHONE_LEN] = EOS;
    strncpy(US.about_tmp, about, ABOUT_LEN);
    US.about_tmp[ABOUT_LEN] = EOS;

    DBG("login_tmp: [%s]", US.login_tmp);
    DBG("email_tmp: [%s]", US.email_tmp);
    DBG(" name_tmp: [%s]", US.name_tmp);
    DBG("phone_tmp: [%s]", US.phone_tmp);
    DBG("about_tmp: [%s]", US.about_tmp);

    /* basic validation */

    plen = strlen(passwd);

#ifdef USERSBYEMAIL
    if ( !email[0] )
        return ERR_EMAIL_EMPTY;
    else if ( !valid_email(email) )
        return ERR_EMAIL_FORMAT;
#else
    if ( email[0] && !valid_email(email) )
        return ERR_EMAIL_FORMAT_OR_EMPTY;
#endif  /* USERSBYEMAIL */
    else if ( plen && plen < MIN_PASSWORD_LEN )
        return ERR_PASSWORD_TOO_SHORT;
    else if ( plen && 0 != strcmp(passwd, rpasswd) )
        return ERR_PASSWORD_DIFFERENT;

    /* if email change, check if the new one has not already been registered */

    strcpy(uemail_old, upper(US.email));
    strcpy(uemail_new, upper(email));

    if ( uemail_new[0] && strcmp(uemail_old, uemail_new) != 0 && (ret=silgy_usr_email_registered(ci)) != OK )
        return ret;

    /* verify existing password against login/email/passwd1 */

#ifdef USERSBYEMAIL
    doit(str1, str2, email, email, opasswd);
    sprintf(sql_query, "SELECT passwd1 FROM users WHERE email_u='%s'", upper(email));
#else
    doit(str1, str2, login, login, opasswd);
    sprintf(sql_query, "SELECT passwd1 FROM users WHERE login_u='%s'", upper(login));
#endif  /* USERSBYEMAIL */
    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    if ( 0 == sql_records )
    {
        ERR("Weird: no such user");
        mysql_free_result(result);
        return ERR_INT_SERVER_ERROR;
    }

    sql_row = mysql_fetch_row(result);

    if ( 0 != strcmp(str1, sql_row[0]) )
    {
        ERR("Invalid old password");
        mysql_free_result(result);
        return ERR_OLD_PASSWORD;
    }

    mysql_free_result(result);

    /* Old password OK ---------------------------------------- */

    DBG("Old password OK");

    if ( QS_HTML_ESCAPE("delete", strdelete) && 0==strcmp(strdelete, "on") )    /* delete user account */
    {
        if ( !QS_HTML_ESCAPE("delconf", strdelconf) || 0 != strcmp(strdelconf, "1") )
            return WAR_BEFORE_DELETE;
        else
        {
            sprintf(sql_query, "UPDATE users SET status=%d WHERE id=%ld", USER_STATUS_DELETED, US.uid);
            DBG("sql_query: %s", sql_query);
            if ( mysql_query(G_dbconn, sql_query) )
            {
                ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
                return ERR_INT_SERVER_ERROR;
            }

            downgrade_uses(conn[ci].usi, ci, TRUE);   /* log user out */

            return MSG_ACCOUNT_DELETED;
        }
    }

    /* anything else than deleting account -- changing email and/or name and/or password */

    get_hashes(str1, str2, login, email, plen?passwd:opasswd);

    sprintf(sql_query, "UPDATE users SET login='%s', email='%s', name='%s', phone='%s', passwd1='%s', passwd2='%s', about='%s' WHERE id=%ld", login, email, name, phone, str1, str2, about, US.uid);
    DBG("sql_query: UPDATE users SET login='%s', email='%s', name='%s', phone='%s',... WHERE id=%ld", login, email, name, phone, US.uid);

    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    DBG("Updating login, email, name, phone & about in user session");

    strcpy(US.login, US.login_tmp);
    strcpy(US.email, US.email_tmp);
    strcpy(US.name, US.name_tmp);
    strcpy(US.phone, US.phone_tmp);
    strcpy(US.about, US.about_tmp);

    /* On password change invalidate all existing sessions except of the current one */

    if ( passwd[0] && 0!=strcmp(passwd, opasswd) )
    {
        DBG("Password change => invalidating all other session tokens");

        sprintf(sql_query, "DELETE FROM users_logins WHERE user_id = %ld AND sesid != BINARY '%s'", UID, US.sesid);
        DBG("sql_query: %s", sql_query);
        if ( mysql_query(G_dbconn, sql_query) )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            return ERR_INT_SERVER_ERROR;
        }

        /* downgrade all currently active sessions belonging to this user */
        /* except of the current one */

        downgrade_uses_by_uid(UID, ci);
    }

    return OK;
}


/* --------------------------------------------------------------------------
   Email taken?
-------------------------------------------------------------------------- */
int silgy_usr_email_registered(int ci)
{
    QSVAL   email;

    DBG("silgy_usr_email_registered");

    if ( !QS_HTML_ESCAPE("email", email) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }

    stp_right(email);

    return email_exists(email);
}


/* --------------------------------------------------------------------------
   Return the best version of the user name for "Dear ..."
-------------------------------------------------------------------------- */
char *silgy_usr_name(const char *login, const char *email, const char *name, long uid)
{
static char dest[128];

    DBG("silgy_usr_name");

    if ( name && name[0] )
    {
        strcpy(dest, name);
    }
    else if ( login && login[0] )
    {
        strcpy(dest, login);
    }
    else if ( email && email[0] )
    {
        int i=0;
        while ( email[i]!='@' && email[i] && i<100 ) dest[i++] = email[i];
        dest[i] = EOS;
    }
    else if ( uid )
    {
        char            sql_query[SQLBUF];
        MYSQL_RES       *result;
        unsigned long   sql_records;
        MYSQL_ROW       sql_row;

        sprintf(sql_query, "SELECT login, email, name FROM users WHERE id=%ld", uid);
        DBG("sql_query: %s", sql_query);
        mysql_query(G_dbconn, sql_query);
        result = mysql_store_result(G_dbconn);
        if ( !result )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            strcpy(dest, "User");
        }
        else    /* OK */
        {
            sql_records = mysql_num_rows(result);

            if ( 0 == sql_records )
            {
                mysql_free_result(result);
                strcpy(dest, "User");
            }
            else
            {
                char db_login[128];
                char db_email[128];
                char db_name[128];

                sql_row = mysql_fetch_row(result);

                strcpy(db_login, sql_row[0]?sql_row[0]:"");
                strcpy(db_email, sql_row[1]?sql_row[1]:"");
                strcpy(db_name, sql_row[2]?sql_row[2]:"");

                mysql_free_result(result);

                /* we can't use recursion with char * function... */
                if ( db_name && db_name[0] )
                    strcpy(dest, db_name);
                else if ( db_login && db_login[0] )
                    strcpy(dest, db_login);
                else if ( db_email && db_email[0] )
                {
                    int i=0;
                    while ( db_email[i]!='@' && db_email[i] && i<100 ) dest[i++] = db_email[i];
                    dest[i] = EOS;
                }
                else
                    strcpy(dest, "User");
            }
        }
    }
    else
    {
        strcpy(dest, "User");
    }

    return dest;
}


/* --------------------------------------------------------------------------
   Send an email with password reset link
-------------------------------------------------------------------------- */
int silgy_usr_send_passwd_reset_email(int ci)
{
    QSVAL       email;
    QSVAL       submit;
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    unsigned long sql_records;
    MYSQL_ROW   sql_row;

    DBG("silgy_usr_send_passwd_reset_email");

    if ( !QS_HTML_ESCAPE("email", email) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }

    stp_right(email);

    if ( !valid_email(email) )      /* invalid email format */
        return ERR_EMAIL_FORMAT;

    sprintf(sql_query, "SELECT id, login, name, status FROM users WHERE email_u='%s'", upper(email));
    DBG("sql_query: %s", sql_query);
    mysql_query(G_dbconn, sql_query);
    result = mysql_store_result(G_dbconn);
    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    DBG("users: %lu record(s) found", sql_records);

    if ( 0 == sql_records )
    {
        mysql_free_result(result);
        WAR("Password reset link requested for non-existent [%s]", email);
        return OK;
    }

    /* -------------------------------------------------------------------------- */

    sql_row = mysql_fetch_row(result);

    if ( atoi(sql_row[3]) == USER_STATUS_DELETED )
    {
        mysql_free_result(result);
        WAR("Password reset link requested for [%s] but user is deleted", email);
        return OK;
    }

    /* -------------------------------------------------------------------------- */

    long uid;
    char login[128];
    char name[128];

    uid = atol(sql_row[0]);
    strcpy(login, sql_row[1]?sql_row[1]:"");
    strcpy(name, sql_row[2]?sql_row[2]:"");

    mysql_free_result(result);

    /* -------------------------------------------------------------------------- */
    /* generate a key */

    char linkkey[PASSWD_RESET_KEY_LEN+1];

    silgy_random(linkkey, PASSWD_RESET_KEY_LEN);

    sprintf(sql_query, "INSERT INTO users_p_resets (linkkey,user_id,created,tries) VALUES ('%s',%ld,'%s',0)", linkkey, uid, G_dt);
    DBG("sql_query: %s", sql_query);

    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    /* -------------------------------------------------------------------------- */
    /* send an email */

    char subject[256];
    char message[4096];
    char tmp[1024];
    char *p=message;

    sprintf(tmp, "Dear %s,\n\n", silgy_usr_name(login, email, name, 0));
    p = stpcpy(p, tmp);
    sprintf(tmp, "You have requested to have your password reset for your account at %s. Please visit this URL to reset your password:\n\n", conn[ci].website);
    p = stpcpy(p, tmp);

#ifdef HTTPS
    if ( G_test )
        sprintf(tmp, "http://%s/preset?k=%s\n\n", conn[ci].host, linkkey);
    else
        sprintf(tmp, "https://%s/preset?k=%s\n\n", conn[ci].host, linkkey);
#else
    sprintf(tmp, "http://%s/preset?k=%s\n\n", conn[ci].host, linkkey);
#endif  /* HTTPS */

    p = stpcpy(p, tmp);

    p = stpcpy(p, "Please keep in mind that this link will only be valid for the next 24 hours.\n\n");
    p = stpcpy(p, "If you did this by mistake or it wasn't you, you can safely ignore this email.\n\n");
#ifdef APP_CONTACT_EMAIL
    sprintf(tmp, "In case you needed any help, please contact us at %s.\n\n", APP_CONTACT_EMAIL);
    p = stpcpy(p, tmp);
#endif
    p = stpcpy(p, "Kind Regards\n");

    sprintf(tmp, "%s\n", conn[ci].website);
    p = stpcpy(p, tmp);

    sprintf(subject, "%s Password Reset", conn[ci].website);

    if ( !silgy_email(email, subject, message) )
        return ERR_INT_SERVER_ERROR;

    /* -------------------------------------------------------------------------- */

    INF("Password reset link requested for [%s]", email);

    return OK;
}


/* --------------------------------------------------------------------------
   Verify the link key for password reset
-------------------------------------------------------------------------- */
int silgy_usr_verify_passwd_reset_key(int ci, char *linkkey, long *uid)
{
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;
    char        esc_linkkey[256];
    int         tries;

    DBG("silgy_usr_verify_passwd_reset_key");

    if ( strlen(linkkey) != PASSWD_RESET_KEY_LEN )
        return ERR_LINK_BROKEN;

    strcpy(esc_linkkey, silgy_sql_esc(linkkey));

    sprintf(sql_query, "SELECT user_id, created, tries FROM users_p_resets WHERE linkkey = BINARY '%s'", esc_linkkey);
    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    DBG("users_p_resets: %lu row(s) found", sql_records);

    if ( !sql_records )     /* no records with this key in users_p_resets -- link broken? */
    {
        mysql_free_result(result);
        return ERR_LINK_MAY_BE_EXPIRED;
    }

    sql_row = mysql_fetch_row(result);

    /* validate expiry time */

    if ( db2epoch(sql_row[1]) < G_now-3600*24 )  /* older than 24 hours? */
    {
        WAR("Key created more than 24 hours ago");
        mysql_free_result(result);
        return ERR_LINK_MAY_BE_EXPIRED;
    }

    /* validate tries */

    tries = atoi(sql_row[2]);

    if ( tries > 12 )
    {
        WAR("Key tried more than 12 times");
        mysql_free_result(result);
        return ERR_LINK_TOO_MANY_TRIES;
    }

    /* otherwise everything's OK ----------------------------------------- */

    /* get the user id */

    *uid = atol(sql_row[0]);

    mysql_free_result(result);

    DBG("Key ok, uid = %ld", *uid);

    /* update tries counter */

    sprintf(sql_query, "UPDATE users_p_resets SET tries=%d WHERE linkkey = BINARY '%s'", tries+1, esc_linkkey);
    DBG("sql_query: %s", sql_query);
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    return OK;
}


/* --------------------------------------------------------------------------
   Activate user account
-------------------------------------------------------------------------- */
int silgy_usr_activate(int ci)
{
    int         ret;
    QSVAL       linkkey;
    long        uid;
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;

    DBG("silgy_usr_activate");

    if ( !QS_HTML_ESCAPE("k", linkkey) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }

    /* verify the key */

	if ( (ret=silgy_usr_verify_activation_key(ci, linkkey, &uid)) != OK )
		return ret;

    /* everything's OK -- activate user -------------------- */

    sprintf(sql_query, "UPDATE users SET status=%d WHERE id=%ld", USER_STATUS_ACTIVE, uid);
    DBG("sql_query: %s", sql_query);
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    /* remove activation link */

//    sprintf(sql_query, "DELETE FROM users_activations WHERE linkkey = BINARY '%s'", linkkey);
    sprintf(sql_query, "UPDATE users_activations SET activated='Y' WHERE linkkey = BINARY '%s'", linkkey);
    DBG("sql_query: %s", sql_query);
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
//        return ERR_INT_SERVER_ERROR;  ignore it
    }

    return OK;
}


/* --------------------------------------------------------------------------
   Change logged in user password and change its status to USER_STATUS_ACTIVE
   Used in case user is forced to change their password
   (status == USER_STATUS_PASSWORD_CHANGE)
-------------------------------------------------------------------------- */
int silgy_usr_change_password(int ci)
{
    int         ret;
    QSVAL       opasswd;
    QSVAL       passwd;
    QSVAL       rpasswd;
    QSVAL       submit;
    long        uid;
    char        sql_query[SQLBUF];
    char        str1[32], str2[32];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;

    DBG("silgy_usr_change_password");

    if ( !QS_HTML_ESCAPE("opasswd", opasswd)
            || !QS_HTML_ESCAPE("passwd", passwd)
            || !QS_HTML_ESCAPE("rpasswd", rpasswd) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }

    /* verify existing password against login/email/passwd1 */

#ifdef USERSBYEMAIL
    doit(str1, str2, US.email, US.email, opasswd);
    sprintf(sql_query, "SELECT passwd1 FROM users WHERE email_u='%s'", upper(US.email));
#else
    doit(str1, str2, US.login, US.login, opasswd);
    sprintf(sql_query, "SELECT passwd1 FROM users WHERE login_u='%s'", upper(US.login));
#endif  /* USERSBYEMAIL */
    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    if ( 0 == sql_records )
    {
        ERR("Weird: no such user");
        mysql_free_result(result);
        return ERR_INT_SERVER_ERROR;
    }

    sql_row = mysql_fetch_row(result);

    if ( 0 != strcmp(str1, sql_row[0]) )
    {
        ERR("Invalid old password");
        mysql_free_result(result);
        return ERR_OLD_PASSWORD;
    }

    mysql_free_result(result);

    /* Old password OK ---------------------------------------- */

    DBG("Old password OK");

    /* new password validation */

    int plen = strlen(passwd);

    if ( plen < MIN_PASSWORD_LEN )       /* password too short */
        return ERR_PASSWORD_TOO_SHORT;
    else if ( 0 != strcmp(passwd, rpasswd) )   /* passwords differ */
        return ERR_PASSWORD_DIFFERENT;

    DBG("New password OK");

    /* everything's OK -- update password -------------------------------- */

    get_hashes(str1, str2, US.login, US.email, passwd);

    mysql_free_result(result);

    DBG("Updating users...");

    sprintf(sql_query, "UPDATE users SET passwd1='%s', passwd2='%s', status=%hd WHERE id=%ld", str1, str2, USER_STATUS_ACTIVE, UID);
    DBG("sql_query: UPDATE users SET passwd1=...");
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    return OK;
}


/* --------------------------------------------------------------------------
   Save new password after reset
-------------------------------------------------------------------------- */
int silgy_usr_reset_password(int ci)
{
    int         ret;
    QSVAL       email;
    QSVAL       linkkey;
    QSVAL       passwd;
    QSVAL       rpasswd;
    QSVAL       submit;
    long        uid;
    char        sql_query[SQLBUF];
    char        str1[32], str2[32];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;

    DBG("silgy_usr_reset_password");

    if ( !QS_HTML_ESCAPE("email", email)
            || !QS_HTML_ESCAPE("k", linkkey)
            || !QS_HTML_ESCAPE("passwd", passwd)
            || !QS_HTML_ESCAPE("rpasswd", rpasswd) )
    {
        WAR("Invalid request (URI val missing?)");
        return ERR_INVALID_REQUEST;
    }

    stp_right(email);

    /* remember form fields */

    if ( conn[ci].usi )
        strcpy(US.email_tmp, email);

    /* general validation */

    int plen = strlen(passwd);

    if ( !valid_email(email) )
        return ERR_EMAIL_FORMAT;
    else if ( plen < MIN_PASSWORD_LEN )       /* password too short */
        return ERR_PASSWORD_TOO_SHORT;
    else if ( 0 != strcmp(passwd, rpasswd) )    /* passwords differ */
        return ERR_PASSWORD_DIFFERENT;

    /* verify the key */

	if ( (ret=silgy_usr_verify_passwd_reset_key(ci, linkkey, &uid)) != OK )
		return ret;

    /* verify that emails match each other */

    sprintf(sql_query, "SELECT login, email FROM users WHERE id=%ld", uid);
    DBG("sql_query: %s", sql_query);
    mysql_query(G_dbconn, sql_query);
    result = mysql_store_result(G_dbconn);
    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    DBG("users: %lu record(s) found", sql_records);

    if ( 0 == sql_records )     /* password reset link expired or invalid email */
    {
        mysql_free_result(result);
        return ERR_LINK_EXPIRED;
    }

    sql_row = mysql_fetch_row(result);

    if ( 0 != strcmp(sql_row[1], email) )   /* emails different */
    {
        mysql_free_result(result);
        return ERR_LINK_EXPIRED;    /* password reset link expired or invalid email */
    }


    /* everything's OK -- update password -------------------------------- */

    get_hashes(str1, str2, sql_row[0], email, passwd);

    mysql_free_result(result);

    DBG("Updating users...");

    sprintf(sql_query, "UPDATE users SET passwd1='%s', passwd2='%s' WHERE id=%ld", str1, str2, uid);
    DBG("sql_query: UPDATE users SET passwd1=...");
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    /* Invalidate all existing sessions */

    DBG("Invalidating all session tokens...");

    sprintf(sql_query, "DELETE FROM users_logins WHERE user_id = %ld", uid);
    DBG("sql_query: %s", sql_query);
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    /* downgrade all currently active sessions belonging to this user */

    downgrade_uses_by_uid(uid, -1);

    /* remove all password reset keys */

    DBG("Deleting from users_p_resets...");

    sprintf(sql_query, "DELETE FROM users_p_resets WHERE user_id=%ld", uid);
    DBG("sql_query: %s", sql_query);
    if ( mysql_query(G_dbconn, sql_query) )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
//        return ERR_INT_SERVER_ERROR;  ignore it
    }

    return OK;
}


/* --------------------------------------------------------------------------
   Log user out
-------------------------------------------------------------------------- */
void silgy_usr_logout(int ci)
{
    DBG("silgy_usr_logout");
    downgrade_uses(conn[ci].usi, ci, TRUE);
}


/* --------------------------------------------------------------------------
   doit wrapper
-------------------------------------------------------------------------- */
static void get_hashes(char *result1, char *result2, const char *login, const char *email, const char *passwd)
{
#ifdef USERSBYLOGIN
    doit(result1, result2, login, email[0]?email:STR_005, passwd);
#else
    doit(result1, result2, email, email, passwd);
#endif
}


/* --------------------------------------------------------------------------
   Generate password hashes
-------------------------------------------------------------------------- */
static void doit(char *result1, char *result2, const char *login, const char *email, const char *src)
{
    char    tmp[4096];
    unsigned char digest[SHA1_DIGEST_SIZE];
    int     i, j=0;

    sprintf(tmp, "%s%s%s%s", STR_001, upper(login), STR_002, src); /* login */
    libSHA1((unsigned char*)tmp, strlen(tmp), digest);
    Base64encode(tmp, (char*)digest, SHA1_DIGEST_SIZE);
    for ( i=0; tmp[i] != EOS; ++i ) /* drop non-alphanumeric characters */
    {
        if ( isalnum(tmp[i]) )
            result1[j++] = tmp[i];
    }
    result1[j] = EOS;

    j = 0;

    sprintf(tmp, "%s%s%s%s", STR_003, upper(email), STR_004, src); /* email */
    libSHA1((unsigned char*)tmp, strlen(tmp), digest);
    Base64encode(tmp, (char*)digest, SHA1_DIGEST_SIZE);
    for ( i=0; tmp[i] != EOS; ++i ) /* drop non-alphanumeric characters */
    {
        if ( isalnum(tmp[i]) )
            result2[j++] = tmp[i];
    }
    result2[j] = EOS;
}


/* --------------------------------------------------------------------------
   Save user string setting
-------------------------------------------------------------------------- */
int silgy_usr_set_str(int ci, const char *us_key, const char *us_val)
{
    int  ret=OK;
    char sql_query[SQLBUF];

    ret = silgy_usr_get_str(ci, us_key, NULL);

    if ( ret == ERR_NOT_FOUND )
    {
        sprintf(sql_query, "INSERT INTO users_settings (user_id,us_key,us_val) VALUES (%ld,'%s','%s')", US.uid, us_key, us_val);

        DBG("sql_query: %s", sql_query);

        if ( mysql_query(G_dbconn, sql_query) )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            return ERR_INT_SERVER_ERROR;
        }
    }
    else if ( ret != OK )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }
    else
    {
        sprintf(sql_query, "UPDATE users_settings SET us_val='%s' WHERE user_id=%ld AND us_key='%s'", us_val, US.uid, us_key);

        DBG("sql_query: %s", sql_query);

        if ( mysql_query(G_dbconn, sql_query) )
        {
            ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
            return ERR_INT_SERVER_ERROR;
        }
    }

    return OK;
}


/* --------------------------------------------------------------------------
   Read user string setting
-------------------------------------------------------------------------- */
int silgy_usr_get_str(int ci, const char *us_key, char *us_val)
{
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    unsigned long sql_records;

    sprintf(sql_query, "SELECT us_val FROM users_settings WHERE user_id=%ld AND us_key='%s'", US.uid, us_key);

    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_records = mysql_num_rows(result);

    DBG("users_settings: %lu record(s) found", sql_records);

    if ( 0 == sql_records )
    {
        mysql_free_result(result);
        return ERR_NOT_FOUND;
    }

    sql_row = mysql_fetch_row(result);

    if ( us_val )
        strcpy(us_val, sql_row[0]);

    mysql_free_result(result);

    return OK;
}


/* --------------------------------------------------------------------------
   Save user number setting
-------------------------------------------------------------------------- */
int silgy_usr_set_int(int ci, const char *us_key, long us_val)
{
    char val[64];

    sprintf(val, "%ld", us_val);
    return silgy_usr_set_str(ci, us_key, val);
}


/* --------------------------------------------------------------------------
   Read user number setting
-------------------------------------------------------------------------- */
int silgy_usr_get_int(int ci, const char *us_key, long *us_val)
{
    int  ret;
    char val[64];

    if ( (ret=silgy_usr_get_str(ci, us_key, val)) == OK )
        *us_val = atol(val);

    return ret;
}


/* --------------------------------------------------------------------------
   Get MAX(msg_id) from users_messages for current user
-------------------------------------------------------------------------- */
static long get_max(int ci, const char *table)
{
    char        sql_query[SQLBUF];
    MYSQL_RES   *result;
    MYSQL_ROW   sql_row;
    long        max=0;

    /* US.uid = 0 for anonymous session */

    if ( 0==strcmp(table, "messages") )
        sprintf(sql_query, "SELECT MAX(msg_id) FROM users_messages WHERE user_id=%ld", US.uid);
    else
        return 0;

    DBG("sql_query: %s", sql_query);

    mysql_query(G_dbconn, sql_query);

    result = mysql_store_result(G_dbconn);

    if ( !result )
    {
        ERR("Error %u: %s", mysql_errno(G_dbconn), mysql_error(G_dbconn));
        return ERR_INT_SERVER_ERROR;
    }

    sql_row = mysql_fetch_row(result);

    if ( sql_row[0] != NULL )
        max = atol(sql_row[0]);

    mysql_free_result(result);

    DBG("get_max for uid=%ld  max = %ld", US.uid, max);

    return max;
}

#endif  /* USERS */
