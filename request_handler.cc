#include <boost/asio.hpp>
#include <iostream>
#include <string>

#include "request_handler.h"
#include "server_containers.h"
#include "http_constants.h"

using boost::asio::ip::tcp;

// DEFINE HTTP HEADERS:

/* REQUEST HANDLER */
RequestHandler::~RequestHandler() {
  std::cout << "Deconstructing request handler" << std::endl;
}

std::string RequestHandler::build_headers() {
  std::string response_header = "";
  response_header += req->HTTP + " " + resp->status + crlf;
  for (unsigned int i = 0; i < headers.size(); ++i)
  {
    Header& h = headers[i];
    response_header += h.name_;
    response_header += name_value_separator;
    response_header += h.value_;
    response_header += crlf;
  }
  response_header += crlf;
  return response_header; 
}

bool RequestHandler::write_headers(tcp::socket& sock) {
  std::string header = build_headers();
  boost::asio::write(sock, boost::asio::buffer(header, header.size()));
  return true;
}

/* ECHO REQUEST HANDLER */
EchoRequestHandler::~EchoRequestHandler() {
  std::cout << "Deconstructing echo request handler" << std::endl;
}

bool EchoRequestHandler::handle_request() {
  std::cout << "Echo request - handle request" << std::endl;
  resp->status = OK;
  headers.push_back(Header(CONTENT_TYPE, req->mime_type)); 
  headers.push_back(Header(SERVER, resp->server)); 
  // [TODO] Set date  
  return true;
}

bool EchoRequestHandler::write_body(tcp::socket& sock) {
  std::cout << "Echo request - write body" << std::endl;
  boost::asio::write(sock, boost::asio::buffer(req->request, req->request_size));
  return true;
}

/* STATIC REQUEST HANDLER */
StaticRequestHandler::~StaticRequestHandler() {
  std::cout << "Deconstructing static request handler" << std::endl;
}

bool StaticRequestHandler::handle_request() {
  std::cout << "Static request - handle request" << std::endl;
  return true;
}

bool StaticRequestHandler::write_body(tcp::socket& sock) {
  std::cout << "static request - write body" << std::endl;
  return true;
}

