#include <boost/asio.hpp>
#include <iostream>

// #include <boost/array.hpp>
int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr <<"Usage: client <host>" << std::endl;
      return 1;
    }
    std::cout << "argument" << argv[1] << std::endl;
    boost::asio::io_context io_context;

    namespace ip = boost::asio::ip;

    ip::tcp::resolver resolver(io_context);
    ip::tcp::resolver::query query(argv[1], "8080");
    // io::tcp::resolver::results_type endpoints =
    //     resolver.resolve(argv[1], "daytime");

    ip::tcp::socket socket(io_context);
    boost::asio::connect(socket, resolver.resolve(query));

    for (;;) {
      std::array<char, 10> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);
      if (error == boost::asio::error::eof)
        break;
      else if (error)
        throw boost::system::system_error(error);
      std::cout.write(buf.data(), len);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
