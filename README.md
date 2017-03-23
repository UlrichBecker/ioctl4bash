# ioctl4bash: ioctl() for bash and/or shell-scripts
**Simple wrapper-program which makes it possible to perform the C-function ioctl() via bash**

A helpful tool e.g. in the developing of linux device-drivers and so on...

**Compiling:**

Type
```
make all
```
for native compiling or
```
make all CC=/path/to/my/cross-compiler/cross-compiler-prefix-gcc
```
for cross compiling.

This program requires two additional source files (parse_opts.h and parse_opts.c) which can be found
in the repository:
[Command line option parser](https://github.com/UlrichBecker/command_line_option_parser)
If "make" doesn't found these both files - or corresponding symbolic links - in the actual directory, so "make" will accomplish this automatically by download this files by "wget".

Type 
```
ioctl -h
```
for more information.


**Note**

Program isn't yet completely tested, but for my purposes it's enough. :-)



