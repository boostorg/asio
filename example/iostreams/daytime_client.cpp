#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: daytime_client <host>" << std::endl;
      return 1;
    }

    tcp::iostream s(argv[1], "daytime");
    std::string line;
    std::getline(s, line);
    std::cout << line << std::endl;
  }
  catch (boost::asio::error& e)
  {
    std::cout << e << std::endl;
  }

  return 0;
}
