#include	"log.h"


#define	log singleton_t<log_t>::instance()			/*  */

int init_log(const std::string &path,
             const std::string &filename,
             bool flag_print_screen,
             bool flag_write_file,
             int log_level,
             const std::vector<std::string> &modules,
             int max_size,
             int max_line)
{
    log.set_path(path.c_str());
    log.set_filename(filename.c_str());
    log.set_maxline(max_line);
    log.set_maxsize(max_size);
    log.enable_log_level(LOG_FLAG(log_level + 1) - 1, true);
    if( flag_write_file ) log.enable_write_file(true);
    if( flag_print_screen ) log.enable_print_screen(true);
    for ( size_t i = 0; i < modules.size() ; i++ ) {
        log.enable_module(modules[i].c_str(), true);
    }
    return log.open();
}

