#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <chrono>
#include <fstream>
#include <set>
#include <string>
#include <filesystem>

#include "utils.hpp"

using boost::asio::ip::tcp;
using boost::system::error_code;
using std::filesystem::path;

class session
  : public std::enable_shared_from_this<session> {
public:
  session(
          tcp::socket socket_,
          std::set<std::shared_ptr<session>> &connected_clients_,
          unsigned long long session_id_,
          std::chrono::milliseconds timeout_check_duration_,
          unsigned int default_buffer_size_,
          path &root_path_
          );

  /*
    start listening to client
  */
  void start_session();

private:
  void end_session();
  /*
    read filename and size from stream and prepare file handle
  */
  void read_file_info(error_code ec, size_t bytes_received);
  /*
    write received bytes to created file
  */
  void write_content(error_code ec);

  /*
    close connection if timeout was reached
  */
  void check_timeout(error_code ec);
  /*
    restart timeout timer
  */
  void restart_timeout();

  tcp::socket socket;
  /*
    path to download directory
  */
  path& root_path;
  /*
    (unique) session id
  */
  unsigned long long session_id;

  boost::asio::steady_timer timeout{socket.get_executor().context()};
  /*
    handle to active session list
  */
  std::set<std::shared_ptr<session>>& connected_clients;
  /*
    name of file that is downloaded to server
  */
  std::string file_name;
  /*
    default timeout value
  */
  std::chrono::milliseconds timeout_check_duration;
  /*
    size of bytes left to receive from client
  */
  unsigned long long int file_size_left;
  /*
    handle to downloaded file
  */
  std::ofstream file_handle;
  /*
    path to downloaded file, constructed from root dir and file name
  */
  path file_path;
  /*
    handle to file lock
  */
  std::ofstream file_lock_handle;
  /*
    path to file lock, constructed from file path with added postfix
  */
  path file_lock_path;
  /*
    vector for received file data with predefined size
  */
  std::vector<char> data_buffer;
  /*
    buffer for communication received from/to client
  */
  boost::asio::streambuf sbuffer;
};

#endif
