#include <iostream>
#include <boost/bind.hpp>
#include "connection.h"

using boost::asio::ip::tcp;

Connection::~Connection() {
  socket_.close();
}

Connection::pointer Connection::create(boost::asio::io_service& io_service) {
  return pointer(new Connection(io_service));
}

tcp::socket& Connection::socket() {
  return socket_;
}

void Connection::start() {
  do_read();
}

void Connection::do_read() {
  socket_.async_read_some(
      boost::asio::buffer(data_, max_request_length_),
      boost::bind(
          &Connection::handle_read,
          shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

bool Connection::handle_read(const boost::system::error_code& error, 
                             std::size_t /*bytes_transferred*/) {
  if (error) {
    return false; // error
  }

  // Since there's no error, we can parse the data.
  // Global RequestParser will insert data into cur_request
  Request cur_request;
  request_parser.parse(cur_request, data_, data_+bytes_transferred);

  do_write();
  return true; // success
}

void Connection::do_write() {
  char response[max_response_length_] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
  strcat(response, data_);
  memset(data_, 0, max_request_length_);

  boost::asio::async_write(
      socket_,
      boost::asio::buffer(response, strlen(response)),
      boost::bind(
          &Connection::handle_write, 
          shared_from_this(), 
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

bool Connection::handle_write(const boost::system::error_code& error,
                              std::size_t /*bytes_transferred*/) {
  if (error) {
    return false; // error
  }

  return true; // success
}
