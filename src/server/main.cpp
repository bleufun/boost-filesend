#include <iostream>
#include <filesystem>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include "server.hpp"

namespace po = boost::program_options;

#define DEFAULT_LISTENING_PORT 8888
#define DEFAULT_TIMEOUT 60000
#define DEFAULT_SESSION_BUFFER_SIZE 1048576
#define DEFAULT_DOWNLOAD_DIRECTORY "download-dir"

int main(int argc, char *argv[]) {
  try
    {
      po::options_description desc("Program options");
      desc.add_options()
        ("help", "show this description")
        ("port", po::value<unsigned short int>()->default_value(DEFAULT_LISTENING_PORT), "port on which file server will listen")
        ("timeout", po::value<unsigned int>()->default_value(DEFAULT_TIMEOUT), "inactivity time after which active session will end (in milliseconds)")
        ("buffer-size", po::value<unsigned int>()->default_value(DEFAULT_SESSION_BUFFER_SIZE), "size of session file buffer")
        ("download-dir", po::value<std::string>()->default_value(DEFAULT_DOWNLOAD_DIRECTORY), "download directory")
        ;

      po::variables_map vm;
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);

      if(vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
      }

      std::filesystem::path root_path = vm["download-dir"].as<std::string>();
      if(!std::filesystem::exists(root_path)) {
        std::cout << "error: download folder \""<< root_path.string() << "\" does not exists" << std::endl;
        return -1;
      }

      boost::asio::io_context io_context;

      std::cout << "running on port " << vm["port"].as<unsigned short int>() << std::endl;
      auto endpoint = tcp::endpoint(tcp::v4(), vm["port"].as<unsigned short int>());

      server s
        (
         io_context,
         endpoint,
         //vm["download-dir"].as<std::string>(),
	 root_path,
         std::chrono::milliseconds(vm["timeout"].as<unsigned int>()),
         vm["buffer-size"].as<unsigned int>()
         );

      io_context.run();
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
    }

  return 0;
}
