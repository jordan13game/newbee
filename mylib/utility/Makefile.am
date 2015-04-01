INCLUDES= -I../log/

#CXXFLAGS = -Wall -g -O2 -fPIC
AM_CPPFLAGS = -Wall -O2  -fPIC -Wno-deprecated -std=c++0x -DUSE_TBB_MALLOC

noinst_LIBRARIES = lib_utility.a

lib_utility_a_SOURCES = \
		signal_handler.cpp \
		zlib_util.cpp \
		io_service_pool.cpp 
