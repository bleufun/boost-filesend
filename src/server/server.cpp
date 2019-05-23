#include "server.hpp"

server::server(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint &endpoint, path root_path_, std::chrono::milliseconds timeout_check_duration_, unsigned int default_buffer_size_)
  : acceptor(io_context, endpoint),
    server_timer(io_context),
    root_path(root_path_),
    timeout_check_duration(timeout_check_duration_),
    default_buffer_size(default_buffer_size_){

  /*
    start accepting connections
  */
  accept_connection();
}

void server::accept_connection() {
  acceptor.async_accept
    ([this](boost::system::error_code ec, tcp::socket socket) {
       if(!ec) {
         std::shared_ptr<session> client_session = std::make_shared<session>(std::move(socket), connected_clients, free_session_id, timeout_check_duration, default_buffer_size, root_path);
         free_session_id++;
         connected_clients.insert(client_session);
         client_session->start_session();

         /*
           ready for receiving new connections
         */
         accept_connection();
       }
     });
}
