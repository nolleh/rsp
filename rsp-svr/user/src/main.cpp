#include <boost/asio.hpp>
#include <iostream>

#include "config.h"
#include "server/tcp_server.hpp"
#include "test.hpp"
#include "thread/thread_pool.hpp"
#include "utils/logger.hpp"
#include "session/session_manager.hpp"

int main() {
  std::cout << "User Version: " << User_VERSION_MAJOR << User_VERSION_MINOR
            << std::endl;

  namespace server = rsp::libs::server;
  namespace utils = rsp::libs::utils;
  namespace session = rsp::user::session;
  try {
    std::cout << func(1, 2) << std::endl;

    server::tcp_server server;
    auto& manager = session::session_manager::instance();    
    server.subscribe(static_cast<server::server_event*>(&manager));
    server.start();

  } catch (std::exception &e) {
    utils::logger::instance().error("exception ocurred" +
                                    std::string(e.what()));
  }
}
