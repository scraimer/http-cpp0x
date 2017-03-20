#ifndef CPPX0_HTTP_SERVER_H_
#define CPPX0_HTTP_SERVER_H_

#include "socket-wrapper.hpp"
#include <assert.h>
#include <sys/select.h>
#include <algorithm>

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
	server_config_t() : _port( 0 )
	{
	}
};

enum error_t
{
	OK = 0,
	ERROR,
	ERROR_INIT_LISTEN_SOCKET,
	ERROR_POLLING_SOCKETS,
};

template <typename REQUEST_HANDLER> class server_t
{
 private:
	typedef REQUEST_HANDLER request_handler_t;

	static int const MAX_CLIENTS = 10;

	request_handler_t _request_handler;
	internal_tcp_socket_wrapper_t _server_socket;
	internal_tcp_socket_wrapper_t _client_sockets[MAX_CLIENTS];

	class clients_range_t;
	clients_range_t _all_clients;

 public:
	server_t( request_handler_t & request_handler )
	    : _request_handler( request_handler )
	    , _all_clients( &_client_sockets[0], &_client_sockets[MAX_CLIENTS] )
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
		// check for incoming connections and for clients sending requests

		fd_collector_for_select_t fds_to_poll;
		fds_to_poll = std::for_each( _all_clients.begin(), _all_clients.end(), fds_to_poll );
		fds_to_poll.add( _server_socket );

		auto fds = fds_to_poll._fds;
		int rc = select( fds_to_poll._largest + 1, &fds, nullptr, nullptr, nullptr );
		if( rc < 0 )
		{
			return ERROR_POLLING_SOCKETS;
		}

		if( rc == 0 )
		{
			return OK;
		}

		std::for_each( _all_clients.begin(), _all_clients.end(), service_readable_client( fds, *this ) );
		if( FD_ISSET( _server_socket.get_socket_fd(), &fds ) )
		{
			accept_new_client();
		}

		return OK;
	}

 private:
	void accept_new_client()
	{
		// TODO: accept incoming clients
	}

	void service_client( internal_tcp_socket_wrapper_t & client )
	{
		// TODO: read incoming data
		// TODO: if got an entire HTTP header, parse and notify the user that a request has arrived.
	}

	class clients_range_t
	{
	 public:
		typedef internal_tcp_socket_wrapper_t * item_t;

	 private:
		item_t _begin;
		item_t _end;

	 public:
		clients_range_t( item_t begin, item_t end ) : _begin( begin ), _end( end )
		{
		}

		item_t begin()
		{
			return _begin;
		}
		item_t end()
		{
			return _end;
		}
	};

	class service_readable_client
	{
	 private:
		fd_set _fds;
		server_t & _server;

	 public:
		service_readable_client( fd_set const & fds, server_t & server ) : _fds( fds ), _server( server )
		{
		}

		void operator()( internal_tcp_socket_wrapper_t & sock_wrapper )
		{
			if( FD_ISSET( sock_wrapper.get_socket_fd(), &_fds ) )
			{
				_server.service_client( sock_wrapper );
			}
		}
	};

	struct fd_collector_for_select_t
	{
		int _largest;
		fd_set _fds;

		fd_collector_for_select_t() : _largest( -1 )
		{
			FD_ZERO( &_fds );
		}

		void add( internal_tcp_socket_wrapper_t const & sock_wrapper )
		{
			if( !sock_wrapper.is_valid() )
			{
				return;
			}

			int fd = sock_wrapper.get_socket_fd();
			if( _largest < fd )
			{
				_largest = fd;
			}

			FD_SET( fd, &_fds );
		}

		void operator()( internal_tcp_socket_wrapper_t const & sock_wrapper )
		{
			add( sock_wrapper );
		}
	};
};

} // namespace http
} // namespace cppx0

#endif // ifndef CPPX0_HTTP_SERVER_H_
