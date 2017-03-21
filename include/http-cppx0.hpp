#ifndef CPPX0_HTTP_SERVER_H_
#define CPPX0_HTTP_SERVER_H_

#include "socket-wrapper.hpp"
#include <assert.h>
#include <sys/select.h>
#include <algorithm>
#include <string>

namespace cppx0
{
namespace http
{

// Forward declaration, see at the end of the file.
class internal_tcp_socket_wrapper_t;

class client_t : public internal_tcp_socket_wrapper_t
{
 public:
	buffer_t _buf;

	void reset()
	{
		_buf.clear();
	}
};

class request_t
{
 public:
	struct uri_t
	{
		std::string _raw;
		std::string _path;
		std::string _query;
		std::vector<std::pair<std::string, std::string> > _query_pairs;

		uri_t() : _raw( "" ), _path( "" ), _query( "" )
		{
		}

		void set( std::string const & uri ) 
		{
			_raw = uri;

			auto query_start = uri.find( '?' );
			if( query_start != std::string::npos )
			{
				_path = uri.substr( 0, query_start );
				_query = uri.substr( query_start + 1 );
			}
			else
			{
				_path = uri;
			}

			// TODO: parse the _query_pairs from _query
		}
	};

 private:
	buffer_t _buffer;
	client_t & _client;

 public:
	uri_t _uri;

	request_t( buffer_t const & buffer, client_t & client )
	    : _buffer( buffer ), _client( client )
	{
	}

	buffer_t const & get_buffer() const
	{
		return _buffer;
	}

	bool send_to_client( buffer_t const & buffer ) const
	{
		auto result = _client.send( buffer );
		return ( result == SOCKET_ERROR_OK );
	}
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

	request_handler_t & _request_handler;
	internal_tcp_socket_wrapper_t _server_socket;

	class clients_set_t;
	clients_set_t _all_clients;

 public:
	server_t( request_handler_t & request_handler )
	    : _request_handler( request_handler )
	    , _all_clients()
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

		struct timeval select_timeout;
		select_timeout.tv_sec = 0;
		select_timeout.tv_usec = 100 * 1000;

		auto fds = fds_to_poll._fds;
		int rc = select( fds_to_poll._largest + 1, &fds, nullptr, nullptr, &select_timeout );
		if( rc < 0 )
		{
			return ERROR_POLLING_SOCKETS;
		}

		if( rc == 0 )
		{
			return OK;
		}

		// The order is important: first the clients, and then the server socket.
		// That way there's no race between adding new clients and reading from them before they are ready.
		// (It's not a real race, I just can't be bothered to try and figure out if there's an issue.)
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
		auto it = _all_clients.get_next_available();
		if( it == _all_clients.end() )
		{
			// Error: Cannot accept another client. So turn away the client with an error.

			std::string err_msg_out_of_resource =
			    "HTTP/1.0 429 Too Many Requests\r\n\r\n"
			    "Sorry, the maximum number of clients has been reached. Please try again later.\r\n";
			_server_socket.reject_incoming_connection( err_msg_out_of_resource );
			return;
		}

		auto & new_client = *it;
		new_client.reset();
		_server_socket.accept_into( new_client );
	}

	void service_client( client_t & client )
	{
		buffer_t & buf = client._buf;
		auto result = client.recv_append_to_vector( buf );
		if( result == SOCKET_ERROR_DISCONNECT_BY_PEER )
		{
			client.close();
			return;
		}
		else if( result != SOCKET_ERROR_OK)
		{
			return;
		}

		// if got an entire HTTP header, notify the user that a request has arrived.
		char const end_of_request[5] = "\r\n\r\n";
		auto pos = std::search( buf.cbegin(), buf.cend(), &end_of_request[0], &end_of_request[4] );
		if( pos == buf.cend() )
		{
			return;
		}

		// Got a valid request, so let's prepare it before sending it to the handler
		request_t req( buf, client );

		// Find the URI
		char const uri_prefix[] = "GET ";
		auto uri_pos =
		    std::search( buf.cbegin(), buf.cend(), &uri_prefix[0], &uri_prefix[sizeof( uri_prefix ) - 1] );
		uri_pos += sizeof( uri_prefix ) - 1;
		if( uri_pos != buf.cend() )
		{
			auto uri_end = std::find( uri_pos, buf.cend(), ' ' );
			if( uri_end != buf.cend() )
			{
				std::string raw_uri;
				raw_uri.insert( raw_uri.cbegin(), uri_pos, uri_end );
				req._uri.set( raw_uri );
			}
		}

		_request_handler( req );

		// After having serviced the request, close the client
		client.close();
	}

	/** A container for all the clients belonging to the server */
	class clients_set_t
	{
	 public:
		typedef client_t item_t;

	 private:
		item_t _client_sockets[MAX_CLIENTS];
		item_t * _begin;
		item_t * const _end;

	 public:
		clients_set_t() : _begin( &_client_sockets[0] ), _end( &_client_sockets[MAX_CLIENTS] )
		{
		}

		item_t * cbegin() const
		{
			return _begin;
		}

		item_t * begin()
		{
			return _begin;
		}
		item_t * end() const
		{
			return _end;
		}

		class is_not_valid_predicate
		{
		 public:
			bool operator()( item_t & item ) const
			{
				return !item.is_valid();
			}
		};

		item_t * get_next_available() const
		{
			auto found_it = std::find_if( cbegin(), end(), is_not_valid_predicate() );
			return found_it;
		}
	};

	/** UnaryPredicate that checks if a socket wrapped by client_t is in an fd_set. Use
	 * after calling select(), eh? */
	class service_readable_client
	{
	 private:
		fd_set _fds;
		server_t & _server;

	 public:
		service_readable_client( fd_set const & fds, server_t & server ) : _fds( fds ), _server( server )
		{
		}

		void operator()( client_t & sock_wrapper )
		{
			if( FD_ISSET( sock_wrapper.get_socket_fd(), &_fds ) )
			{
				_server.service_client( sock_wrapper );
			}
		}
	};

	/** Builds a set of file descriptors for handing to select(). This is two things:
	 *
	 *  * The largest file descriptor
	 *  * The fd_set of file descriptors
	 *
	 * To use, call it when iterating on a set of internal_tcp_socket_wrapper_t items. When done, get the
	 * values out of the members.
	 *
	 */
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
