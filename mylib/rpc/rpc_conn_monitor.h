#ifndef  _RPC_CONN_MONITOR_H_
#define  _RPC_CONN_MONITOR_H_


#include	<unordered_map>


#include	<boost/shared_ptr.hpp>
#include	<boost/thread.hpp>
#include	"singleton.h"
#include	"heart_beat_service.h"

struct heart_beat_setting_t
{
    heart_beat_setting_t()
    {
        memset(this, 0 ,sizeof(heart_beat_setting_t));
    }
    
    heart_beat_setting_t(bool timeout_flag, unsigned int timeout, bool max_limit_flag, unsigned long max_limit):
        timeout_flag(timeout_flag),
        timeout(timeout),
        max_limit_flag(max_limit_flag),
        max_limit(max_limit)
    {
    }


    bool timeout_flag;
    unsigned int timeout;
    bool max_limit_flag;
    unsigned long max_limit;
};


class rpc_connecter_t;

class rpc_conn_monitor_t
{
public:
    typedef boost::shared_ptr<rpc_connecter_t> sp_rpc_conn_t;
    rpc_conn_monitor_t ();                             /* constructor */
    ~rpc_conn_monitor_t();
private:
    struct hash_conn_t
    {
        size_t operator ()(const sp_rpc_conn_t &p)const
        {
            return (size_t)p.get();
        }
    };
    typedef heart_beat_service_t<sp_rpc_conn_t, hash_conn_t> rpc_conn_heart_beat_t;

public:
    int start(const heart_beat_setting_t &heart_beat_setting);
    int stop();

    void add(sp_rpc_conn_t sp_conn);
    void update(sp_rpc_conn_t sp_conn);
    void del(sp_rpc_conn_t sp_conn);

    void close_all();

    static void start_monitor(heart_beat_setting_t *heart);
    static void stop_monitor();

private:
    void handle_timeout(sp_rpc_conn_t sp_conn);

private:
    bool m_started;
    rpc_conn_heart_beat_t m_rpc_conn_heart_beat;

}; /* -----  end of class rpc_conn_monitor_t  ----- */

#endif   /* ----- #ifndef _RPC_CONN_MONITOR_H_  ----- */
