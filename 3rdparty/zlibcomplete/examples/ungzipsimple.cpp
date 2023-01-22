#include <iostream>
#include <zlc/zlibcomplete.hpp>

using namespace zlibcomplete;
using namespace std;

int main(int argc, char **argv)
{
  const int CHUNK = 16384;
  char inbuf[CHUNK];
  int readBytes;
  GZipDecompressor decompressor;
  for (;;) {
    cin.read(inbuf, CHUNK);
    readBytes = cin.gcount();
    if (readBytes == 0) {
      break;
    }
    string input(inbuf, readBytes);
    string output = decompressor.decompress(input);
    cout << output;
  }
  return 0;
}
