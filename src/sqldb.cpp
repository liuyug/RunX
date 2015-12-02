
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include <sqlite3.h>

#include "global.h"
#include "sqldb.h"
#include "utils_win32.h"


PVOID sqldb_init(const TCHAR * dbname)
{
    sqlite3 *db;
    if(_sqlite3_open(dbname,&db)!=SQLITE_OK){
        debug_errmsg(_T("sqlite3_open"),_sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

int sqldb_exec(PVOID vdb,const TCHAR * sql,sqldb_callback *callback,int cols,TCHAR record[][MAX_BUFFER],PVOID param)
{
    int ret=0;
    sqlite3 * db = (sqlite3 *)vdb;
    sqlite3_stmt *sql_statement;
    int sql_len;
#if defined(_UNICODE) || defined(UNICODE)
    // UTF-16
    sql_len=_tcslen(sql)*2;
#else
    sql_len=_tcslen(sql);
#endif
    debug_output(_T("%d: %s"),sql_len, sql);
    if(_sqlite3_prepare_v2(db,sql,sql_len,&sql_statement,0)!=SQLITE_OK){
        debug_errmsg(_T("sqlite3_prepare_v2"),_sqlite3_errmsg(db));
        ret=1;
        goto sqldb_exec_exit;
    }
    int sql_ret;
    if (record != NULL) record[0][0] = _T('\0');
    while((sql_ret=_sqlite3_step(sql_statement))==SQLITE_ROW){
        if(_sqlite3_column_count(sql_statement)<=cols){
            for(int i=0;i<_sqlite3_column_count(sql_statement);i++){
                _tcscpy_s(record[i],(TCHAR*)_sqlite3_column_text(sql_statement,i));
                debug_log(record[i]);
            }
            if(callback!=NULL)
                callback(_sqlite3_column_count(sql_statement),record,param);
        }
    }
    if(sql_ret==SQLITE_DONE){
        ret=0;
        goto sqldb_exec_exit;
    }else if(sql_ret==SQLITE_ERROR){
        debug_errmsg(_T("sqlite3_step"),_sqlite3_errmsg(db));
        ret=2;
        goto sqldb_exec_exit;
    }else if(sql_ret==SQLITE_MISUSE ){
        debug_errmsg(_T("sqlite3_step"),_sqlite3_errmsg(db));
        ret=2;
        goto sqldb_exec_exit;
    }else if(sql_ret==SQLITE_BUSY){
            // my app don't catch it
        debug_errmsg(_T("sqlite3_step"),_sqlite3_errmsg(db));
        ret=2;
        goto sqldb_exec_exit;
    }else {
        debug_errmsg(_T("sqlite3_step"),_sqlite3_errmsg(db));
        ret = 3;
    }
sqldb_exec_exit:
    _sqlite3_finalize(sql_statement);
    return ret;
}

void sqldb_close(PVOID vdb)
{
    sqlite3 * db = (sqlite3 *)vdb;
    _sqlite3_close(db);
}
