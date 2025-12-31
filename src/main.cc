// #include "libelfin/dwarf/dwarf++.hh"
// #include "libelfin/elf/elf++.hh"
#include "debugger.hh"
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <pthread.h>
#include <string>
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace minidbg;

// debugger class implementation
static pthread_t thread_info;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cerr << "pls specify the program name" << '\n';
    return EXIT_FAILURE;
  }

  char *prog = argv[1];
  auto pid = fork();
  if (pid == 0)
  {
    // child execute the program to debug
    personality(ADDR_NO_RANDOMIZE);
    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    execl(prog, prog, nullptr);
    thread_info = pthread_self();
  }
  else if (pid >= 1)
  {
    // parent
    std::cout << "started debugging the process" << pid << '\n';
    std::cout << thread_info << std::endl;
    debugger dbg{prog, pid};
    dbg.run();
  }
  return 0;
}
