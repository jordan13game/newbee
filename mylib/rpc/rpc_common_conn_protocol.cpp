#include	"rpc_common_conn_protocol.h"
#include	"rpc_conn_monitor.h"


#define	LOG "RPC_CCP"			/*  */

rpc_common_conn_protocal_t::rpc_common_conn_protocal_t(io_t& io):
m_io(io),
m_msg_size(0),
m_is_broken(false)
{
}

void rpc_common_conn_protocal_t::reset()
{
    m_msg_size = 0;
    m_is_broken = false;
    m_body.clear();
}

rpc_common_conn_protocal_t::~rpc_common_conn_protocal_t()
{
}

int rpc_common_conn_protocal_t::on_remote_open(sp_rpc_conn_t conn_)
{

    if ( conn_->has_conn_type(RPC_PASSIVE) ) {
        logtrace((LOG, "add a passive conn."));
        if ( conn_->has_monitor() ) {
            singleton_t<rpc_conn_monitor_t>::instance().add(conn_);
        }
    }
    reset();
    conn_->async_read();
    return 0;
}

int rpc_common_conn_protocal_t::on_connect(sp_rpc_conn_t conn_, const boost::system::error_code& e_)
{

    if ( e_ ) {
        logerror((LOG, "on_connect error=<%s>", e_.message().c_str()));
        post_broken_msg_i(conn_);
    } else {
        reset();
        conn_->async_read();
    }
    return 0;
}

int rpc_common_conn_protocal_t::on_read(sp_rpc_conn_t conn_, const char* data_, size_t bytes_transferred_)
{
    logtrace((LOG, "on_read begin... size=<%u>, msg_size<%u>", bytes_transferred_, m_msg_size));
    size_t left = bytes_transferred_;
    size_t tmp = 0;

    while( left > 0)
    {
        if ( !has_read_head_end_i() )
        {
            tmp = read_head_i(data_, left);
            left -= tmp;
            data_ += tmp;
            if ( has_read_head_end_i() ) {
                logtrace((LOG, "read head end, cmd:%u, body size:%u", m_head.cmd, m_head.len));
            }
        }
        else 
        {
            tmp = read_body_i(data_, left);
            left -= tmp;
            data_ += tmp;
            if ( has_read_body_end_i() ) {
                if ( conn_->has_conn_type(RPC_PASSIVE) ) {
                    if ( conn_->has_monitor() ) {
                        singleton_t<rpc_conn_monitor_t>::instance().update(conn_);
                    }

                }

                m_io.post(boost::bind(&rpc_common_conn_protocal_t::handle_msg, this, 1, m_head.cmd, m_head.res, m_body, conn_));
                memset(&m_head, 0, sizeof(m_head));
                m_msg_size = 0;
                m_body.clear();
            }
        }
    }
    conn_->async_read();
    return 0;
}

size_t rpc_common_conn_protocal_t::read_head_i(const char *data, size_t left)
{
    size_t tmp = sizeof(m_head) - m_msg_size;
    if( tmp > left) tmp = left;

    void *dest = (char *)&m_head + m_msg_size;
    memcpy(dest, data, tmp);
    m_msg_size += tmp;
    return tmp;
}

size_t rpc_common_conn_protocal_t::read_body_i(const char *data, size_t left)
{
    size_t tmp = sizeof(m_head) + m_head.len - m_msg_size;
    if ( tmp > left ) tmp = left;
    m_body.append(data, tmp);
    m_msg_size += tmp;
    return tmp;
}

void rpc_common_conn_protocal_t::on_closed(sp_rpc_conn_t conn_)
{

    if ( conn_->has_conn_type(RPC_PASSIVE) ) {
        if ( conn_->has_monitor() ) {
            singleton_t<rpc_conn_monitor_t>::instance().del(conn_);
        }
    }
    post_broken_msg_i(conn_);
}

void rpc_common_conn_protocal_t::post_broken_msg_i(sp_rpc_conn_t conn_)
{
    if( m_is_broken ) return ;
    m_is_broken = true;
    m_io.post(boost::bind(&rpc_common_conn_protocal_t::handle_msg, this, 0, 0, 0, "", conn_));
}
