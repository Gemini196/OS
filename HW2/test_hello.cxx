#include <iostream>
#include <unistd.h>
int main() {
 long r = syscall(333);
 std::cout << "sys_hello returned " << r << std::endl;
 return 0;
}
