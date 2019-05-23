#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>

#include <iostream>
#include <chrono>
#include <fstream>
#include <filesystem>

using boost::asio::ip::tcp;
using boost::system::error_code;
using std::filesystem::path;

class session {
public:
  session(tcp::socket& socket_, path file_path_, unsigned int default_buffer_size_);

private:
  void start_session();
  void send_data_chunk();
  tcp::socket& socket;
  /*
    path to uploaded file
  */
  path file_path;
  /*
    vector for file data to send with predefined size
  */
  std::vector<char> data_buffer;
  /*
    handle to file
  */
  std::ifstream file_handle;
  unsigned int file_size;
  unsigned int file_size_left;
  /*
    buffer for communication received from/to server
  */
  boost::asio::streambuf sbuffer;
};

#endif
