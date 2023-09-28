#include <istream>
#include <streambuf>
// https://stackoverflow.com/questions/45722747/how-can-i-create-a-istream-from-a-uint8-t-vector
struct membuf : std::streambuf {
  membuf(char const* base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
};

struct imemstream : virtual membuf, std::istream {
  imemstream(char const* base, size_t size)
      : membuf(base, size), std::istream(static_cast<std::streambuf*>(this)) {}
};
