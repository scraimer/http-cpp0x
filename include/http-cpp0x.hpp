#ifndef CPP0X_HTTP_SERVER_H_
#define CPP0X_HTTP_SERVER_H_

#include "socket-wrapper.hpp"
#include <assert.h>
#include <sys/select.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <utility>

namespace cpp0x
{
namespace http
{

static std::pair<std::string, std::string> make_string_pair_from_offsets( std::string const & src,
                                                                   int const key_start, int const key_end,
                                                                   int const value_start,
                                                                   int const value_end )
{
	std::string key = src.substr( key_start, key_end - key_start + 1 );
	std::string val = src.substr( value_start, value_end - value_start );
	return std::make_pair( key, val );
}

/** Iterate on a URI query string and call a function with an std::pair<std::string,std::string> where the
 * first string is the key, and the second is the value. 
 * 
 * Credit: Inspired by a code sample in 
 * 	"C++ Cookbook" by Jeff Cogswell, Jonathan Turkanis, Christopher Diggins, D. Ryan Stephens
 * */
template <class UnaryFunction, char PAIRS_DELIM = '&', char VALUE_DELIM = '='>
void split_key_values_into_pairs( std::string const & src, UnaryFunction f )
{
	auto key_start = 0;
	auto val_end = src.find( PAIRS_DELIM );

	while( val_end != std::string::npos )
	{
      {
		   // If there's no delimeter between the key and the value, ignore it
		   auto inner_delim_pos = src.find( VALUE_DELIM, key_start );
		   if( inner_delim_pos == std::string::npos )
		   {
			   continue;
		   }

		   int key_end = inner_delim_pos - 1;
		   int val_start = inner_delim_pos + 1;

		   auto pair = make_string_pair_from_offsets( src, key_start, key_end, val_start, val_end );

		   f( pair );
      }

		key_start = ++val_end;
		val_end = src.find( PAIRS_DELIM, val_end );

		// Handle the last one, which won't have another PAIRS_DELIM after it
		if( val_end == std::string::npos )
		{
			// If there's no delimeter between the key and the value, ignore it
			auto inner_delim_pos = src.find( VALUE_DELIM, key_start );
			if( inner_delim_pos == std::string::npos )
			{
				continue;
			}

			int key_end = inner_delim_pos - 1;
			int val_start = inner_delim_pos + 1;

			auto pair = make_string_pair_from_offsets( src, key_start, key_end, val_start, src.length() );

			f( pair );
		}
	}
}

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

			split_key_values_into_pairs(
			    _query, add_item_to_vector<std::pair<std::string, std::string> >( _query_pairs ) );
		}

	 private:
		template <typename T>
		class add_item_to_vector
		{
		 public:
			add_item_to_vector( std::vector<T> & dest ) : _dest( dest )
			{
			}

			void operator()( T const & item )
			{
				_dest.push_back( item );
			}

		 private:
			std::vector<T> & _dest;
		};
	};

 private:
	buffer_t _buffer;
	client_t & _client;

 public:
	uri_t _uri;
   buffer_t::const_iterator _body_iterator;

	request_t( buffer_t const & buffer, client_t & client )
	    : _buffer( buffer ), _client( client ), _body_iterator( _buffer.end() )
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
   
   bool has_body() const
   {
      return _body_iterator != _buffer.cend();
   }
   
   buffer_t::const_iterator body_cbegin() const
   {
      return _body_iterator;
   }
   
   buffer_t::const_iterator body_cend() const
   {
      return _buffer.cend();
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
   
   static bool starts_with( buffer_t const & haystack, std::string needle )
	{
		auto const len = needle.size();
      // Why len*2? Because the std::search algorithm only looks from up to needle.size() from the end.
		auto pos = std::search( haystack.cbegin(), haystack.cbegin() + len * 2, needle.cbegin(), needle.cend() );
		return ( pos == haystack.cbegin() );
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
		else if( result != SOCKET_ERROR_OK )
		{
			return;
		}

		// if got an entire HTTP header, notify the user that a request has arrived.
		char const end_of_header[5] = "\r\n\r\n";
		auto end_of_header_pos = std::search( buf.cbegin(), buf.cend(), &end_of_header[0], &end_of_header[4] );
		if( end_of_header_pos == buf.cend() )
		{
			return;
		}

		// Use the Content-Length header to determine if we have finished reading
		// the entire message, if it's present
		char const content_length[] = "Content-Length: ";
		auto len_pos = std::search( buf.cbegin(), buf.cend(), &content_length[0],
		                            &content_length[sizeof( content_length ) - 1] );
		if( len_pos != buf.cend () )
		{
			auto len_end_pos = std::find( len_pos, buf.cend(), '\r' );
			if( len_end_pos == buf.cend() )
			{
				assert( false && "Error finding end of Content-Length" );
				return;
			}

			std::string len_string( len_pos + sizeof(content_length) - 1, len_end_pos );
			int len = std::stoi( len_string );
			int content_body_received =
			    std::distance( end_of_header_pos + sizeof( end_of_header ) - 1, buf.cend() );

			// Is there still more that needs to be read from the socket?
			if( content_body_received < len )
			{
				// Don't produce a request, wait for more incoming data (i.e. wait for next time)
				return;
			}
		}

		// Got a valid request, so let's prepare it before sending it to the handler
		request_t req( buf, client );

		// Which method is it?
		char const method_get[] = "GET ";
		char const method_post[] = "POST ";
      auto uri_begin = buf.cend();
		if( starts_with( buf, method_get ) )
		{
			uri_begin = buf.cbegin() + sizeof( method_get ) - 1;
		}
		else if( starts_with( buf, method_post ) )
		{
			uri_begin = buf.cbegin() + sizeof( method_post ) - 1;
		}
		else
		{
			return;
		}

		// Find the URI
		auto uri_end = std::find( uri_begin, buf.cend(), ' ' );
		if( uri_end != buf.cend() )
		{
			std::string raw_uri;
			// Note: It ought to be raw_uri.cbegin(), but GCC 4.8.5 chokes on that
			raw_uri.insert( raw_uri.begin(), uri_begin, uri_end );
			req._uri.set( raw_uri );
		}

		// Find the body
		auto body_pos = end_of_header_pos + sizeof( end_of_header ) - 1;
		if( body_pos != buf.cend() )
		{
			// The request object has its own copy of the buffer, so to initialize the iterator that points to
			// the body inside that buffer, we must create a parallel iterator for that copy of the buffer.
			req._body_iterator = req.get_buffer().cbegin() + std::distance( buf.cbegin(), body_pos );
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
} // namespace cpp0x

#endif // ifndef CPP0X_HTTP_SERVER_H_
