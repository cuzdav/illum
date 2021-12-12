#include "Illum.hpp"

int
main() {
  Illum demo;
  if (demo.Construct(512, 480, 4, 4))
    demo.Start();
  return 0;
}
