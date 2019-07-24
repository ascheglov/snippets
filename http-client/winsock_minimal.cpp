// cl /std:c++latest /W4 /EHsc /nologo winsock_minimal.cpp && winsock_minimal.exe

#include <iostream>
#include <string_view>

#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
  WSADATA wsaData;
  ::WSAStartup(0x0202, &wsaData);

  auto s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  int connect_ok = WSAConnectByNameW(s, L"httpbin.org", L"http", 0, nullptr, 0, nullptr, nullptr, nullptr);
  if (!connect_ok)
    return std::cerr << "connect error " << WSAGetLastError() << '\n', 1;

  const std::string_view kRequest = "GET /ip HTTP/1.0\r\n\r\n";
  auto written = ::send(s, kRequest.data(), kRequest.size(), 0);
  if (written != (int)kRequest.size())
    return std::cerr << "send error\n", 1;

  char buf[2048];
  auto n = ::recv(s, buf, sizeof(buf), 0);
  std::cout.write(buf, n);
}
