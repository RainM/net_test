#include "server.hpp"

#include <iostream>

int main(int argc, char** argv) {
  if (argc < 2) {
      std::cerr << "No sufficient arguments\n";
      std::cerr << "Usage: " << argv[0] << " port" << std::endl;
      ::exit(-1);
  }

  try {
      int port = atoi(argv[1]);
      mirror_server srv(port);

      srv.do_serve();
  } catch (std::exception& e) {
      std::cerr << "Fatal error" << std::endl;
      std::cerr << e.what() << std::endl;
      ::exit (-1);
  }
  
  return 0;
}
