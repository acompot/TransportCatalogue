/*
   Хорошего времени суток! ಠ‿ಠ
*/
#include <deque>
#include <iostream>
#include <fstream>
#include "inc/request_handler.h"


int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage:\n"sv << argv[0] << "tr [make_base or process_requests]\n"sv << std::endl;
    return 1;
  }
  const std::string_view mode(argv[1]);
  if ( mode == "make_base"sv || mode == "process_requests"sv) {
    requesthandler::RequestHandler rh;
    if (mode == "make_base"sv) {
      auto doc =std::make_unique<json::Document>(json::Load(std::cin));
      rh.Read(doc).SaveDatabaseFile();
      doc.release();
    } else if (mode == "process_requests"sv) {
      auto doc =std::make_unique<json::Document>(json::Load(std::cin));
      rh.Read(doc).ReadFromDB().SendJson(std::cout);
    }
  } else {
    std::cerr << "Usage:\n"sv << argv[0] << "tr [make_base or process_requests]\n"sv << std::endl;
    return 1;
  }
}
