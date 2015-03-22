#include	"rpc_connecter.h"
#include	"log.h"

#define LOG "RPC_CONNECTER"

rpc_connecter_t::rpc_connecter_t(boost::asio::io_service& io, rpc_listen_strategy_i* strategy):
m_io_service(io),
m_strategy(strategy),
m_socket(io),
m_use_monitor(false),
m_is_writing(false),
m_total_wait_write(0),
m_connect_type(RPC_PASSIVE),
m_status(RPC_DISCONNECTED),
m_pdata(NULL)
{
    if(m_strategy == NULL)
        throw std::runtime_error("rpc_connecter_t strategy is NULL!");
}

rpc_connecter_t::~rpc_connecter_t()
{
    sync_close_i(RPC_CLOSE);

    if ( m_strategy != NULL ) {
        delete m_strategy;
        m_strategy = NULL;
    }

    if ( m_pdata != NULL ) {
        delete m_pdata;
    }
}

void rpc_connecter_t::remote_open(bool use_monitor)
{
    m_use_monitor = use_monitor;
    m_status = RPC_CONNECTED;
    m_strategy->on_remote_open(shared_from_this());
}

void rpc_connecter_t::close(rpc_close_type_e type)
{
    m_io_service.post(boost::bind(&rpc_connecter_t::sync_close_i, shared_from_this(), type));
}

void rpc_connecter_t::async_read()
{
    m_socket.async_read_some(boost::asio::buffer(m_read_buffer), boost::bind(&rpc_connecter_t::read_complete_i, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void rpc_connecter_t::read_complete_i(const boost::system::error_code& err, size_t bytes_transferred_)
{
    if(err)
    {
        logerror((LOG, "read_complete_i error=<%s>", err.message().c_str()));
        
        if ( !has_status(RPC_DISCONNECTED) ) {
            sync_close_i(RPC_CLOSE);
        }
        return ;
    }

    int ret = m_strategy->on_read(shared_from_this(), m_read_buffer, bytes_transferred_);
    if ( ret ) {
        close(RPC_SHUTDOWN);
    }
}

void rpc_connecter_t::async_write(const string& data)
{
    m_io_service.post(boost::bind(&rpc_connecter_t::sync_write_i, shared_from_this(), std::move(data)));
}

void rpc_connecter_t::async_write(const string&& data)
{
    m_io_service.post(boost::bind(&rpc_connecter_t::sync_write_i, shared_from_this(), std::move(data)));
}

void rpc_connecter_t::sync_write_i(const string& data)
{
    m_total_wait_write += data.size();
    if ( m_total_wait_write >= 1024*1024*10 ) {
        logerror((LOG, "sync_write_i write buff is too full, total:%lu size:%lu", data.size(), m_total_wait_write));
    }

    m_write_queue.push_back(std::move(data));

    if(!m_is_writing)
    {
        start_sending_buffer_i();
    }
}

void rpc_connecter_t::start_sending_buffer_i()
{
    if ( !m_write_queue.empty() ) {
        set_write_state(true);
        boost::asio::async_write(m_socket, std::move(boost::asio::buffer(*m_write_queue.begin())), boost::bind(&rpc_connecter_t::write_complete_i, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
}

void rpc_connecter_t::write_complete_i(const boost::system::error_code& err, size_t bytes_transferred_)
{
    set_write_state(false);

    m_total_wait_write -= bytes_transferred_;
    m_write_queue.pop_front();

    if ( err ) {
        logerror((LOG, "write_completed_i socket error=<%s>", err.message().c_str()));
        if ( !has_status(RPC_DISCONNECTED) ) {
            sync_close_i(RPC_CLOSE);
        }
        return;
    }
    start_sending_buffer_i();
}

int rpc_connecter_t::sync_connect(const string &address, const string &port)
{
    using boost::asio::ip::tcp;
    tcp::resolver resolver(m_io_service);
    tcp::resolver::query query(address, port);
    tcp::resolver::iterator end_point_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    boost::system::error_code err = boost::asio::error::host_not_found;
    while(err && end_point_iterator != end)
    {
        m_socket.close();
        m_socket.connect(*end_point_iterator++, err);
    }

    if ( err ) {
        logwarn((LOG, "sync_connect catch exception=<%s>", err.message().c_str()));
        m_status = RPC_DISCONNECTED;
        return -1;
    }

    m_status = RPC_CONNECTED;
    m_connect_type = RPC_ACTIVE;

    async_read();
    return 0;
}
