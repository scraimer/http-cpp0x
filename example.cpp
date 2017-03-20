#include "http-cppx0.hpp"

#include <iostream>
#include <iterator>

using namespace cppx0::http;
using std::cout;
using std::endl;

class my_request_handler_t
{
 public:
	void operator()( request_t const & request )
	{
		cout << "Got a request! Content:" << endl;
		auto& buf = request.get_buffer();
		cout << "\t";
		for( auto it = buf.cbegin(); it != buf.cend(); ++it )
		{
			auto& c = *it;
			cout << *it;
			if( c == '\n' )
			{
				cout << "\t";
			}
		}

		request.send_to_client( buf );
	}
};

int main( int const argc, char const * const * const argv )
{
	server_config_t cfg;
	cfg._port = 47890;

	my_request_handler_t request_handler;
	server_t<decltype( request_handler )> server( request_handler );
	if( server.init( cfg ) )
	{
		cout << "Error configuring server!" << endl;
		return 1;
	}

	cout << "Listen socket ready on port " << cfg._port << endl;

	while( true )
	{
		// This tells the HTTP server to use some CPU time, since it has no thread 
		// of it's own. This takes care of:
		//   1. Accepting incoming connections
		//   2. Reading requests and calling the request_handler
		//   
		server.work();
	}

	return 0;
}

