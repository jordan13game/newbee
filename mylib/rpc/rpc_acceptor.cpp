#include	"rpc_acceptor.h"
#include	"log.h"
#include	"io_service_pool.h"


#define	LOG "RPC_ACCEPTOR"			/*  */

rpc_acceptor_t::rpc_acceptor_t(io_t& io, cb_listen_strategy_creator_t cb):
m_io(io),
m_acceptor(io),
m_use_monitor(false),
m_started(false),
m_cb_ls_creator(cb)
{
}

rpc_acceptor_t::~rpc_acceptor_t()
{

    if ( m_started ) {
        m_started = false;
        sync_close_i();
    }
}

int rpc_acceptor_t::start(const string& ip, const string& port, bool use_monitor)
{
    if ( m_started ) {
        logwarn((LOG, "rpc_acceptor_t already started"));
        return 0;
    }

    m_use_monitor = use_monitor;
    m_ip = ip;
    m_port = port;

    sync_start_listen_i();

    m_started = true;
    return 0;
}

void rpc_acceptor_t::sync_start_listen_i()
{
    
    try {
        using boost::asio::ip::tcp;
        tcp::resolver resolver(m_io);
        tcp::endpoint endpoint;
        tcp::resolver::query query(tcp::v4(), m_ip, m_port);
        endpoint = *resolver.resolve(query);

        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();

        sp_rpc_conn_t tmp(new rpc_connecter_t(io_pool.get_io_service(), m_cb_ls_creator()));
        m_acceptor.async_accept(tmp->socket(), boost::bind(&rpc_acceptor_t::sync_accept_i, this, tmp, boost::asio::placeholders::error));
        
    }
    catch ( const std::exception &ExceptObj ) {		/* handle exception: */
        logerror((LOG, "sync_start_listen_i err=<%s>", ExceptObj.what()));
    }
    catch (...) {		/* handle exception: unspezified */
    }

}

void rpc_acceptor_t::sync_accept_i(sp_rpc_conn_t conn, boost::system::error_code& err)
{
    try {

        if ( err ) {
            logerror((LOG, "sync_accept_i error<%s>", err.message().c_str()));
            return ;
        }
        sp_rpc_conn_t tmp(new rpc_connecter_t(io_pool.get_io_service(), m_cb_ls_creator()));
        m_acceptor.async_accept(tmp->socket(), boost::bind(&rpc_acceptor_t::sync_accept_i, this, tmp, boost::asio::placeholders::error));

        conn->remote_open(m_use_monitor);
    }
    catch ( const std::exception &ExceptObj ) {		/* handle exception: */
        logerror((LOG, "sync_accept_i exception:<%s>", ExceptObj.what()));
    }
    catch (...) {		/* handle exception: unspezified */
    }

}

void rpc_acceptor_t::sync_close_i()
{
    try {
        m_acceptor.cancel();
        m_acceptor.close();
    }
    catch ( const std::exception &ExceptObj ) {		/* handle exception: */
        logerror((LOG, "sync_close_i exception:<%s>", ExceptObj.what()));
    }
    catch (...) {		/* handle exception: unspezified */
    }

}

void rpc_acceptor_t::stop()
{

    if ( !m_started ) {
        logwarn((LOG, "rpc_acceptor_t is alread stoped"));
        return ;
    }
    m_started = false;
    sync_close_i();
}
