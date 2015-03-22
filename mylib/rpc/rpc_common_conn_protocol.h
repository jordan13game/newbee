
#ifndef  _RPC_COMMON_CONN_PROTOCOL_H_INC
#define  _RPC_COMMON_CONN_PROTOCOL_H_INC



#include	"rpc_listen_strategy_i.h"
#include	"rpc_msg_head.h"

class rpc_common_conn_protocal_t: public rpc_listen_strategy_i
{
public:
    typedef boost::shared_ptr<rpc_connecter_t>      sp_rpc_conn_t;
    typedef boost::asio::io_service     io_t;
public:
    rpc_common_conn_protocal_t(io_t& io);
    ~rpc_common_conn_protocal_t();

    void reset();
    int on_remote_open(sp_rpc_conn_t conn_);
    int on_connect(sp_rpc_conn_t conn_, const boost::system::error_code& e_);
    int on_read(sp_rpc_conn_t conn_, const char* data_, size_t bytes_transferred_);
    void on_closed(sp_rpc_conn_t conn_);

    io_t& get_io(){ return m_io; }
private:
    bool has_read_head_end_i() const { return m_msg_size >= sizeof(m_head); }
    bool has_read_body_end_i() const { return m_head.len == m_body.size(); }

    size_t read_head_i(const char *data, size_t left);
    size_t read_body_i(const char *data, size_t left);

    void post_broken_msg_i(sp_rpc_conn_t conn_);
private:
    io_t& m_io;
    size_t m_msg_size;
    struct rpc_msg_head_t m_head;
    string m_body;
    bool m_is_broken;
};


#endif   /* ----- #ifndef _RPC_COMMON_CONN_PROTOCOL_H_INC  ----- */
