#ifndef CPPX0_HTTP_SERVER_H_
#define CPPX0_HTTP_SERVER_H_

#include "socket-wrapper.hpp"
#include <assert.h>

namespace cppx0
{
namespace http
{

// Forward declaration, see at the end of the file.
class internal_tcp_socket_wrapper_t;

class request_t
{
};

class server_config_t
{
 public:
	int _port;
	server_config_t() : _port( 0 ) {}
};

enum error_t
{
	OK = 0,
	ERROR,
	ERROR_INIT_LISTEN_SOCKET,
};

template <typename REQUEST_HANDLER> class server_t
{
 private:
	 typedef REQUEST_HANDLER request_handler_t ;

	 request_handler_t _request_handler;
	 internal_tcp_socket_wrapper_t _server_socket;

 public:
	server_t( request_handler_t & request_handler)
	    : _request_handler( request_handler )
	{
	}

   error_t init( server_config_t const config )
	{
		assert( config._port > 0 && "Must specify a non-zero port!" );

		if( _server_socket.listen( config._port ) )
		{
			return ERROR_INIT_LISTEN_SOCKET;
		}

		return OK;
	}

	error_t work()
	{
		// TODO: check for incoming connections and for clients sending requests

		return OK;
	}
};

} // namespace http
} // namespace cppx0

#endif // ifndef CPPX0_HTTP_SERVER_H_
