#ifndef MINIDBG_DEBUGGER_HPP
#define MINIDBG_DEBUGGER_HPP

#include "breakpoint.hh"
#include <cstdint>
#include <linux/types.h>
#include <string>
#include <unordered_map>
#include <utility>

using namespace minidbg;

class debugger {
public:
  debugger(std::string prog_name, pid_t pid)
      : m_prog_name{std::move(prog_name)}, m_pid{pid} {}

  void run();
  void handle_command(const std::string &line);
  void continue_execution();

  void set_breakpoint_at_address(std::intptr_t addr);

  uint64_t read_memory(uint64_t address);
  void write_memory(uint64_t address, uint64_t value);
  void dump_registers();
  uint64_t get_program_counter();

private:
  std::string m_prog_name;
  pid_t m_pid;
  std::unordered_map<std::intptr_t, breakpoint> m_breakpoints;
};

#endif
