#include "debugger.hh"
#include "breakpoint.hh"
#include "debugger.hh"
#include "linenoise.h"
#include "registers.hh"
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <libelfin/elf/elf++.hh>
#include <libelfin/dwarf/dwarf++.hh>

using namespace minidbg;
template <typename T>
std::string int_to_hex(T i)
{
  std::stringstream stream;
  stream << "0x"
         << std::setfill('0') << std::setw(sizeof(T) * 2)
         << std::hex << i;
  return stream.str();
}
// helper for prefix
bool is_prefix(const std::string &s, const std::string &of)
{
  if (s.size() > of.size())
    return false;
  return std::equal(s.begin(), s.end(), of.begin());
}
std::vector<std::string> split(const std::string &s, char delimiter)
{
  std::vector<std::string> out{};
  std::stringstream ss{s};
  std::string item;

  while (std::getline(ss, item, delimiter))
  {
    out.push_back(item);
  }

  return out;
}
void debugger::run()
{
  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);
  char *line = nullptr;
  // main dbg loop
  while ((line = linenoise((int_to_hex(get_program_counter()) + "dbg> ").c_str())) != nullptr)
  {
    handle_command(line);
    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }
}
void debugger::continue_execution()
{
  ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);
}
void debugger::handle_command(const std::string &line)
{
  auto args = split(line, ' ');
  if (args.size() == 0)
  {
    return;
  }

  auto command = args[0];

  if (is_prefix(command, "cont"))
  {
    continue_execution();
  }
  else if (is_prefix(command, "break"))
  {
    std::string addr{args[1], 2}; // assume 0xADDRESS
    set_breakpoint_at_address(std::stol(addr, 0, 16));
  }
  else if (is_prefix(command, "register"))
  {
    if (is_prefix(args[1], "dump"))
    {
      dump_registers();
    }
    else if (is_prefix(args[1], "read"))
    {

      std::cout << get_register_value(m_pid, get_register_from_name(args[2]))
                << std::endl;
    }
    else if (is_prefix(args[1], "write"))
    {

      std::string val{args[3], 2}; // assume 0xVAL
      set_register_value(m_pid, get_register_from_name(args[2]),
                         std::stol(val, 0, 16));
    }
  }
  else
  {
    std::cerr << "Unknown command\n";
  }
}

void debugger::dump_registers()
{
  for (const auto &rd : g_register_descriptors)
  {
    std::cout << rd.name << " 0x" << std::setfill('0') << std::setw(16)
              << std::hex << get_register_value(m_pid, rd.r) << std::endl;
  }
}

void debugger::set_breakpoint_at_address(std::intptr_t addr)
{

  std::cout << "Set breakpoint at address 0x" << std::hex << addr << std::endl;

  breakpoint bp{m_pid, addr};
  bp.enable();

  m_breakpoints[addr] = bp;
}

uint64_t debugger::read_memory(uint64_t address)
{
  return ptrace(PTRACE_PEEKDATA, m_pid, address, nullptr);
}

void debugger::write_memory(uint64_t address, uint64_t value)
{
  ptrace(PTRACE_POKEDATA, m_pid, address, value);
}

// return the instruction pointer
uint64_t debugger::get_program_counter()
{
  return get_register_value(m_pid, reg::rip);
}

dwarf::die debugger::get_function_from_pc(uint64_t pc)
{
  for (auto &cu : m_dwarf.compilation_units())
  {
    if (dwarf::die_pc_range(cu.root()).contains(pc))
    {
      for (const auto &die : cu.root())
      {
        if (die.tag == dwarf::DW_TAG::subprogram)
        {
          if (dwarf::die_pc_range(die).contains(pc))
          {
            return die;
          }
        }
      }
    }
  }
  throw std::out_of_range("Connot find function");
}

dwarf::line_table::iterator debugger::get_line_entry_from_pc(uint64_t pc)
{
  for (auto &cu : m_dwarf.compilation_units())
  {
    if (die_pc_range(cu.root()).contains(pc))
    {
      auto &lt = cu.get_line_table();
      auto it = lt.find_address(pc);
      if (it == lt.end())
      {
        throw std::out_of_range{"Cannot find line entry"};
      }
      else
      {
        return it;
      }
    }
  }

  throw std::out_of_range{"Cannot find line entry"};
}