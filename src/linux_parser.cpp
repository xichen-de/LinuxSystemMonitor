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
  string unit;
  long memory_total = 1;
  long memory_free = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);
      while (line_stream >> key >> value >> unit) {
        if (key == "MemTotal") {
          memory_total = std::stol(value);
        }
        if (key == "MemFree") {
          memory_free = std::stol(value);
        }
      }
    }
  }
  return (float)(memory_total - memory_free) / (float)memory_total;
}

long LinuxParser::UpTime() {
  string line;
  string key;
  string value;
  string unit;
  long up_time = 1;
  long idle_time = 0;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream line_stream(line);
    line_stream >> up_time >> idle_time;
  }
  return up_time;
}

long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

long LinuxParser::ActiveJiffies() {
  // Calculate active jiffies according to:
  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  std::vector<string> cpu_utilization = LinuxParser::CpuUtilization();
  return std::stol(cpu_utilization[kUser_]) +
         std::stol(cpu_utilization[kNice_]) +
         std::stol(cpu_utilization[kSystem_]) +
         std::stol(cpu_utilization[kIRQ_]) +
         std::stol(cpu_utilization[kSoftIRQ_]) +
         std::stol(cpu_utilization[kSteal_]);
}

long LinuxParser::IdleJiffies() {
  // Calculate idle jiffies according to:
  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux

  std::vector<string> cpu_utilization = LinuxParser::CpuUtilization();
  return std::stol(cpu_utilization[kIdle_]) +
         std::stol(cpu_utilization[kIOwait_]);
}

vector<string> LinuxParser::CpuUtilization() {
  std::vector<string> cpu_utilization;
  string line;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if (file_stream.is_open()) {
    getline(file_stream, line);
    std::istringstream line_stream(line);
    line_stream >> cpu_utilization[kUser_] >> cpu_utilization[kNice_] >>
        cpu_utilization[kSystem_] >> cpu_utilization[kIdle_] >>
        cpu_utilization[kIOwait_] >> cpu_utilization[kIRQ_] >>
        cpu_utilization[kSoftIRQ_] >> cpu_utilization[kSteal_] >>
        cpu_utilization[kGuest_] >> cpu_utilization[kGuestNice_];
  }
  return cpu_utilization;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { return 0; }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { return 0; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid [[maybe_unused]]) { return 0; }
