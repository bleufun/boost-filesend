#include "session.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>

session::session(tcp::socket socket_, std::set<std::shared_ptr<session>> &connected_clients_, unsigned long long session_id_, std::chrono::milliseconds timeout_check_duration_, unsigned int default_buffer_size_, path &root_path_)
  : socket(std::move(socket_)),
    root_path(root_path_),
    connected_clients(connected_clients_),
    session_id(session_id_),
    timeout_check_duration(timeout_check_duration_),
    data_buffer(default_buffer_size_) {
  // constructor
}

void session::start_session() {
  std::cout << get_timestamp() << "id: " << session_id << " starting session" << std::endl;
  timeout.expires_after(timeout_check_duration);

  auto self = shared_from_this();
  timeout.async_wait(boost::bind(&session::check_timeout, self, _1));

  boost::asio::async_read_until(socket, sbuffer, "\r\n\r\n", boost::bind(&session::read_file_info, shared_from_this(), _1, _2));
}

void session::end_session() {
  file_lock_handle.close();

  timeout.cancel();

  socket.shutdown(tcp::socket::shutdown_both);
  socket.close();
  std::cout << get_timestamp() << "id: " << session_id << " closed connected socket" << std::endl;

  auto self(shared_from_this());
  connected_clients.erase(self);
}

void session::read_file_info(error_code ec, size_t /*bytes_received*/) {
  if(ec) {

  }

  std::istream response_stream(&sbuffer);
  std::string hello_msg;
  response_stream >> hello_msg;
  if(hello_msg == "hello") {
    restart_timeout();

    response_stream >> file_name;
    response_stream >> file_size_left;

    if(file_size_left <= 0) {
      std::cout << get_timestamp() << "id: " << session_id << " file size error" << std::endl;
      end_session();
    }

    file_path /= root_path;
    file_path /= file_name;
    file_lock_path = file_path;
    file_lock_path += ".lock";

    std::cout << get_timestamp() << "id: " << session_id << " file " << file_name << ":size " << file_size_left << " info received, preparing to transfer" << std::endl;
    // lock file exists
    if(std::filesystem::exists(file_lock_path)) {
      std::cout << get_timestamp() << "fail: cannot write to " << file_name << ", file locked" << std::endl;
      std::string msg_fail_file_busy("fail_file_busy");
      auto self = shared_from_this();
      // cannot write to file, close connection
      // TODO: check if client risks getting connection closed before receiving error message
      boost::asio::async_write
        (socket, boost::asio::buffer(msg_fail_file_busy),
         [this, self](error_code ec, std::size_t) {
           if(ec)
             std::cout << get_timestamp() << "error code: " << ec << std::endl;
           file_lock_handle.close();
           end_session();
         });

      return;
    }

    file_lock_handle.open(file_lock_path);
    file_lock_handle.flush();

    file_handle.open(file_path, std::ios::binary);

    const std::string ok_msg{"ok\r\n\r\n"};
    auto self = shared_from_this();
    boost::asio::async_write
      (socket, boost::asio::buffer(ok_msg),
       [this, self](error_code ec, std::size_t) {
         write_content(ec);
       });
  }
  else {
    std::cout << get_timestamp() << "wrong incoming message, aborting" << std::endl;
    end_session();
  }
}

void session::write_content(error_code ec) {
  if(ec) {
    std::filesystem::remove(file_lock_path);
    return;
  }

  auto self = shared_from_this();
  socket.async_read_some
    (boost::asio::buffer(data_buffer),
     [this, self](error_code ec, std::size_t bytes_received) {
       if(!ec) {
         if(bytes_received > 0 && bytes_received <= file_size_left) {
           //std::cout << get_timestamp() << "id: " << session_id << " got " << bytes_received << " bytes, " << file_size_left << " bytes left" <<std::endl;
           restart_timeout();
           file_size_left -= bytes_received;

           file_handle.write(data_buffer.data(), bytes_received);
           file_handle.flush();

           // TODO refactor
           if(file_size_left == 0) {
             file_handle.close();
             std::filesystem::remove(file_lock_path);
             std::cout << get_timestamp() << "id: " << session_id << " downloaded successfully" <<std::endl;
             end_session();
           }

           write_content(ec);
         }
         else if(bytes_received > file_size_left) {
           // close connection
           file_handle.close();
           std::filesystem::remove(file_lock_path);
           end_session();
         }
       }
     });
}

void session::check_timeout(error_code ec) {
  if(timeout.expiry() <= std::chrono::steady_clock::now()) {
    std::cout << get_timestamp() << "id: " << session_id << " session timeout" << std::endl;
    std::filesystem::remove(file_lock_path);
    end_session();
  }
}

void session::restart_timeout() {
  if(timeout.expires_after(timeout_check_duration) > 0) {
    auto self = shared_from_this();
    timeout.async_wait(boost::bind(&session::check_timeout, self, _1));
  }
  else {
    std::filesystem::remove(file_lock_path);
    end_session();
  }
}
