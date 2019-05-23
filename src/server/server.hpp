#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <chrono>
#include <set>

#include "utils.hpp"
#include "session.hpp"

using boost::asio::steady_timer;
using boost::asio::ip::tcp;
using std::filesystem::path;

class server {
public:
  server(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint& endpoint, path root_path, std::chrono::milliseconds timeout_check_duration_, unsigned int default_buffer_size_);

  void accept_connection();

private:
  steady_timer server_timer;
  tcp::acceptor acceptor;
  /*
    active session list
  */
  std::set<std::shared_ptr<session>> connected_clients;
  /*
    root path to download directory
  */
  path root_path;
  /*
    default timeout
  */
  std::chrono::milliseconds timeout_check_duration;
  /*
    free id for newly created sessions
  */
  unsigned long long free_session_id = 1;
  /*
    default buffer size for newly created sessions
  */
  unsigned int default_buffer_size;
};

#endif
