#ifndef  _RPC_H_INC
#define  _RPC_H_INC


#include	<boost/noncopyable.hpp>
#include	<boost/shared_ptr.hpp>

#include	<unordered_map>
using namespace std;

#include	"rpc_common_conn_protocol.h"



class rpc_t : public rpc_common_conn_protocal_t, private boost::noncopyable
{
private:
    typedef void(rpc_t::*void_fun_t)();
    struct call_i
    {
        void_fun_t cb;
        virtual bool happen(rpc_t* rpc, uint16_t cmd, const string& buff, sp_rpc_conn_t conn) = 0;
    };

    typedef boost::shared_ptr<call_i> sp_call_i;
    typedef unordered_map<uint16_t, sp_call_i> sp_call_hm_t;
public:
    rpc_t(io_t& io);
    virtual ~rpc_t(){}

    template<class T, class TMsg>
    void reg_call(void(T::*method_)(sp_rpc_conn_t, TMsg&));

    template<class TMsg>
    static int async_call(sp_rpc_conn_t conn, TMsg& msg)
    {
        string out;
        msg >> out;
        async_send(conn, TMsg::cmd(), out);
    }
    virtual void on_broken(sp_rpc_conn_t conn) = 0;
    virtual int on_unknown_msg(uint16_t cmd, const string &body, sp_rpc_conn_t conn) { return -1; }
protected:
    static int async_send(sp_rpc_conn_t conn, uint16_t cmd, const string &body);
    void handle_msg(uint8_t flag, uint16_t cmd, uint16_t res, const string &body, sp_rpc_conn_t conn);
private:
    sp_call_hm_t m_calls;
};



#endif   /* ----- #ifndef _RPC_H_INC  ----- */
