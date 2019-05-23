#include <iostream>
#include <filesystem>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include "session.hpp"

namespace po = boost::program_options;
using boost::asio::ip::tcp;

#define DEFAULT_LISTENING_PORT "8888"
#define DEFAULT_SESSION_BUFFER_SIZE 1048576

int main(int argc, char *argv[]) {
  try
    {
      po::options_description desc("Program options");
      desc.add_options()
        ("help", "show this description")
        ("port", po::value<std::string>()->default_value(DEFAULT_LISTENING_PORT), "port on which file server will listen")
        ("buffer-size", po::value<unsigned int>()->default_value(DEFAULT_SESSION_BUFFER_SIZE), "size of session file buffer")
        ("address", po::value<std::string>(), "IPv4 address")
        ("file", po::value<std::string>(), "file to download")
        ;

      po::positional_options_description pos;
      pos.add("file", -1);

      po::variables_map vm;
      po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
      po::notify(vm);

      if(vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
      }

      std::filesystem::path file_path;
      try {
        file_path = vm["file"].as<std::string>();
      }
      catch(boost::bad_any_cast &e) {
        std::cout << "error: you must describe a path to a file" << std::endl;
        return -1;
      }

      if(!std::filesystem::exists(file_path)) {
        std::cout << "error: the file does not exists" << std::endl;
        return -1;
      }

      if(!std::filesystem::is_regular_file(file_path)) {
        std::cout << "error: described path is not a file" << std::endl;
        return -1;
      }

      boost::asio::io_context io_context;
      tcp::socket socket(io_context);
      boost::asio::ip::tcp::resolver ip_resolver(io_context);

      std::string ipv4_address;
      try {
        ipv4_address = vm["address"].as<std::string>();
      }
      catch(boost::bad_any_cast &e) {
        std::cout << "error: you must describe a correct IPv4 address" << std::endl;
        return -1;
      }

      boost::asio::connect(socket, ip_resolver.resolve(ipv4_address, vm["port"].as<std::string>()));

      try {
        session s(socket, file_path, vm["buffer-size"].as<unsigned int>());
      }
      catch(std::exception &e) {
        std::cout << "connection refused" << std::endl;
        return -1;
      }

      io_context.run();
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
    }

  return 0;
}
