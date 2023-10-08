/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>
#include <memory>

#include <boost/asio.hpp>

#include "rspcli/state/factory.hpp"
#include "rspcli/state/state.hpp"
#include "rsplib/logger/logger.hpp"

// #include <boost/array.hpp>
int main(int argc, char *argv[]) {
  namespace lg = rsp::libs::logger;

  namespace ip = boost::asio::ip;
  namespace rsp_cli = rsp::cli;
  auto &logger = lg::logger(lg::log_level::kInfo);

  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    logger.debug() << "connect to ... " << argv[1] << lg::L_endl;
    boost::asio::io_context io_context;

    ip::tcp::resolver resolver(io_context);
    ip::tcp::resolver::query query(argv[1], "8080");

    ip::tcp::socket socket(io_context);
    boost::asio::connect(socket, resolver.resolve(query));

    auto next = rsp_cli::state::State::kInit;
    for (;;) {
      auto client = rsp_cli::state::factory::create(next, &socket);
      client->init();
      std::array<char, 128> buf;
      boost::system::error_code error;
      // rsp::libs::buffer::shared_mutable_buffer buffer(buf);
      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof) {
        logger.info() << "eof" << lg::L_endl;
        client->close(error);
        break;
      } else if (error) {
        throw boost::system::system_error(error);
      } else {
        next = client->handle_buffer(buf, len);
      }
    }
  } catch (std::exception &e) {
    logger.error() << e.what() << lg::L_endl;
  }
}
