#include	"rpc_conn_monitor.h"


rpc_conn_monitor_t::rpc_conn_monitor_t ():
    m_started(false)
{
}  /* -----  end of method rpc_conn_monitor_t::rpc_conn_monitor_t  (constructor)  ----- */

rpc_conn_monitor_t::~rpc_conn_monitor_t ()
{
    stop();
}  /* -----  end of method rpc_conn_monitor_t::~rpc_conn_monitor_t  (destructor)  ----- */


int
rpc_conn_monitor_t::start ( const heart_beat_setting_t &heart_beat_setting )
{
    if(m_started) return 0;

    m_rpc_conn_heart_beat.set_callback_function(boost::bind(&rpc_conn_monitor_t::handle_timeout, this, _1));
    m_rpc_conn_heart_beat.set_timeout(heart_beat_setting.timeout_flag, heart_beat_setting.timeout);
    m_rpc_conn_heart_beat.set_max_limit(heart_beat_setting.max_limit_flag, heart_beat_setting.max_limit);

    if ( m_rpc_conn_heart_beat.start() ) {
        logerror((RPC_CONN_MGR, "start failed to start heart beat service"));
        return -1;
    }
    m_started = true;
    return 0;
}		/* -----  end of method rpc_conn_monitor_t::start  ----- */


int
rpc_conn_monitor_t::stop (  )
{
    if( !m_started ) return 0;
    close_all();
    m_started = false;
    return 0;
}		/* -----  end of method rpc_conn_monitor_t::stop  ----- */


void
rpc_conn_monitor_t::add ( sp_rpc_conn_t sp_conn )
{
    if ( !m_started ) return ;
    
    logtrace((RPC_CONN_MGR, "add connecter"));
    m_rpc_conn_heart_beat.asyn_add_element(sp_conn);
}		/* -----  end of method rpc_conn_monitor_t::add  ----- */


void
rpc_conn_monitor_t::update ( sp_rpc_conn_t sp_conn )
{
    if ( !m_started ) return ;
    
    logtrace((RPC_CONN_MGR, "update connecter"));
    m_rpc_conn_heart_beat.asyn_update_element(sp_conn);
}		/* -----  end of method rpc_conn_monitor_t::add  ----- */


void
rpc_conn_monitor_t::del ( sp_rpc_conn_t sp_conn )
{
    if ( !m_started ) return ;
    
    logtrace((RPC_CONN_MGR, "del connecter"));
    m_rpc_conn_heart_beat.asyn_del_element(sp_conn);
}		/* -----  end of method rpc_conn_monitor_t::add  ----- */


void
rpc_conn_monitor_t::close_all (  )
{
    if (!m_started) return ;
    logwarn((RPC_CONN_MGR, "close_all begin..."));

    m_rpc_conn_heart_beat.trigger_all_timeout();

    logwarn((RPC_CONN_MGR, "close_all end ok"));
}		/* -----  end of method rpc_conn_monitor_t::close_all  ----- */


void
rpc_conn_monitor_t::handle_timeout ( sp_rpc_conn_t sp_conn )
{
    if (!m_started) return;
    logwarn((RPC_CONN_MGR, "handle_timeout, to close connecter..."));

    sp_conn->close(RPC_CLOSE);

    logwarn((RPC_CONN_MGR, "handle_timeout end ok"));
}		/* -----  end of method rpc_conn_monitor_t::handle_timeout  ----- */


void
rpc_conn_monitor_t::start_monitor ( heart_beat_setting_t *heart )
{
    heart_beat_setting_t heart_beat_set(0, 86400, 1, 50000);
    if(heart != NULL)
        heart_beat_set = *heart;
    singleton_t<rpc_conn_monitor_t>::instance().start(heart_beat_set);
}		/* -----  end of method rpc_conn_monitor_t::start_monitor  ----- */


void
rpc_conn_monitor_t::stop_monitor (  )
{
    singleton_t<rpc_conn_monitor_t>::instance().stop();
}		/* -----  end of method rpc_conn_monitor_t::stop_monitor  ----- */

