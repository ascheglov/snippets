// cl /std:c++latest /W4 /EHsc /nologo SetAddrInfoEx.cpp && SetAddrInfoEx.exe

#include <iostream>
#include <string_view>

#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

auto resolve(const char* host) {
  struct addrinfo* ai = nullptr;
  struct addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int resolve_err = ::getaddrinfo(host, "http", &hints, &ai);
  if (resolve_err != 0)
    throw std::runtime_error("resolve error");

  return std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)>(ai, &freeaddrinfo);
}

void http_get(const char* host) {
  auto ai = resolve(host);

  auto s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  int connect_err = ::connect(s, ai->ai_addr, ai->ai_addrlen);
  if (connect_err != 0)
    throw std::runtime_error("connect error");

  const std::string_view kRequest = "GET /ip HTTP/1.0\r\n\r\n";
  auto written = ::send(s, kRequest.data(), kRequest.size(), 0);
  if (written != (int)kRequest.size())
    throw std::runtime_error("send error");

  char buf[2048];
  auto n = ::recv(s, buf, sizeof(buf), 0);
  std::cout.write(buf, n);
}

int main() {
  WSADATA wsaData;
  ::WSAStartup(0x0202, &wsaData);

  auto ai = resolve("example.com");

  SOCKET_ADDRESS sa = {ai->ai_addr, (int)ai->ai_addrlen};
  int err = ::SetAddrInfoExW(
      L"httpbin.org",
      nullptr,  // service
      &sa,  // array of addresses
      1,  // one element
      nullptr,  // blob
      0,  // flags
      NS_DNS,  // namespace  <---- DOESN'T WORK BECAUSE |NS_DNS| IS NOT SUPPORTED.
      nullptr,  // namespace GUID
      nullptr,  // timeout (reserved)
      nullptr,  // overlapped (reserved)
      nullptr,  // callback (reserved)
      nullptr);  // name handle (reserved)
  if (err != 0) {
    err = WSAGetLastError();
    std::cerr << "SetAddrInfoExA error: " << err << '\n';
    // !!! Returns ERROR_NOT_SUPPORTED (50) !!!
    return 1;
  }

  try {
    http_get("httpbin.org");
  } catch (std::exception& e) {
    std::cerr << e.what() << '\n';
  }
}
