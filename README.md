To make all .sh files executable:
`find . -iname "*.sh" -exec chmod u+x {} \;`

System:
	Linux
	MacOS?
	BSD?
	Not Windows! (sorry, try using Cygwin?)

Required programs (names given are for Nix):
 - `clang`: C++17 compiler (clang recommended)
 - `clang`: C17	compiler (clang recommended)
 - `flex`: Flex 2.6.4
 - `bison`: Bison 3.7.3
 - `bash_5`/`bash`: Bash 5.1.4
 - `gnumake`/`make`: Gnu Make 4.3 (Make if not available on PM, CMake not required)
 - `find`: Find
 - `boost`: C++ Boost libraries (if not available through PM, download files and add -Ipath/to/boost in clang++/g++ commands)

Recommended (not required so don't stress):
 - hexyl: Hexyl, a hex file reader
 - neovim: Neovim (nvim), use coc.vim for .ccls to work

