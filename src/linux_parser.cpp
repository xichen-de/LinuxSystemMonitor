// PROJECT LICENSE
//
// This project was submitted by Xi Chen as part of the Nanodegree At Udacity.
//
// As part of Udacity Honor code, your submissions must be your own work, hence
// submitting this project as yours will cause you to break the Udacity Honor
// Code and the suspension of your account.
//
// Me, the author of the project, allow you to check the code as a reference,
// but if you submit it, it's your own responsibility if you get expelled.
//
// Copyright (c) 2021 Xi Chen
//
// Besides the above notice, the following license applies and this license
// notice must be included in all works derived from this project.
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "format.h"
using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string value;
  long memory_total;
  long memory_free;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "MemTotal") {
          memory_total = std::stol(value);
        }
        if (key == "MemFree") {
          memory_free = std::stol(value);
        }
      }
    }
    return (float)(memory_total - memory_free) / (float)memory_total;
  }
  return 0.0;
}

long LinuxParser::UpTime() {
  // https://man7.org/linux/man-pages/man5/proc.5.html
  // This file contains two numbers (values in seconds): the
  // uptime of the system (including time spent in suspend) and
  // the amount of time spent in the idle process.
  string line;
  string up_time{"0"};
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream line_stream(line);
    line_stream >> up_time;
  }
  return std::stol(up_time);
}

long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

long LinuxParser::ActiveJiffies(int pid) {
  // https://stackoverflow.com/questions/39066998/what-are-the-meaning-of-values-at-proc-pid-stat
  // utime, stime, cutime, cstime are in clock ticks
  // clock per second: sysconf(_SC_CLK_TCK) (declared in the header unistd.h)
  vector<string> process_utilization = ParseProcessStat(pid);
  long utime = GetValueFromVectorWithDefaultZero(process_utilization, 13);
  long stime = GetValueFromVectorWithDefaultZero(process_utilization, 14);
  long cutime = GetValueFromVectorWithDefaultZero(process_utilization, 15);
  long cstime = GetValueFromVectorWithDefaultZero(process_utilization, 16);
  return utime + stime + cutime + cstime;
}

long LinuxParser::GetValueFromVectorWithDefaultZero(const vector<string>& vec,
                                                    const int index) {
  try {
    return std::stol(vec[index]);
  } catch (const std::out_of_range& oor) {
    return 0;
  }
}
vector<string> LinuxParser::ParseProcessStat(int pid) {
  string line;
  string token;
  std::vector<string> process_utilization;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream line_stream(line);
    while (line_stream >> token) {
      process_utilization.push_back(token);
    }
  }
  return process_utilization;
}

long LinuxParser::ActiveJiffies() {
  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  // expressed in USER_HZ sysconf(_SC_CLK_TCK)
  std::vector<string> cpu_utilization = LinuxParser::CpuUtilization();
  return std::stol(cpu_utilization[kUser_]) +
         std::stol(cpu_utilization[kNice_]) +
         std::stol(cpu_utilization[kSystem_]) +
         std::stol(cpu_utilization[kIRQ_]) +
         std::stol(cpu_utilization[kSoftIRQ_]) +
         std::stol(cpu_utilization[kSteal_]);
}

long LinuxParser::IdleJiffies() {
  // Expressed in USER_HZ sysconf(_SC_CLK_TCK)
  std::vector<string> cpu_utilization = LinuxParser::CpuUtilization();
  return std::stol(cpu_utilization[kIdle_]) +
         std::stol(cpu_utilization[kIOwait_]);
}

vector<string> LinuxParser::CpuUtilization() {
  // The amount of time, measured in units of USER_HZ
  //(1/100ths of a second on most architectures, use
  // sysconf(_SC_CLK_TCK) to obtain the right value)
  std::vector<string> cpu_utilization(10, "0");
  string line;
  string token;
  string cpu;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if (file_stream.is_open()) {
    getline(file_stream, line);
    std::istringstream line_stream(line);
    line_stream >> cpu >> cpu_utilization[kUser_] >> cpu_utilization[kNice_] >>
        cpu_utilization[kSystem_] >> cpu_utilization[kIdle_] >>
        cpu_utilization[kIOwait_] >> cpu_utilization[kIRQ_] >>
        cpu_utilization[kSoftIRQ_] >> cpu_utilization[kSteal_] >>
        cpu_utilization[kGuest_] >> cpu_utilization[kGuestNice_];
  }
  return cpu_utilization;
}

int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  std::vector<string> process_utilization;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "processes") return std::stoi(value);
      }
    }
  }
  return 0;
}

int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  std::vector<string> process_utilization;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "procs_running") return std::stoi(value);
      }
    }
  }
  return 0;
}

string LinuxParser::Command(int pid) {
  string line;
  std::vector<string> process_utilization;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
}

string LinuxParser::Ram(int pid) {
  // https://stackoverflow.com/questions/131303/how-can-i-measure-the-actual-memory-usage-of-an-application-or-process
  string line;
  string key;
  string value{"0"};
  string unit;
  std::vector<string> process_utilization;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "VmSize:") return std::to_string(std::stoi(value) / 1000);
      }
    }
  }
  return value;
}

string LinuxParser::Uid(int pid) {
  // https://superuser.com/questions/479746/how-to-find-pids-user-name-in-linux/1361046
  string line;
  string key;
  string value;
  std::ifstream file_stream(kProcDirectory + std::to_string(pid) +
                            kStatusFilename);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "Uid") {
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string ignore;
  string token;
  string line;
  string username;
  string value;
  std::ifstream file_stream(kPasswordPath);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);
      line_stream >> username >> ignore >> token;
      if (token == uid) {
        return username;
      }
    }
  }
  return username;
}

long LinuxParser::UpTime(int pid) {
  // The time the process started after system boot.  In
  // kernels before Linux 2.6, this value was expressed
  // in jiffies.  Since Linux 2.6, the value is
  // expressed in clock ticks (divide by
  // sysconf(_SC_CLK_TCK)).
  vector<string> process_utilization = ParseProcessStat(pid);
  long start_time = std::stol(process_utilization[21]);
  return UpTime() - start_time / sysconf(_SC_CLK_TCK);
}
