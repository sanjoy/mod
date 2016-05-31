#include "integer.hpp"

#include <iostream>

using namespace mod;
using namespace std;

typedef Integer<6, 64> Int64;

static void additions() {
  {
    cout << "\n## Addition phase 1\n";
    Int64 a, b, c;

    a.read("00u");
    b.read("001");
    c.read("001");

    cout << a.add(b).add(c).write() << "\n";
  }
  {
    cout << "\n## Addition phase 2\n";
    Int64 a, b, c;

    a.read("00u");
    b.read("001");
    c.read("001");

    cout << a.add(b.add(c)) << "\n";
  }
  {
    cout << "\n## Addition phase 3\n";
    Int64 a, b, c;

    a.read("000");
    b.read("001");
    c.read("0u1");

    cout << a.add(b) << "\n";
    cout << a.add(c) << "\n";
    cout << b.add(c) << "\n";
  }
}

static void multiplications() {
  {
    cout << "\n## Multiplication phase 1\n";

    Int64 a, b, c, d;

    a.read("00u");
    b.read("1u1");
    c.read("001");
    d.read("100");

    cout << a.multiply(b).write() << "\n";
    cout << b.multiply(a).write() << "\n";
    cout << a.multiply(a).write() << "\n";
  }
}

int main() {
  additions();
  multiplications();
}
