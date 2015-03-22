
#ifndef  _LOG_IMPL_H__INC
#define  _LOG_IMPL_H__INC

#include	<stdarg.h>
#include	<string.h>

#include	<fstream>
#include	<sstream>
#include	<set>
#include	<string>
using namespace std;

#include	<boost/asio.hpp>
#include	<boost/thread.hpp>
#include	<boost/bind.hpp>
#include	<boost/scoped_ptr.hpp>

enum
{
    LF_FATAL = 0,
    LF_ERROR,
    LF_WARN,
    LF_INFO,
    LF_TRACE,
    LF_DEBUG,
    ALL_LOG_LEVEL
};

#define LOG_FLAG(x) ( 1 << (x) )

class log_t
{
public:
    log_t();
    ~log_t();
    void set_path(const char *path);
    void set_filename(const char *filename);
    void set_maxline(int maxline);
    void set_maxsize(int maxsize);

    void enable_log_level(unsigned int log_level, bool enable);

    void enable_module(const char *module, bool enable);

    void enable_print_screen(bool enable);
    void enable_write_file(bool enable);

    int open();
    void close();

    void _logfatal(const char *module, const char *fmt, ...);
    void _logerror(const char *module, const char *fmt, ...);
    void _logwarn(const char *module, const char *fmt, ...);
    void _loginfo(const char *module, const char *fmt, ...);
    void _logtrace(const char *module, const char *fmt, ...);
    void _logdebug(const char *module, const char *fmt, ...);

private:

    int open_file_thread();

    int log(const char * module, int level, const char * fmt, va_list ap, const char * begin_color, const char *end_color);

    int check_module(const char *module);

    int check_log_level(int level);

    void format_log_head(char *str, const char *module, int level, struct tm *tmp);

    void handle_write_file(const string &txt);

private:
    boost::asio::io_service work_io_service_;

    boost::scoped_ptr<boost::thread> work_thread_;

    boost::scoped_ptr<boost::asio::io_service::work> work_;

    set<string> modules_;

    ofstream ofstream_;

private:

    static const int MAX_LOG_LINE = 102400;

    bool m_opened;

    bool m_enable_print_screen;

    bool m_enable_write_file;

    char m_path[1024];

    char m_filename[1024];

    unsigned int m_log_level;

    int m_maxline;

    int m_maxsize;

    int m_cur_year;
    int m_cur_month;
    int m_cur_day;

    int m_cur_sn;

    int m_cur_size;

    int m_cur_line;

    static const char* m_log_level_desp[];
};


#endif   /* ----- #ifndef _LOG_IMPL_H__INC  ----- */
