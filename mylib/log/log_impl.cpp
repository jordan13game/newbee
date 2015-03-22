#include	"log_impl.h"
#include	<string.h>
#include	<sys/syscall.h>
#define gettid() syscall(SYS_gettid)


const char * log_t::m_log_level_desp[] =
{
    "FATAL",
    "ERROR",
    "WARN ",
    "INFO ",
    "TRACE",
    "DEBUG"
};

log_t::log_t()
{
    m_maxline = 50000;
    m_maxsize = 102400;

    m_cur_sn = 0;
    m_cur_year = 0;
    m_cur_month = 0;
    m_cur_day = 0;

    m_cur_size = 0;
    m_cur_line = 0;

    m_enable_write_file = false;
    m_enable_print_screen = false;

    m_log_level = 0;
    m_opened = false;

    memset(m_path, 0, sizeof(m_path));
    memset(m_filename, 0, sizeof(m_filename));
    
}

log_t::~log_t()
{
    close();
}


void log_t::set_path(const char *path)
{
    strcpy(m_path, path);
}

void log_t::set_filename(const char *filename)
{
    strcpy(m_filename, filename);
}

void log_t::set_maxline(int maxline)
{
    m_maxline = maxline;
}

void log_t::set_maxsize(int maxsize)
{
    m_maxsize = maxsize;
}

void log_t::enable_log_level(unsigned int log_level, bool enable)
{
    if ( enable ) {
        if ( LOG_FLAG(ALL_LOG_LEVEL) == log_level ) {
            unsigned int z = 0;
            m_log_level = ~z;
        }

        m_log_level |= log_level;
    } else {
        if ( LOG_FLAG(ALL_LOG_LEVEL) == log_level ) {
            m_log_level = 0;
        }

        m_log_level &= ~log_level;
    }
}

void log_t::enable_module(const char *module, bool enable)
{
    if ( enable ) {
        modules_.insert(module);
    } else {
        modules_.erase(module);
    }
}

void log_t::enable_print_screen(bool enable)
{
    m_enable_print_screen = enable;
}

void log_t::enable_write_file(bool enable)
{
    m_enable_write_file = enable;
}

int log_t::open()
{
    if(m_opened) return 0;

    time_t ti = time(NULL);
    struct tm *tmp = localtime(&ti);

    m_cur_year = tmp->tm_year;
    m_cur_month = tmp->tm_mon;
    m_cur_day = tmp->tm_mday;

    m_cur_sn = 0;
    m_cur_line = 0;
    m_cur_size = 0;

    int ret = 0;

    if ( m_enable_write_file ) {
        ret = open_file_thread();
    }

    m_opened = true;
    return ret;
}

int log_t::open_file_thread()
{
    assert(!m_opened);

    char file[1024];

    int rc = access(m_path, F_OK);

    if ( rc != 0 ) {
        rc = mkdir(m_path, 0777);
        if ( rc != 0 ) {
            std::cout << "log_t::open_file_thread mkdir=" << m_path << "failed:" << strerror(errno) << std::endl;
            return -1;
        }
    }

    sprintf(file, "%s/%d-%d-%d", m_path, m_cur_year + 1900, m_cur_month + 1, m_cur_day);
    rc = access(file, F_OK);

    if ( rc != 0 ) {
        rc = mkdir(file, 0777);
        if ( rc != 0 ) {
            std::cout << "log_t::open_file_thread mkdir=" << file << "failed:" << strerror(errno) << std::endl;
            return -1;
        }
    }
    
    while(true)
    {
        sprintf(file, "%s/%d-%d-%d/%s.%d", m_path, m_cur_year + 1900, m_cur_month + 1, m_cur_day, m_filename, m_cur_sn);
        rc = access(file, F_OK);
        if ( rc == 0 ) {
            m_cur_sn++;
            continue;
        }
        break;
    }

    ofstream_.open(file);

    work_.reset(new boost::asio::io_service::work(work_io_service_));
    work_thread_.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &work_io_service_)));

    if ( !work_thread_ ) {
        std::cout << "create file thread failed!" << std::endl;
        return -1;
    }
    return 0;

}

void log_t::close()
{
    if( !m_opened )
        return ;
    m_opened = false;

    modules_.clear();
    work_.reset();

    if(work_thread_)
        work_thread_->join();
}

int log_t::log(const char * module, int level, const char * fmt, va_list ap, const char * begin_color, const char *end_color)
{
    int ret = check_module(module);

    if( ret ) return ret;

    ret = check_log_level(level);

    if( ret ) return ret;

    time_t ti = time(NULL);
    struct tm *tmp = localtime(&ti);

    char logmsg[MAX_LOG_LINE];
    format_log_head(logmsg, module, level, tmp);

    int len = strlen(logmsg);
    vsnprintf(logmsg + len, MAX_LOG_LINE - len - 2, fmt, ap);

    strcat(logmsg, "\n");
    logmsg[MAX_LOG_LINE - 1] = 0;

    string log_txt = string(logmsg);

    if ( m_enable_write_file ) {
        work_io_service_.post(boost::bind(&log_t::handle_write_file, this, log_txt));
    }

    if ( m_enable_print_screen ) {
        cout << begin_color + log_txt + end_color << endl;
    }

    return ret;
}

int log_t::check_module(const char *module)
{
    if ( modules_.find(module) != modules_.end() ) {
        return 0;
    }
    else 
        return -1;
}

int log_t::check_log_level(int level)
{
    return m_log_level & LOG_FLAG(level);
}

void log_t::format_log_head(char *str, const char *module, int level, struct tm *tmp)
{
    struct timeval curtm;
    gettimeofday(&curtm, NULL);
    sprintf(str, "%04d%02d%02d %02d:%02d:%02d.%06ld %s [%ld] [%-15s]:", tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec, curtm.tv_usec, m_log_level_desp[level], gettid(), module);
}

void log_t::handle_write_file(const string &txt)
{
    time_t ti = time(NULL);
    struct tm *tmp = localtime(&ti);

    if ( tmp->tm_year != m_cur_year || tmp->tm_mon != m_cur_month || tmp->tm_mday != m_cur_day ) {
        ofstream_.close();
        ofstream_.clear();

        m_cur_year = tmp->tm_year;
        m_cur_month = tmp->tm_mon;
        m_cur_day = tmp->tm_mday;

        m_cur_sn = 0;
        m_cur_line = 0;
        m_cur_size = 0;

        char file[1024];
        sprintf(file, "%s/%d-%d-%d", m_path, m_cur_year + 1900, m_cur_month + 1, m_cur_day);
        mkdir(file,0777);

        sprintf(file, "%s/%d-%d-%d/%s.%d", m_path, m_cur_year + 1900, m_cur_month + 1, m_cur_day, m_filename, m_cur_sn);
        ofstream_.open(file);
    }
    ofstream_ << txt;
    ofstream_.flush();

    if ( m_cur_line >= m_maxline || m_cur_size >= m_maxsize ) {
        ofstream_.close();
        ofstream_.clear();

        m_cur_sn++;
        m_cur_size = 0;
        m_cur_line = 0;

        char file[1024];
        sprintf(file, "%s/%d-%d-%d/%s.%d", m_path, m_cur_year + 1900, m_cur_month + 1, m_cur_day, m_filename, m_cur_sn);
        ofstream_.open(file);
        
    }
    m_cur_size += txt.length();
    m_cur_line ++;

}

void log_t::_logfatal(const char *module, const char *fmt, ...)
{
    if(!m_opened) return;
    va_list vl;
    va_start(vl, fmt);
    log(module, LF_FATAL, fmt, vl, "\033[0;35m", "\033[0m");
    va_end(vl);
}

void log_t::_logerror(const char *module, const char *fmt, ...)
{
    if(!m_opened) return;
    va_list vl;
    va_start(vl, fmt);
    log(module, LF_ERROR, fmt, vl, "\033[0;31m", "\033[0m");
    va_end(vl);
}

void log_t::_logwarn(const char *module, const char *fmt, ...)
{
    if(!m_opened) return;
    va_list vl;
    va_start(vl, fmt);
    log(module, LF_WARN, fmt, vl, "\033[1;33m", "\033[0m");
    va_end(vl);
}

void log_t::_loginfo(const char *module, const char *fmt, ...)
{
    if(!m_opened) return;
    va_list vl;
    va_start(vl, fmt);
    log(module, LF_INFO, fmt, vl, "\033[1;33m", "\033[0m");
    va_end(vl);
}

void log_t::_logtrace(const char *module, const char *fmt, ...)
{
    if(!m_opened) return;
    va_list vl;
    va_start(vl, fmt);
    log(module, LF_TRACE, fmt, vl, "", "");
    va_end(vl);
}

void log_t::_logdebug(const char *module, const char *fmt, ...)
{
    if(!m_opened) return;
    va_list vl;
    va_start(vl, fmt);
    log(module, LF_DEBUG, fmt, vl, "\033[1;33m", "\033[0m");
    va_end(vl);
}

