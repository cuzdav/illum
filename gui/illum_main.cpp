#include "Illum.hpp"

int
main() {
  Illum demo;
  if (demo.Construct(512, 480, 2, 2))
    demo.Start();
  return 0;
}
