#ifndef  _RPC_SERVER_H_INC
#define  _RPC_SERVER_H_INC

#include	"rpc.h"
#include	"io_service_pool.h"
#include	"rpc_acceptor.h"
#include	"log.h"

#define LOG "RPC_SERVER"

template <class Listener>
class rpc_server_t
{
    typedef boost::asio::io_service io_t;
    typedef boost::shared_ptr<rpc_acceptor_t> sp_rpc_acc_t;
    typedef boost::shared_ptr<rpc_connecter_t> sp_rpc_conn_t;
public:
    rpc_server_t():m_io(io_pool.get_io_service()){}
    rpc_server_t(io_t &io):m_io(io){}

    int listen(const string &host, const string &port, bool use_monitor = false)
    {
        if(!m_sp_acc)
        {
            logtrace((LOG, "listen: ip:%s port:%s", host.c_str(), port.c_str()));
            m_sp_acc.reset(new rpc_acceptor_t(io_pool.get_io_service(), boost::bind(&rpc_server_t<Listener>::create_listener, boost::ref(m_io))));
        }

        try {
            m_sp_acc->start(host, port, use_monitor);
            return 0;
        }
        catch ( const exception &ExceptObj ) {		/* handle exception: */
            logerror((LOG, "listen exception:%s, ip:%s, port:%s", ExceptObj.what(), host.c_str(), port.c_str()));
            m_sp_acc.reset();
            return -1;
        }
        catch (...) {		/* handle exception: unspezified */
        }

    }

    virtual void close()
    {
        if ( m_sp_acc ) {
            m_sp_acc->stop();
        }
    }

    io_t& get_io()
    {
        return m_io;
    }

    template <typename F, typename ... Args>
    void async_do(F fun, Args&... args)
    {
        m_io.post(boost::bind(&rpc_server_t<Listener>::handle_do<F, Args...>, this, fun, args...));
    }

    template <class TMsg>
    void async_call(sp_rpc_conn_t conn, TMsg& msg)
    {
        string out;
        msg >> out; 
        async_call(conn, TMsg::cmd(), out);
    }

    void async_call(sp_rpc_conn_t conn, int cmd, const string &body)
    {
        conn->async_write(std::move(rpc_msg_util_t::compress_msg(cmd, body)));
    }
private:
    static rpc_listen_strategy_i* create_listener (io_t& io)
    {
        return new Listener(io);
    }

    template <typename F, typename ... Args>
    void handle_do(F fun, Args&... args)
    {
        fun(args...);
    }
private:
    io_t& m_io;
    sp_rpc_acc_t m_sp_acc;
};


#endif   /* ----- #ifndef _RPC_SERVER_H_INC  ----- */
