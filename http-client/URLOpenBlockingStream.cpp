// cl /std:c++latest /W4 /EHsc /nologo URLOpenBlockingStream.cpp && URLOpenBlockingStream.exe

#include <iostream>

#include <Urlmon.h>
#pragma comment(lib, "Urlmon.lib")

int main() {
  IStream* stream = nullptr;
  auto hr = URLOpenBlockingStreamA(nullptr, "http://httpbin.org/ip", &stream, 0, nullptr);
  if (hr != S_OK)
    return std::cerr << "open error " << hr << '\n', 1;

  char buf[2048];
  ULONG n = 0;
  while (stream->Read(buf, sizeof(buf), &n) == S_OK)
    std::cout.write(buf, n);

  stream->Release();
}
