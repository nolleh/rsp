#include <boost/asio.hpp>
#include <iostream>
#include <memory>

#include "state/state.hpp"

// #include <boost/array.hpp>
int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    std::cout << "connect to ... " << argv[1] << std::endl;
    boost::asio::io_context io_context;

    namespace ip = boost::asio::ip;
    namespace rsp_cli = rsp::cli;

    ip::tcp::resolver resolver(io_context);
    ip::tcp::resolver::query query(argv[1], "8080");
    // io::tcp::resolver::results_type endpoints =
    //     resolver.resolve(argv[1], "daytime");

    ip::tcp::socket socket(io_context);
    boost::asio::connect(socket, resolver.resolve(query));

    auto state = std::make_unique<rsp_cli::state::base_state>();
    for (;;) {
      std::array<char, 128> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);
      auto next = state->handle_buffer(buf, len);
      if (error == boost::asio::error::eof) {
        std::cout << "closed" << std::endl;
        break;
      } else if (error) {
        throw boost::system::system_error(error);
      }

      // state.reset(next);
      // std::cout << buf.data() << std::endl;
      // std::cout.write(buf.data(), len);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
