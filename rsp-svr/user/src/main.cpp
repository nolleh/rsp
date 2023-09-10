#include <boost/asio.hpp>
#include <iostream>

#include "config.h"
#include "server/tcp_server.hpp"
#include "test.hpp"
#include "utils/logger.hpp"

int main() {
  std::cout << "User Version: " << User_VERSION_MAJOR << User_VERSION_MINOR
            << std::endl;

  namespace libs = rsp::libs;
  try {
    std::cout << func(1, 2) << std::endl;

    boost::asio::io_context io_context;
    libs::server::tcp_server server(&io_context);
    // TODO(@nolleh) thread pool
    io_context.run();
  } catch (std::exception &e) {
    libs::utils::logger::get_instance().error("exception ocurred");
  }
}
