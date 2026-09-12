/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <unistd.h>

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <boost/asio.hpp>

#include "rspcli/client_session.hpp"
#include "rsplib/debug/tracer.hpp"
#include "rsplib/logger/logger.hpp"

namespace {

using input_stream = boost::asio::posix::stream_descriptor;

void read_command(input_stream& input, boost::asio::streambuf& buffer,
                  const std::shared_ptr<rsp::cli::client_session>& client) {
  boost::asio::async_read_until(
      input, buffer, '\n',
      [&input, &buffer, client](const boost::system::error_code& error,
                                std::size_t) {
        if (error) {
          if (error != boost::asio::error::operation_aborted) client->stop();
          return;
        }

        std::istream stream(&buffer);
        std::string command;
        std::getline(stream, command);
        if (!command.empty() && command.back() == '\r') command.pop_back();
        client->handle_command(command);

        if (client->current_state() != rsp::cli::state::State::kExit) {
          read_command(input, buffer, client);
        }
      });
}

}  // namespace

int main(int argc, char* argv[]) {
  namespace lg = rsp::libs::logger;
  namespace ip = boost::asio::ip;

  auto& logger = lg::logger(lg::log_level::kDebug);
  rsp::libs::tracer::install();

  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    boost::asio::io_context io_context;
    ip::tcp::resolver resolver(io_context);
    ip::tcp::socket socket(io_context);

    logger.debug() << "connect to ... " << argv[1] << lg::L_endl;
    boost::asio::connect(socket, resolver.resolve(argv[1], "8080"));

    const int input_fd = ::dup(STDIN_FILENO);
    if (input_fd == -1) {
      throw std::runtime_error("failed to duplicate stdin");
    }

    input_stream input(io_context, input_fd);
    boost::asio::streambuf input_buffer;
    auto client = std::make_shared<rsp::cli::client_session>(std::move(socket));

    client->set_exit_handler([&input]() {
      boost::system::error_code ignored;
      input.cancel(ignored);
      input.close(ignored);
    });

    client->start();
    read_command(input, input_buffer, client);
    io_context.run();
  } catch (const std::exception& error) {
    logger.error() << error.what() << lg::L_endl;
    return 1;
  }
}
