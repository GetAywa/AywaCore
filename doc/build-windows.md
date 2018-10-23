WINDOWS BUILD NOTES
====================

Some notes on how to build Aywa Core for Windows.

Most developers use cross-compilation from Ubuntu to build executables for
Windows. This is also used to build the release binaries.

Building on Windows itself is possible (for example using msys / mingw-w64),
but no one documented the steps to do this. If you are doing this, please contribute them.

Cross-compilation
-------------------

These steps can be performed on, for example, an Ubuntu VM. The depends system
will also work on other Linux distributions, however the commands for
installing the toolchain will be different.

First install the toolchains:

    sudo apt-get install g++-mingw-w64-i686 mingw-w64-i686-dev g++-mingw-w64-x86-64 mingw-w64-x86-64-dev curl
    
    sudo apt-get install curl g++-aarch64-linux-gnu g++-4.8-aarch64-linux-gnu gcc-4.8-aarch64-linux-gnu binutils-aarch64-linux-gnu g++-arm-linux-gnueabihf g++-4.8-arm-linux-gnueabihf gcc-4.8-arm-linux-gnueabihf binutils-arm-linux-gnueabihf g++-4.8-multilib gcc-4.8-multilib binutils-gold bsdmainutils


To build executables for Windows 32-bit:

    cd depends
    make HOST=i686-w64-mingw32 -j4
    cd ..
    ./configure --prefix=`pwd`/depends/i686-w64-mingw32
    make

To build executables for Windows 64-bit:

    cd depends
    make HOST=x86_64-w64-mingw32 -j4
    cd ..
    ./configure --prefix=`pwd`/depends/x86_64-w64-mingw32
    make

For further documentation on the depends system see [README.md](../depends/README.md) in the depends directory.

Known error:

On Unix subsystem Windows 10:

user@PC01:/mnt/c/Users/user/aywacore/depends$ make HOST=i686-w64-mingw32 -j 4
Configuring native_ccache...
/bin/sh: 1: Syntax error: "(" unexpected
funcs.mk:238: recipe for target '/mnt/c/Users/user/aywacore/depends/work/build/i686-w64-mingw32/native_ccache/3.2.4-c84424c254e/./.stamp_configured' failed
make: *** [/mnt/c/Users/user/aywacore/depends/work/build/i686-w64-mingw32/native_ccache/3.2.4-c84424c254e/./.stamp_configured] Error 2

WORKAROUND
remove default windows PATH

user@PC01:/mnt/c/Users/user/aywacore/depends$ echo ${PATH}
/home/user/bin:/home/user/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/mnt/c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v9.1/bin:/mnt/c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v9.1/libnvvp:/mnt/c/ProgramData/Oracle/Java/javapath_target_711130843:/mnt/c/Perl64/site/bin:/mnt/c/Perl64/bin:/mnt/c/Windows/System32:/mnt/c/Windows:/mnt/c/Windows/System32/wbem:/mnt/c/Windows/System32/WindowsPowerShell/v1.0:/mnt/c/Program Files (x86)/NVIDIA Corporation/PhysX/Common:/mnt/c/Windows/System32:/mnt/c/Windows:/mnt/c/Windows/System32/wbem:/mnt/c/Windows/System32/WindowsPowerShell/v1.0:/mnt/c/Program Files/dotnet:/mnt/c/Program Files/Microsoft SQL Server/130/Tools/Binn:/mnt/c/MinGW/bin:/mnt/c/Program Files (x86)/PuTTY:/mnt/c/Program Files (x86)/WinMerge:/mnt/c/Program Files/doxygen/bin:/mnt/c/Program Files/Geth:/mnt/c/Program Files/Btcd Suite/Btcd:/mnt/c/Users/user/AppData/Local/Microsoft/WindowsApps:/mnt/c/Users/user/AppData/Local/GitHubDesktop/bin:/mnt/c/Users/user/AppData/Local/Microsoft/WindowsApps:/snap/bin

user@PC01:/mnt/c/Users/user/aywacore/depends$ export PATH=/home/user/bin:/home/user/.local/bin:/usr/local/sbin:/usr/loca
l/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games

user@PC01:/mnt/c/Users/user/aywacore/depends$ echo ${PATH}
/home/user/bin:/home/user/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games