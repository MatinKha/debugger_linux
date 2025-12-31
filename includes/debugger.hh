#ifndef MINIDBG_DEBUGGER_HPP
#define MINIDBG_DEBUGGER_HPP

#include "breakpoint.hh"
#include <cstdint>
#include <linux/types.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <fcntl.h>
#include <libelfin/elf/elf++.hh>
#include <libelfin/dwarf/dwarf++.hh>

using namespace minidbg;

class debugger
{
public:
  debugger(std::string prog_name, pid_t pid)
      : m_prog_name{std::move(prog_name)}, m_pid{pid}
  {
    auto fd = open(m_prog_name.c_str(), O_RDONLY);
    m_elf = elf::elf(elf::create_mmap_loader(fd));
    m_dwarf = dwarf::dwarf(dwarf::elf::create_loader(m_elf));
  }

  // general
  void run();
  void handle_command(const std::string &line);
  void continue_execution();

  // breakpoints
  void set_breakpoint_at_address(std::intptr_t addr);

  // dwarf
  dwarf::die get_function_from_pc(uint64_t pc);
  dwarf::line_table::iterator get_line_entry_from_pc(uint64_t pc);

  // memory
  uint64_t read_memory(uint64_t address);
  void write_memory(uint64_t address, uint64_t value);

  // registers
  void dump_registers();
  uint64_t get_program_counter();

private:
  std::string m_prog_name;
  pid_t m_pid;
  elf::elf m_elf;
  dwarf::dwarf m_dwarf;
  std::unordered_map<std::intptr_t, breakpoint>
      m_breakpoints;
};

#endif
