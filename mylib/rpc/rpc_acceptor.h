#ifndef  _RPC_ACCEPTOR_H_INC
#define  _RPC_ACCEPTOR_H_INC


#include	<boost/noncopyable.hpp>
#include	<boost/function.hpp>

#include	"rpc_connecter.h"


class rpc_acceptor_t : private boost::noncopyable
{
public:
    typedef boost::shared_ptr<rpc_connecter_t> sp_rpc_conn_t;
    typedef boost::asio::io_service io_t;
    typedef boost::function<rpc_listen_strategy_i*()> cb_listen_strategy_creator_t;

public:
    rpc_acceptor_t(io_t& io, cb_listen_strategy_creator_t cb);
    virtual ~rpc_acceptor_t();

    int start(const string& ip, const string& port, bool use_monitor = false);
    void stop();

protected:
    void sync_start_listen_i();
    void sync_accept_i(sp_rpc_conn_t conn, boost::system::error_code& err);
    void sync_close_i();
private:
    io_t& m_io;
    boost::asio::ip::tcp::acceptor m_acceptor;

    string m_ip;
    string m_port;

    bool m_use_monitor;
    bool m_started;

    cb_listen_strategy_creator_t m_cb_ls_creator;
};

#endif   /* ----- #ifndef _RPC_ACCEPTOR_H_INC  ----- */
