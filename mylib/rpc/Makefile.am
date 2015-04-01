INCLUDES=-I../log -I../heart_beat -I../utility 

CXXFLAGS = -Wall -g -O2 -fPIC -Wno-deprecated -std=c++0x -DUSE_TBB_MALLOC -DUSE_PERF -DDEBUG_LOG

noinst_LIBRARIES = lib_rpc.a
lib_rpc_a_LIBFLAGS =
lib_rpc_a_LIBADD =

lib_rpc_a_SOURCES = \
					   rpc_acceptor.cpp     			\
					   rpc_connecter.cpp  				\
					   rpc_common_conn_protocol.cpp  	\
					   rpc_conn_monitor.cpp 			\
					   rpc.cpp 						    \
					   rpc_utility.cpp 					
