
#ifndef  _RPC_CONNECTER_H_INC
#define  _RPC_CONNECTER_H_INC

#include	<string>
#include	<deque>

#include	<boost/asio.hpp>
#include	<boost/enable_shared_from_this.hpp>
#include	<boost/shared_ptr.hpp>
#include	<boost/noncopyable.hpp>
#include	<boost/bind.hpp>

#include	"rpc_listen_strategy_i.h"
#include	"rpc_def.h"

using namespace std;

class rpc_listen_strategy_i;
class rpc_connecter_t: public boost::enable_shared_from_this<rpc_connecter_t>, private boost::noncopyable
{
public:
    enum { BUFFER_SIZE = 1024 * 8};

public:
    rpc_connecter_t(boost::asio::io_service& io, rpc_listen_strategy_i* strategy);
    ~rpc_connecter_t();

    void remote_open(bool use_monitor = false);
    void close(rpc_close_type_e type = RPC_CLOSE);

    void async_read();
    void async_write(const string& data);
    void async_write(const string&& data);

    int sync_connect(const string &address, const string &port);
    template <class T>
    T* get_data()
    {
        return static_cast<T*>(m_pdata);
    }

    void set_data(void *pdata)
    {
        if(m_pdata != NULL)
        {
            delete m_pdata;
        }
        m_pdata = pdata;
    }

    bool has_status(rpc_status_e status)
    {
        return m_status == status;
    }
    bool has_monitor() { return m_use_monitor; }
    bool has_conn_type(rpc_connection_type_e type)
    {
        return m_connect_type == type;
    }
    boost::asio::ip::tcp::socket& socket()
    { return m_socket; }
private:
    void sync_close_i(rpc_close_type_e type);

    void read_complete_i(const boost::system::error_code& err, size_t bytes_transferred_);

    void sync_write_i(const string& data);

    void write_complete_i(const boost::system::error_code& err, size_t bytes_transferred_);

    void start_sending_buffer_i();

    void set_write_state(bool writing);
private:
    boost::asio::io_service& m_io_service;
    rpc_listen_strategy_i* m_strategy;

    boost::asio::ip::tcp::socket m_socket;

    char m_read_buffer[BUFFER_SIZE];
    bool m_use_monitor;

    deque<string> m_write_queue;
    bool m_is_writing;
    size_t m_total_wait_write;

    rpc_connection_type_e m_connect_type;
    rpc_status_e m_status;

    void * m_pdata;

};




#endif   /* ----- #ifndef _RPC_CONNECTER_H_INC  ----- */
