#define TEST_REQUEST_HANDLER

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "gtest/gtest.h"
#include "server_containers.h"
#include "server.h"
#include "request_handler.h"
#include "http_constants.h"

using boost::asio::ip::tcp;

// Empty Request should return false 
TEST(EchoRequestHandlerTest, HandleEmptyRequest)
{
	char* request_buffer = "";
	std::size_t request_buffer_size = 0;

	ParsedRequest parsed_request(request_buffer, request_buffer_size);
	std::map<std::string, std::string> uri_map;
	Response server_response;
	EchoRequestHandler echo_request_handler(&parsed_request, uri_map, &server_response);
	ASSERT_FALSE(echo_request_handler.handle_request());
}

// Echo Request should return true
TEST(EchoRequestHandlerTest, HandleEchoRequest)
{
	char* request_buffer = "";
	std::size_t request_buffer_size = 0;

	ParsedRequest parsed_request(request_buffer, request_buffer_size);
	std::map<std::string, std::string> uri_map;
	Response server_response;
	parsed_request.URI = ECHO_REQUEST;
	EchoRequestHandler echo_request_handler(&parsed_request, uri_map, &server_response);
	ASSERT_TRUE(echo_request_handler.handle_request());
}

// TEST build_headers

// Static Request with Bad Path should return false
TEST(StaticRequestHandlerTest, HandleBadPath1)
{
	char* request_buffer = "";
	std::size_t request_buffer_size = 0;

	ParsedRequest parsed_request(request_buffer, request_buffer_size);
	std::map<std::string, std::string> uri_map;
	Response server_response;
	server_response.server = "my_server";
	StaticRequestHandler static_request_handler(&parsed_request, uri_map, &server_response);
	ASSERT_FALSE(static_request_handler.handle_request())
	<< "File does not exist";
}

// Static Request with Bad Path should return false
TEST(StaticRequestHandlerTest, HandleBadPath2)
{
	char* request_buffer = "";
	std::size_t request_buffer_size = 0;

	ParsedRequest parsed_request(request_buffer, request_buffer_size);
	std::map<std::string, std::string> uri_map;
	Response server_response;
	server_response.server = "my_server";
	StaticRequestHandler static_request_handler(&parsed_request, uri_map, &server_response);
	static_request_handler.handle_request();
	ASSERT_EQ(server_response.status, NOT_FOUND)
	<< "Status set to not found";
}

// Build a header using write headers and compare to expected string
TEST(HeaderTest, BuildHeader)
{
	char* request_buffer = "";
	std::size_t request_buffer_size = 0;
	char* expected_response_buffer = "HTTP/1.1 200 OK\r\nServer: Serve 2.0\r\nContent-Type: text/plain\r\n\r\n";
	ParsedRequest parsed_request(request_buffer, request_buffer_size);
	parsed_request.URI = ECHO_REQUEST;
	parsed_request.HTTP = "HTTP/1.1";
	parsed_request.mime_type = "text/plain";
	parsed_request.print_contents();
	
	std::map<std::string, std::string> uri_map;
	Response server_response;
	EchoRequestHandler echo_request_handler(&parsed_request, uri_map, &server_response);

	echo_request_handler.handle_request();
	std::string response_buffer = echo_request_handler.build_headers();
	ASSERT_EQ(response_buffer, expected_response_buffer);
}