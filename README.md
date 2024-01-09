# Project that hopefully will turn in a very basic OS :)

## Build
In order to build the project, you'll need to set up a gcc cross-compiler for i686-elf. Without using one, there is a chance that a lot of unexpected things will happen because the compiler assumes that the code is running on the host operating system, which will not be the case. To set up one, follow the following steps (for Linux):

1. Make sure you have the following packages installed: `gcc, g++, make, bison, flex, GMP, MPFR, MPC, Texinfo`. Search the specific names for your distribution. On Debia/Ubuntu you can install them with `sudo apt install build-essential bison flex libgmp3-dev libmpfr-dev libmpc-dev texinfo`.

2. Download the source code for `gcc` and `binutils`. You can find them [here](https://www.gnu.org/software/gcc/) and [here](https://www.gnu.org/software/binutils/). Choose a version similar to the one you have installed on your system. You can check the version with `gcc --version` and `ld --version`.

3. Choose a directory where you want to install the cross-compiler. I chose `$HOME/opt/cross` but you can choose whatever you want.

4. Build Binutils
	- Run the following commands:
		```bash
		mkdir build-binutils
		cd build-binutils
		../binutils-x.y.z/configure --target=i686-elf --prefix=<path_to_cross_compiler_location> --with-sysroot --disable-nls --disable-werror
		make
		make install
		```
		Replace `x.y.z` with the version you downloaded and `<path_to_cross_compiler_location>` with the path you chose in step 3.

5. Add the `bin` directory of the cross-compiler to your `PATH` variable. You can do this by adding the following line to your `.bashrc` file if you want to make it permanent or by running it in the terminal if you want to make it temporary:
	```bash
	export PATH="$HOME/opt/cross/bin:$PATH"
	```
	Replace `$HOME/opt/cross` with the path you chose in step 3.

6. Build GCC
	- Run the following commands:
		```bash
		mkdir build-gcc
		cd build-gcc
		../gcc-x.y.z/configure --target=i686-elf --prefix=<path_to_cross_compiler_location> --disable-nls --enable-languages=c,c++ --without-headers
		make all-gcc
		make all-target-libgcc
		make install-gcc
		make install-target-libgcc
		```
		Replace `x.y.z` with the version you downloaded and `<path_to_cross_compiler_location>` with the path you chose in step 3.

7. You should now have a working cross-compiler. You can test it by running `i686-elf-gcc --version`.
