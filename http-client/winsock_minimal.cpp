// cl /std:c++latest /W4 /EHsc /nologo winsock_minimal.cpp && winsock_minimal.exe

#include <iostream>
#include <string_view>

#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
  WSADATA wsaData;
  ::WSAStartup(0x0202, &wsaData);

  struct addrinfo* ai = nullptr;
  struct addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int resolve_err = ::getaddrinfo("httpbin.org", "http", &hints, &ai);
  if (resolve_err != 0)
    return std::cerr << "resolve error\n", 1;
  std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> ai_holder(ai, &freeaddrinfo);

  auto s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  int connect_err = ::connect(s, ai->ai_addr, ai->ai_addrlen);
  if (connect_err != 0)
    return std::cerr << "connect error\n", 1;

  const std::string_view kRequest = "GET /ip HTTP/1.0\r\n\r\n";
  auto written = ::send(s, kRequest.data(), kRequest.size(), 0);
  if (written != (int)kRequest.size())
    return std::cerr << "send error\n", 1;

  char buf[2048];
  auto n = ::recv(s, buf, sizeof(buf), 0);
  std::cout.write(buf, n);
}
