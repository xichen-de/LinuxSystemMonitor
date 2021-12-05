# Linux System Monitor

## Description
![CI](https://github.com/xichen-de/LinuxSystemMonitor/actions/workflows/docker.yml/badge.svg)

This is a command line system monitor program for Linux, similar to `htop`. This tool reads data from the Linux file system and calculates CPU load, memory usage, process information and displays it in the CLI. It also provides information about the system, kernel and uptime.

Built and tested on Ubuntu 20.04.

![image-20211106182734273](README.assets/example.png)

## Usage

### Cloning

```
git clone https://github.com/xichen-de/LinuxSystemMonitor.git
```

### Install dependencies

* cmake >= 3.11
* make >= 4.1
* gcc/g++ >= 5.4
* [ncurses](https://www.gnu.org/software/ncurses/) 

### Compiling

To compile the project, first, create a `build` directory and change to that directory:

```
mkdir build && cd build
```

From within the `build` directory, then run `cmake` and `make` as follows:

```
cmake ..
make
```

### Running

The executable will be placed in the `build` directory. From within `build`, you can run the project as follows:

```
./monitor
```

