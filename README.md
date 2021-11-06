# Linux System Monitor

This is a command line system monitor tool for Linux, similar to `htop`.

![image-20211106182734273](README.assets/example.png)

## Cloning

```
git clone https://github.com/xichen-de/LinuxSystemMonitor.git
```

## Compiling and Running	

## Ncurses

This project uses ncurses.

[ncurses](https://www.gnu.org/software/ncurses/) is a library that facilitates text-based graphical output in the terminal. This project relies on ncurses for display output.

Within the Udacity Workspace, `.student_bashrc` automatically installs ncurses every time you launch the Workspace.

If you are not using the Workspace, install ncurses within your own Linux environment: `sudo apt install libncurses5-dev libncursesw5-dev`

## Compiling

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

