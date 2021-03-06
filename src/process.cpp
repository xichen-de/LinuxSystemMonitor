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

#include "process.h"

#include <linux_parser.h>
#include <unistd.h>

#include <string>

Process::Process(int pid) {
  pid_ = pid;
  command = LinuxParser::Command(pid);
  user = LinuxParser::User(pid);
  uptime = LinuxParser::UpTime(pid);
  ram = LinuxParser::Ram(pid);
  cpu_utilization = Process::CalculateCpuUtilization();
}

int Process::Pid() const { return pid_; }

float Process::CalculateCpuUtilization() const {
  // https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599
  long active_time = LinuxParser::ActiveJiffies(pid_) / sysconf(_SC_CLK_TCK);
  long total_time = Process::UpTime();
  return (float)active_time / (float)total_time;
}
float Process::CpuUtilization() const { return cpu_utilization; }

std::string Process::Command() const { return command; }

std::string Process::Ram() const { return ram; }

std::string Process::User() const { return user; }

long int Process::UpTime() const { return uptime; }

bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}
