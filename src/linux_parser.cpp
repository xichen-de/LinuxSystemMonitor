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

#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>

// DONE: An example of how to read data from the filesystem
std::string LinuxParser::OperatingSystem() {
  std::string line;
  std::string key;
  std::string value = "";
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
    return value;
  }
  return "";
}

// DONE: An example of how to read data from the filesystem
std::string LinuxParser::Kernel() {
  std::string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    std::string os, version, kernel;
    linestream >> os >> version >> kernel;
    return kernel;
  }
  return "";
}

// BONUS: Update this to use std::filesystem
std::vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  if (directory != nullptr) {
    struct dirent* file;
    while ((file = readdir(directory)) != nullptr) {
      // Is this a directory?
      if (file->d_type == DT_DIR) {
        // Is every character of the name a digit?
        std::string filename(file->d_name);
        if (std::all_of(filename.begin(), filename.end(), isdigit)) {
          int pid = stoi(filename);
          pids.push_back(pid);
        }
      }
    }
    closedir(directory);
  }
  return pids;
}

float LinuxParser::MemoryUtilization() {
  std::string line;
  std::string key;
  long value;
  long memory_total = 0;
  long memory_free = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "MemTotal") {
          memory_total = value;
        }
        if (key == "MemFree") {
          memory_free = value;
        }
      }
    }
    return (float)(memory_total - memory_free) / (float)memory_total;
  }
  return 0.0;
}

long LinuxParser::UpTime() {
  std::string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream line_stream(line);
    long up_time;
    line_stream >> up_time;
    return up_time;
  }
  return 0;
}

long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

long LinuxParser::ActiveJiffies(int pid) {
  // https://stackoverflow.com/questions/39066998/what-are-the-meaning-of-values-at-proc-pid-stat
  // utime, stime, cutime, cstime are in clock ticks
  // clock per second: sysconf(_SC_CLK_TCK) (declared in the header unistd.h)
  std::vector<std::string> process_utilization = ParseProcessStat(pid);
  long utime = GetValueFromVectorWithDefaultZero(process_utilization, 13);
  long stime = GetValueFromVectorWithDefaultZero(process_utilization, 14);
  long cutime = GetValueFromVectorWithDefaultZero(process_utilization, 15);
  long cstime = GetValueFromVectorWithDefaultZero(process_utilization, 16);
  return utime + stime + cutime + cstime;
}

long LinuxParser::GetValueFromVectorWithDefaultZero(
    const std::vector<std::string>& vec, const int index) {
  try {
    return std::stol(vec[index]);
  } catch (const std::out_of_range& oor) {
    return 0;
  }
}
std::vector<std::string> LinuxParser::ParseProcessStat(int pid) {
  std::string line;
  std::string token;
  std::vector<std::string> process_utilization;
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
  std::vector<std::string> cpu_utilization = LinuxParser::CpuUtilization();
  return std::stol(cpu_utilization[kUser_]) +
         std::stol(cpu_utilization[kNice_]) +
         std::stol(cpu_utilization[kSystem_]) +
         std::stol(cpu_utilization[kIRQ_]) +
         std::stol(cpu_utilization[kSoftIRQ_]) +
         std::stol(cpu_utilization[kSteal_]);
}

long LinuxParser::IdleJiffies() {
  // Expressed in USER_HZ sysconf(_SC_CLK_TCK)
  std::vector<std::string> cpu_utilization = LinuxParser::CpuUtilization();
  return std::stol(cpu_utilization[kIdle_]) +
         std::stol(cpu_utilization[kIOwait_]);
}

std::vector<std::string> LinuxParser::CpuUtilization() {
  // The amount of time, measured in units of USER_HZ
  //(1/100ths of a second on most architectures, use
  // sysconf(_SC_CLK_TCK) to obtain the right value)
  std::vector<std::string> cpu_utilization(10, "0");
  std::string line;
  std::string token;
  std::string cpu;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if (file_stream.is_open()) {
    if (std::getline(file_stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> cpu >> cpu_utilization[kUser_] >> cpu_utilization[kNice_] >>
          cpu_utilization[kSystem_] >> cpu_utilization[kIdle_] >>
          cpu_utilization[kIOwait_] >> cpu_utilization[kIRQ_] >>
          cpu_utilization[kSoftIRQ_] >> cpu_utilization[kSteal_] >>
          cpu_utilization[kGuest_] >> cpu_utilization[kGuestNice_];
    }
  }
  return cpu_utilization;
}

int LinuxParser::TotalProcesses() {
  std::string line;
  std::string key;
  std::string value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "processes") return std::stoi(value);
      }
    }
    return 0;
  }
  return 0;
}

int LinuxParser::RunningProcesses() {
  std::string line;
  std::string key;
  std::string value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "procs_running") return std::stoi(value);
      }
    }
    return 0;
  }
  return 0;
}

std::string LinuxParser::Command(int pid) {
  std::string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open() && std::getline(stream, line)) {
    return line;
  }
  return "";
}

std::string LinuxParser::Ram(int pid) {
  std::string line;
  std::string key;
  std::string value = "0";
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "VmSize:") return std::to_string(std::stoi(value) / 1000);
      }
    }
    return value;
  }
  return "";
}

std::string LinuxParser::Uid(int pid) {
  std::string line;
  std::string key;
  std::string value = "";
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
    return value;
  }
  return "";
}

std::string LinuxParser::User(int pid) {
  std::string uid = LinuxParser::Uid(pid);
  std::string ignore;
  std::string token;
  std::string line;
  std::string username;
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
    return username;
  }
  return "";
}

long LinuxParser::UpTime(int pid) {
  std::vector<std::string> process_utilization = ParseProcessStat(pid);
  long start_time = std::stol(process_utilization[21]);
  return UpTime() - start_time / (long)sysconf(_SC_CLK_TCK);
}
