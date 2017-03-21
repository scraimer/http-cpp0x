#include "http-cppx0.hpp"

#include <iostream>
#include <iterator>
#include <map>

#include "pages/index.html.h"
#include "pages/main.js.h"
#include "pages/style.css.h"

using namespace cppx0::http;
using std::cout;
using std::endl;

struct pages_t
{
	struct page_t
	{
		std::string _name;
		unsigned char const * _content;
		unsigned int _content_len;

		page_t() : _name( "" ), _content( nullptr ), _content_len( -1 )
		{
		}

		page_t(page_t const & other )
		{
			_name = other._name;
			_content = other._content;
			_content_len = other._content_len;
		}
	};

	std::map<std::string, page_t> _pages;

	pages_t()
	{
		page_t page;

		page._name = "/";
		page._content = pages_index_html;
		page._content_len = pages_index_html_len;
		_pages[page._name] = page;

		page._name = "/main.js";
		page._content = pages_main_js;
		page._content_len = pages_main_js_len;;
		_pages[page._name] = page;

		page._name = "/style.css";
		page._content = pages_style_css;
		page._content_len = pages_style_css_len;
		_pages[page._name] = page;
	}

	bool get_page(std::string const page_name, page_t & page )
	{
		auto it = _pages.find( page_name );
		if( it != _pages.end() )
		{
			page = it->second;
			return true;
		}

		return false;
	};
};

class my_request_handler_t
{
 private:
	pages_t _pages;

 public:
	void operator()( request_t const & request )
	{
		cout << "\nGot a request!" << endl;
		if( request._uri._raw.size() )
		{
			cout << "URI: \"" << request._uri._raw << "\"" << endl;
			cout << "  path = \"" << request._uri._path << "\"" << endl;
		}
		cout << "Content:" << endl;
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

		std::string query = request._uri._query;

		char const raw_header_404[] = "HTTP/1.0 404 Not Found\r\n\r\n";
		buffer_t header_404;
		header_404.insert( header_404.begin(), &raw_header_404[0], &raw_header_404[sizeof(raw_header_404) - 1]);

		char const raw_header[] = "HTTP/1.0 200 OK\r\n\r\n";
		buffer_t header_ok;
		header_ok.insert( header_ok.begin(), &raw_header[0], &raw_header[sizeof(raw_header) - 1]);

		pages_t::page_t page;
		auto result = _pages.get_page( request._uri._path, page );
		if( result )
		{
			request.send_to_client( header_ok );
			buffer_t page_buf;
			page_buf.insert( page_buf.begin(), &page._content[0], &page._content[page._content_len - 1] );
			request.send_to_client( page_buf );
		}
		else if( request._uri._path == "/a" )
		{
			// Special case: We've been asked to do an action.
			request.send_to_client( header_ok );
			buffer_t page_buf;
			std::string output("Got this query string:<br/>\n");
			output += request._uri._query;
			page_buf.insert( page_buf.begin(), output.begin(), output.end() );
			request.send_to_client( page_buf );
		}
		else
		{
			request.send_to_client( header_404 );
		}
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

