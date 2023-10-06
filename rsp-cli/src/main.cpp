/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>
#include <memory>
#include <boost/asio.hpp>

#include "rsplib/buffer/shared_mutable_buffer.hpp"
#include "rsplib/logger/logger.hpp"
#include "state/state.hpp"

// #include <boost/array.hpp>
int main(int argc, char *argv[]) {
  namespace lg = rsp::libs::logger;

  namespace ip = boost::asio::ip;
  namespace rsp_cli = rsp::cli;
  auto &logger = lg::logger(lg::log_level::kTrace);

  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    logger.debug() << "connect to ... " << argv[1] << lg::L_endl;
    boost::asio::io_context io_context;

    ip::tcp::resolver resolver(io_context);
    ip::tcp::resolver::query query(argv[1], "8080");
    // io::tcp::resolver::results_type endpoints =
    //     resolver.resolve(argv[1], "daytime");

    ip::tcp::socket socket(io_context);
    boost::asio::connect(socket, resolver.resolve(query));

    auto state = std::make_unique<rsp_cli::state::base_state>(&socket);
    for (;;) {
      std::array<char, 128> buf;
      boost::system::error_code error;
      // std::vector<char> buf(22);
      // rsp::libs::buffer::shared_mutable_buffer buffer(buf);
      size_t len = socket.read_some(boost::asio::buffer(buf), error);
      auto next = state->handle_buffer(buf, len);
      if (error == boost::asio::error::eof) {
        logger.debug() << "closed" << lg::L_endl;
        break;
      } else if (error) {
        throw boost::system::system_error(error);
      }

      // state.reset(next);
      // std::cout << buf.data() << std::endl;
      // std::cout.write(buf.data(), len);
    }
  } catch (std::exception &e) {
    logger.error() << e.what() << lg::L_endl;
  }
}
