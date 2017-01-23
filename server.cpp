// Team: spaceteam
// Members: Crystal Hsieh, Ryan Voong, Jason Liu
//
// Citation: Based on tutorial for async TCP daytime server
// http://www.boost.org/doc/libs/1_63_0/doc/html/boost_asio/tutorial.html#boost_asio.tutorial.tutdaytime3

#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;



namespace {
  const int PORT = 8118;
}

// shared_ptr and enable_shared_from_this keeps the connection alive
// as long as there's an operation referring to it.
class connection
    :  public boost::enable_shared_from_this<connection> {
  public:
  	typedef boost::shared_ptr<connection> pointer;

  	static pointer create(boost::asio::io_service& io_service) {
  	  return pointer(new connection(io_service));
  	}

  	tcp::socket& socket() {
  	  return socket_;
  	}

    
  	void start() {
  	  // data meant to be sent is stored in message_
   	  message_ = "HTTP-Version: HTTP/1.0 200 OK\nContent-Type: text/plain";
   	             // Later we will need to add the HTTP request

      // async_write() serves data to the client
      boost::asio::async_write(socket_, boost::asio::buffer(message_),
      	  boost::bind(&connection::handle_write, shared_from_this(),
      	  	  boost::asio::placeholders::error,
      	  	  boost::asio::placeholders::bytes_transferred));
  	}

  private:
  	connection(boost::asio::io_service& io_service) : socket_(io_service) {}

  	void handle_write(const boost::system::error_code& error,
  	    size_t bytes_transferred) {}

    tcp::socket socket_;
    std::string message_;
};

class server {
  public:
  	server (boost::asio::io_service& io_service)
  	    : acceptor_(io_service, tcp::endpoint(tcp::v6(), PORT)) {
  	  start_accept();
  	}

  private:
  	// creates a socket and initiates asynchronous accept operation
  	// to wait for a new connection
    void start_accept() {
      connection::pointer new_connection =
          connection::create(acceptor_.get_io_service());

      acceptor_.async_accept(new_connection->socket(),
      	boost::bind(&server::handle_accept, this, new_connection,
      	  boost::asio::placeholders::error));
    }

    void handle_accept(connection::pointer new_connection,
        const boost::system::error_code& error) {
      if (!error) {
      	new_connection->start();
      }

      start_accept();
    }

    tcp::acceptor acceptor_;
};


// Main function to run server
int main() {
  // Creates server object to accept incoming client connections
  try {
    boost::asio::io_service io_service;
    server server(io_service);
    io_service.run();
  } catch (std::exception& e) {
  	std::cerr << e.what() << std::endl;
  }

  return 0;
}