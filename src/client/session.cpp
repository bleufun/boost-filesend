#include "session.hpp"

session::session(tcp::socket &socket_, path file_path_, unsigned int default_buffer_size_)
  : socket(socket_),
    file_path(file_path_),
    data_buffer(default_buffer_size_) {

  start_session();
}

void session::start_session() {
  std::iostream command_stream(&sbuffer);

  std::cout << "sending metadata..." << std::endl;

  command_stream << "hello" << "\r\n";
  command_stream << file_path.filename().string() << "\r\n";
  file_size = std::filesystem::file_size(file_path);
  file_size_left = file_size;
  command_stream << file_size << "\r\n";
  command_stream << "\r\n";

  std::cout << "hang in there" << std::endl;
  boost::asio::write(socket, sbuffer);

  std::cout << "metadata sent, waiting for response" << std::endl;
  boost::asio::read_until(socket, sbuffer, "\r\n\r\n");

  std::string response;
  command_stream >> response;

  if(response == "ok") {
    std::cout << "metadata received, starting sending file data..." << std::endl;

    file_handle.open(file_path);
    send_data_chunk();
  }
  else {
    std::cout << "file send error: bad incoming message" << std::endl;
    return ;
  }
}

void session::send_data_chunk() {
  //std::chrono::steady_timer timer;
  std::chrono::time_point<std::chrono::steady_clock> begin, end;
  begin = std::chrono::steady_clock::now();
  while(file_size_left > 0) {
    file_handle.read(data_buffer.data(), data_buffer.size());
    unsigned int bytes_extracted = file_handle.gcount();
    end = std::chrono::steady_clock::now();
    if(std::chrono::duration_cast<std::chrono::seconds>(end - begin) > std::chrono::seconds(1)) {
      begin = std::chrono::steady_clock::now();
      std::cout << file_size - file_size_left << "/" << file_size << " bytes sent" << "\r";
      std::cout.flush();
    }
    boost::asio::write(socket, boost::asio::buffer(data_buffer, bytes_extracted));
    file_size_left -= bytes_extracted;
  }
  std::cout << std::endl;

  file_handle.close();
  socket.close();

  // TODO add confirmation message
  std::cout << "file sent successfully" << std::endl;
}
