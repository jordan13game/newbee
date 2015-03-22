
#ifndef  _LOG_H_INC
#define  _LOG_H_INC

#include	"log_impl.h"
#include	"singleton.h"
#include	<string>
#include	<vector>

#define	ENABLE_FATAL			/*  */
#define	ENABLE_ERROR            /*  */

#ifdef  DEBUG
#define	ENABLE_WARN			/*  */
#define	ENABLE_INFO			/*  */
#define	ENABLE_TRACE			/*  */
#define	ENABLE_DEBUG			/*  */
#endif     /* -----  not DEBUG  ----- */

#ifdef  ENABLE_FATAL
#define	logfatal(x) singleton_t<log_t>::instance()._logfatal x			/*  */
#else      /* -----  not ENABLE_FATAL  ----- */
#define	logfatal(x) {}			/*  */
#endif     /* -----  not ENABLE_FATAL  ----- */

#ifdef  ENABLE_ERROR
#define	logerror(x) singleton_t<log_t>::instance()._logerror x			/*  */
#else      /* -----  not ENABLE_FATAL  ----- */
#define	logerror(x) {}			/*  */
#endif     /* -----  not ENABLE_FATAL  ----- */

#ifdef  ENABLE_WARN
#define	logwarn(x) singleton_t<log_t>::instance()._logwarn x			/*  */
#else      /* -----  not ENABLE_FATAL  ----- */
#define	logwarn(x) {}			/*  */
#endif     /* -----  not ENABLE_FATAL  ----- */

#ifdef  ENABLE_INFO
#define	loginfo(x) singleton_t<log_t>::instance()._loginfo x			/*  */
#else      /* -----  not ENABLE_FATAL  ----- */
#define	loginfo(x) {}			/*  */
#endif     /* -----  not ENABLE_FATAL  ----- */

#ifdef  ENABLE_TARCE
#define	logtrace(x) singleton_t<log_t>::instance()._logtrace x			/*  */
#else      /* -----  not ENABLE_FATAL  ----- */
#define	logtrace(x) {}			/*  */
#endif     /* -----  not ENABLE_FATAL  ----- */

#ifdef  ENABLE_DEBUG
#define	logdebug(x) singleton_t<log_t>::instance()._logdebug x			/*  */
#else      /* -----  not ENABLE_FATAL  ----- */
#define	logdebug(x) {}			/*  */
#endif     /* -----  not ENABLE_FATAL  ----- */

int init_log(const std::string &path,
             const std::string &filename,
             bool flag_print_screen,
             bool flag_write_file,
             int log_level,
             const std::vector<std::string> &modules,
             int max_size = 5000000,
             int max_line = 100000);


#endif   /* ----- #ifndef _LOG_H_INC  ----- */
