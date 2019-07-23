// cl /std:c++17 /W4 /EHsc /nologo winapi_sync.cpp && winapi_sync.exe

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

const std::string kHost = "httpbin.org";
const int kPort = 80;
const std::string_view kRequest = "GET /ip HTTP/1.0\r\n\r\n";

int main() {
  // 1) Init WinSock
  WSADATA wsaData;
  ::WSAStartup(0x0202, &wsaData);
  // TODO: defer `::WSACleanup();`

  // 2) Resolve hostname
  struct addrinfo* ai = nullptr;
  struct addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;  // Don't use AI_ADDRCONFIG on Win.
                       // See http://crbug.com/5234

  int resolve_err = ::getaddrinfo(kHost.c_str(), nullptr, &hints, &ai);
  if (resolve_err != 0) {
    std::cerr << "resolve error: " << gai_strerrorA(resolve_err) << '\n';
    return 1;
  }
  std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> ai_holder(ai, &freeaddrinfo);

  // 3) Connect loop.
  auto s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  std::vector<char> addr_buf;
  for (;;) {
    // Copy address and set the port.
    addr_buf.assign((char*)ai->ai_addr, (char*)ai->ai_addr + ai->ai_addrlen);
    auto* addr = (sockaddr_in*)addr_buf.data();
    addr->sin_port = ::htons(kPort);

    // Alternatively, we could call `::getaddrinfo(kHost.c_str(), "http", &hints, &ai);`.
    // and then `::connect(s, ai->ai_addr, ai->ai_addrlen);`
    int connect_err = ::connect(s, (sockaddr*)addr, ai->ai_addrlen);
    if (connect_err == 0)
      break;

    connect_err = WSAGetLastError();
    char ip_buf[64];
    auto* ip = inet_ntop(ai->ai_family, ai->ai_addr, ip_buf, sizeof(ip_buf));
    std::cerr << "connect error " << connect_err
              << " to " << (ip ? ip : "???") << '\n';

    ai = ai->ai_next;
    if (!ai)
      return 1;
  }

  // 4) Send request.
  auto send_data = kRequest;
  while (!send_data.empty()) {
    int written = ::send(s, send_data.data(), send_data.size(), 0);
    if (written <= 0) {
      int send_err = WSAGetLastError();
      std::cerr << "send error: " << send_err << '\n';
      return 1;
    }
    send_data.remove_prefix(written);
  }

  // 5) Read reply.
  for (;;) {
    char buf[2048];
    auto n = ::recv(s, buf, sizeof(buf), 0);
    if (n == 0)
      break;
    if (n > 0) {
      std::cout.write(buf, n);
    } else {
      int recv_err = WSAGetLastError();
      std::cerr << "recv error: " << recv_err << '\n';
      return 1;
    }
  }
  std::cout << "\n--- done ---\n";
}
