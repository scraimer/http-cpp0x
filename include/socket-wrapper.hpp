#ifndef CPP0X_SOCKET_WRAPPER_H_
#define CPP0X_SOCKET_WRAPPER_H_

/////////////////////////////////////////////////////////////////////////////////////
// socket wrapper, just so I don't have to worry about a socket staying unreleased when it goes out of scope
//
// I just can't imagine this is a very good implemenation, so it's just a placeholder to be replaced with
// something better later.
//

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <assert.h>
#include <string>
#include <errno.h>
#include <vector>


namespace cpp0x
{
namespace http
{

/** Type for buffers. Using buffer instead of just string, since buffer may contain null-characters.
 * 
 * Right now I'm using std::vector since it's easy to resize. Just don't point into its data while resizing it!
 */
typedef std::vector<char> buffer_t;

static std::ostream & operator<<( std::ostream & out, cpp0x::http::buffer_t const & src )
{
	for( auto c : src )
	{
		out << c;
	}
	return out;
}

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
	SOCKET_ERROR_ACCEPTING_CONNECTION,
	SOCKET_ERROR_DISCONNECT_BY_PEER,
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
	}

	~internal_tcp_socket_wrapper_t()
	{
		close();
	}

	void close()
	{
		if( _socket_fd != SOCKET_ERROR )
		{
			::close( _socket_fd );
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
		_socket_fd = ::socket( PF_INET, SOCK_STREAM, 0 );
		assert( _socket_fd != SOCKET_ERROR &&
		        "Error while creating socket. This is an early assert to make it "
		        "easier to debug, but later calls will also be checked." );

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

	/** Accept incoming connection (assuming this socket is a listening socket) into the destination socket
	 *
	 * Returns 'OK' on success.
	 */
	socket_error_t accept_into( internal_tcp_socket_wrapper_t & dest )
	{
		if( !is_valid() )
		{
			return SOCKET_ERROR_INVALID_SOCKET;
		}

		int new_client_fd = ::accept( _socket_fd, nullptr, nullptr );
		if( new_client_fd == SOCKET_ERROR )
		{
			return SOCKET_ERROR_ACCEPTING_CONNECTION;
		}

		dest.replace_socket_fd( new_client_fd );

		return SOCKET_ERROR_OK;
	}

	socket_error_t reject_incoming_connection(std::string const & err_msg)
	{
		internal_tcp_socket_wrapper_t new_client;
		auto result = accept_into( new_client );
		if( result != SOCKET_ERROR_OK )
		{
			return result;
		}

		new_client.send( err_msg );
		new_client.close();

		return SOCKET_ERROR_OK;
	}

	socket_error_t send( char const * const buf, int const buflen )
	{
		auto result = ::send( _socket_fd, buf, buflen, MSG_NOSIGNAL);
		if( result == SOCKET_ERROR )
		{
			return SOCKET_ERROR;
		}

		return SOCKET_ERROR_OK;
	}

	socket_error_t send( std::string const & msg )
	{
		return send( msg.c_str(), msg.size() );
	}

	socket_error_t send( buffer_t const & buf )
	{
		return send( &buf[0], buf.size() );
	}



	socket_error_t recv_append_to_vector( buffer_t & dest )
	{
		if( !is_valid() )
		{
			return SOCKET_ERROR_INVALID_SOCKET;
		}

		int const LEN = 16 * 1024;
		char buf[LEN];

		auto result = ::recv( _socket_fd, buf, LEN, MSG_DONTWAIT );
		if( result == SOCKET_ERROR )
		{
			if( errno == EAGAIN || errno == EWOULDBLOCK )
			{
				return SOCKET_ERROR_OK;
			}

			return SOCKET_ERROR;
		}
		else if( result == 0 )
		{
			return SOCKET_ERROR_DISCONNECT_BY_PEER;
		}

		// Make sure there's enough room in the buffer
		dest.reserve( dest.size() + result + 1 );

		auto src_begin = &buf[0];
		auto src_end = &buf[result];
		dest.insert( dest.end(), src_begin, src_end );

		return SOCKET_ERROR_OK;
	}

 private:
	void replace_socket_fd( int const socket_fd )
	{
		// Close the old socket, if there was one.
		close();

		_socket_fd = socket_fd;
	}
};

} // namespace http
} // namespace cpp0x

#endif // ifndef CPP0X_SOCKET_WRAPPER_H_
