// MIT License
//
// Copyright (c) 2021 Xi Chen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "system.h"

#include <linux_parser.h>

#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

System::System() {
  kernel_ = LinuxParser::Kernel();
  os_ = LinuxParser::OperatingSystem();
}
Processor& System::Cpu() { return cpu_; }

std::vector<Process>& System::Processes() {
  processes_.clear();
  std::vector<int> pids = LinuxParser::Pids();
  for (int pid : pids) {
    try {
      auto process = Process(pid);
      processes_.push_back(process);
    } catch (std::exception& e) {
      // Do nothing
    }
  }
  std::sort(processes_.rbegin(), processes_.rend());
  return processes_;
}

std::string System::Kernel() { return kernel_; }

float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() { return os_; }

int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

long int System::UpTime() { return LinuxParser::UpTime(); }