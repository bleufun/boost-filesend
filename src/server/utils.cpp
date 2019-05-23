#include "utils.hpp"

std::string get_timestamp() {
  std::stringstream ss;
  auto log_time_point = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

  std::time_t log_time_c = std::chrono::system_clock::to_time_t(log_time_point);
  ss << "[" << std::put_time(std::localtime(&log_time_c), "%Y-%b-%d %H:%M:%S") << "]: ";
  return ss.str();
}
