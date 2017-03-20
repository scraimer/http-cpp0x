#ifndef CPPX0_SOCKET_WRAPPER_H_
#define CPPX0_SOCKET_WRAPPER_H_

#include <assert.h>

/////////////////////////////////////////////////////////////////////////////////////
// socket wrapper, just so I don't have to worry about a socket staying unreleased when it goes out of scope
//
// I just can't imagine this is a very good implemenation, so it's just a placeholder to be replaced with
// something better later.
//

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>

namespace cppx0
{
namespace http
{

enum socket_error_t
{
	SOCKET_ERROR = -1,
	SOCKET_ERROR_OK = 0,
	SOCKET_ERROR_UNKNOWN = 1,
	SOCKET_ERROR_CREATING_SOCKET,
	SOCKET_ERROR_INVALID_SOCKET,
	SOCKET_ERROR_SETTING_SOCKOPT,
	SOCKET_ERROR_BINDING_SOCKET,
	SOCKET_ERROR_LISTENING_ON_SOCKET,
};

/** Just an RAII wrapper for sockets */
class internal_tcp_socket_wrapper_t
{
 private:
	static const int listen_socket_backlog = 5;
	int _socket_fd;

	// Don't allow copy constructor
	internal_tcp_socket_wrapper_t( internal_tcp_socket_wrapper_t const & ) = delete;
	internal_tcp_socket_wrapper_t( internal_tcp_socket_wrapper_t & ) = delete;
	internal_tcp_socket_wrapper_t & operator=( internal_tcp_socket_wrapper_t const & );

 public:
	internal_tcp_socket_wrapper_t() : _socket_fd( SOCKET_ERROR )
	{
		_socket_fd = ::socket( PF_INET, SOCK_STREAM, 0 );
		assert( _socket_fd != SOCKET_ERROR &&
		        "Error while creating socket. This is an early assert to make it "
		        "easier to debug, but later calls will also be checked." );
	}

	~internal_tcp_socket_wrapper_t()
	{
		if( _socket_fd != SOCKET_ERROR )
		{
			close( _socket_fd );
			_socket_fd = SOCKET_ERROR;
		}
	}

	int get_socket_fd() const
	{
		return _socket_fd;
	}

	bool is_valid() const
	{
		return _socket_fd != SOCKET_ERROR;
	}

	socket_error_t listen( int const port )
	{
		assert( _socket_fd != SOCKET_ERROR &&
		        "Cannot listen on a badly-created socket, or one that was de-allocated." );
		if( _socket_fd == SOCKET_ERROR )
		{
			return SOCKET_ERROR_INVALID_SOCKET;
		}

		// Set socket to be reusable since I hate having to wait for zombie sockets to be released by the
		// Linux kernel
		int tmp_int_holding_one = 1;
		int rc = setsockopt( _socket_fd, SOL_SOCKET, SO_REUSEADDR, (char const *)&tmp_int_holding_one,
		                     sizeof( tmp_int_holding_one ) );
		if( rc )
		{
			return SOCKET_ERROR_SETTING_SOCKOPT;
		}

		// Bind the socket to the port
		struct sockaddr_in addr;
		in_addr_t local_addr = INADDR_ANY;
		addr.sin_addr.s_addr = local_addr;

		// Bind the socket to the defined port
		addr.sin_family = AF_INET;
		addr.sin_port = htons( static_cast<uint16_t>( port ) );
		rc = ::bind( _socket_fd, reinterpret_cast<sockaddr const *>( &addr ), sizeof( addr ) );
		if( rc )
		{
			return SOCKET_ERROR_BINDING_SOCKET;
		}

		rc = ::listen( _socket_fd, listen_socket_backlog );
		if( rc )
		{
			return SOCKET_ERROR_LISTENING_ON_SOCKET;
		}

		return SOCKET_ERROR_OK;
	}
};

} // namespace http
} // namespace cppx0

#endif // ifndef CPPX0_SOCKET_WRAPPER_H_
