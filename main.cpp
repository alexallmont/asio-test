#include <boost/asio.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <regex>
#include <thread>

//! Experimental boost asio code that continually searches for USB serial port to
//! read RPM info in an Arduino project. When the serial port is found it reads data
//! whilst it can, then attempts to reconnect.
int main()
{
  using namespace boost::asio;

  io_service io;

  std::atomic_bool running = true;
  std::thread t{
    [&] {
      const std::string path{"/dev"};
      const std::regex arduino_serial_prefix{"^cu\\.usbmodem"};

      while (running) {
        std::optional<std::string> arduino_serial_dev;
        for (const auto & entry : std::filesystem::directory_iterator(path)) {
          const std::string devname{entry.path().filename().string()};
          if (std::regex_search(devname, arduino_serial_prefix)) {
            arduino_serial_dev = entry.path().string();
            break;
          }
        }

        if (!arduino_serial_dev) {
          std::cout << "Could not find serial input" << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds{1});
          continue;
        }

        std::cout << "Serial found on port " << *arduino_serial_dev << std::endl;
        serial_port serial(io, arduino_serial_dev->c_str());
        serial.set_option(serial_port_base::baud_rate(115200));

        std::string result;
        while (running) {
          char chr;
          try {
            read(serial, buffer(&chr, 1));
          } catch (boost::system::system_error& err) {
            std::cout << "Disconnect detected" << std::endl;
            break;
          }
          if (chr == '\n') {
            std::cout << result;
            std::cout << '\n';
            result.clear();
          } else if (chr == '\r') {
            // ignore
          } else {
            result += chr;
          }
        }
      }

      std::cout << "Stopped serial read" << std::endl;
    }
  };

  for (size_t i = 0, n = 20; i < n; ++i) {
    std::cout << "Running for " << i + 1 << " of " << n << " seconds" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds{1});
  }

  running = false;
  t.join();

  return 0;
}
