#include	"rpc.h"
#include	"log.h"
#include	"rpc_utility.h"


#define	LOG "RPC"			/*  */
rpc_t::rpc_t(io_t& io):rpc_common_conn_protocal_t(io)
{
}

template<class T, class TMsg>
void rpc_t::reg_call(void(T::*method_)(sp_rpc_conn_t, TMsg&))
{
    struct call_t : public call_i
    {
        bool happen(rpc_t* rpc, uint16_t cmd, const string& buff, sp_rpc_conn_t conn)
        {
            TMsg msg;
            try
            {
                msg << buff;
            }
            catch(exception &e)
            {
                logerror((LOG, "exception:<%s>", e.what()));
                return false;
            }
            typedef void(T::*F)(sp_rpc_conn_t, TMsg&);
            F tmp = (F)cb;
            (((T*)rpc)->*tmp)(conn, msg);
            return true;
        }
    };

    call_t* c = new call_t;
    c->cb = (void_fun_t)method_;

    m_calls[TMsg::cmd()] = sp_call_i(c);
}

int rpc_t::async_send(sp_rpc_conn_t conn, uint16_t cmd, const string &body)
{
    conn->async_write(rpc_msg_util_t::make_msg(cmd, body));
}

void rpc_t::handle_msg(uint8_t flag, uint16_t cmd, uint16_t res, const string &body, sp_rpc_conn_t conn)
{
    if( flag == 0)
    {
        on_broken(conn);
        return ;
    }
    auto it = m_calls.find(cmd);
    if ( it != m_calls.end() ) {
        if( !it->second->happen(this, cmd, body, conn))
        {
            logerror((LOG, "handle_msg...failed,unpack exception! cmd:%u, body:%s, connecter will be SHUTDOWN", cmd, body.c_str()));
            conn->close(RPC_SHUTDOWN);
            return ;
        }
        logtrace((LOG, "handle_msg ok...cmd=<%u>", cmd));
        return ;
    }

    if ( on_unknown_msg(cmd, body, conn) != 0 ) {
        logerror((LOG, "handle_msg...failed, unknown msg! cmd:%u connecter will be SHUTDOWN", cmd));
        conn->close(RPC_SHUTDOWN);
        string all_cmd;
        for( auto it = m_calls.begin(); it != m_calls.end(); ++it)
        {
            char buf[8];
            sprintf(buf, "%u", it->first);
            all_cmd.append(buf);
            all_cmd.append(",");
        }
        logwarn((LOG, "handle_msg...failed,unknown cmd:%u, body:%s, all_cmd[%s]", cmd, body.c_str(), all_cmd.c_str()));
    }
}
