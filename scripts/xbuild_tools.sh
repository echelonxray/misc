#!/bin/bash

# Start: Configuration

MARCH=rv32ia
MABI=ilp32

SYSROOT="/home/quantum/devel/prefixes/try_musl_0"

BINUTILS_SOURCE_DIR="/home/quantum/devel/git/binutils-gdb"
GCC_SOURCE_DIR="/home/quantum/devel/git/gcc"
MPC_SOURCE_DIR="/home/quantum/devel/mpc/mpc"
MPFR_SOURCE_DIR="/home/quantum/devel/mpfr/mpfr"
GMP_SOURCE_DIR="/home/quantum/devel/gmp/gmp"
LINUX_SOURCE_DIR="/home/quantum/devel/git/linux"
LIBC_SOURCE_DIR="/home/quantum/devel/git/musl"

TGT1=riscv32-rv32ia1-elf
#TGT1=riscv32-rv32ia1-linux-musl
TGT2=riscv32-rv32ia2-linux-musl
TGT3=riscv32-rv32ia3-linux-musl

HOST=x86_64-pc-linux-gnu

# End: Configuration


# Binutils
# Stage 1: ./../configure --prefix=$PFX/tools/usr --with-sysroot=$PFX --target=$TGT1 --host=$HOST --disable-nls --disable-werror
# Stage 2: ./../configure --prefix=$PFX/tools/usr --with-sysroot=$PFX/tools --target=$TGT2 --host=$HOST --disable-nls --disable-werror

# GCC
# Stage 1: ./../configure --target=$TGT1 --host=$HOST --prefix=$PFX/tools/usr --with-sysroot=$PFX --with-newlib --without-headers --enable-initfini-array --disable-nls --disable-multilib --disable-decimal-float --disable-threads --disable-libatomic --disable-libgomp --disable-libquadmath --disable-libssp --disable-libvtv --disable-libstdcxx --enable-languages=c,c++ --with-arch=rv64ia --with-abi=lp64
# Stage 2: ./../configure --target=$TGT2 --prefix=$PFX/tools/usr --with-sysroot=$PFX/tools --disable-multilib --with-arch=rv64ia --with-abi=lp64 --enable-languages=c,c++ --disable-libstdcxx

# MUSL
# Stage 1: ./../configure --prefix=$PFX/tools/usr --host=$TGT2 --build=$HOST --disable-wrapper CROSS_COMPILE=$TGT1- CFLAGS="-march=rv64ia -mabi=lp64 -O3 -pipe" --disable-shared

renice -n 10 $$

export PATH=$SYSROOT/bin:$SYSROOT/usr/bin:$SYSROOT/tools/bin:$SYSROOT/tools/usr/bin:$PATH

rm -rf "$SYSROOT"/*
rm -rf "$SYSROOT"/.*

# Binutils Stage 1
cd "$BINUTILS_SOURCE_DIR"
rm -rf ./build1
mkdir ./build1
cd ./build1
./../configure --target=$TGT1 --host=$HOST --prefix="$SYSROOT/tools/usr" --with-sysroot="$SYSROOT/tools" --disable-nls --disable-werror
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: Binutils Stage 1"
	exit 1
fi
make install

# GCC Stage 1
cd "$GCC_SOURCE_DIR"
rm -rf ./build1
mkdir ./build1
cd ./build1
./../configure --target=$TGT1 --host=$HOST --prefix="$SYSROOT/tools/usr" --with-sysroot="$SYSROOT/tools" --with-mpc="$MPC_SOURCE_DIR" --with-mpfr="$MPFR_SOURCE_DIR" --with-gmp="$GMP_SOURCE_DIR" --with-newlib --without-headers --enable-initfini-array --disable-nls --disable-multilib --disable-decimal-float --disable-threads --disable-libatomic --disable-libgomp --disable-libquadmath --disable-libssp --disable-libvtv --disable-libstdcxx --enable-languages=c,c++ --with-arch=$MARCH --with-abi=$MABI --enable-checking=none
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: GCC Stage 1"
	exit 1
fi
make install

# Linux Headers Stage 1
cd "$LINUX_SOURCE_DIR"
make ARCH=riscv INSTALL_HDR_PATH="$SYSROOT/tools/usr" headers_install

# MUSL Stage 1
cd "$LIBC_SOURCE_DIR"
rm -rf ./build1
mkdir ./build1
cd ./build1
./../configure --host=$TGT2 --build=$HOST --prefix="$SYSROOT/tools/usr" --disable-wrapper --disable-shared CROSS_COMPILE=$TGT1- CFLAGS="-march=$MARCH -mabi=$MABI -O3 -pipe -fPIC"
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: MUSL Stage 1"
	exit 1
fi
make install

# Binutils Stage 2
cd "$BINUTILS_SOURCE_DIR"
rm -rf ./build2
mkdir ./build2
cd ./build2
./../configure --target=$TGT2 --host=$HOST --prefix="$SYSROOT/tools/usr" --with-sysroot="$SYSROOT/tools" --disable-nls --disable-werror
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: Binutils Stage 2"
	exit 1
fi
make install

# GCC Stage 2
cd "$GCC_SOURCE_DIR"
rm -rf ./build2
mkdir ./build2
cd ./build2
./../configure --target=$TGT2 --host=$HOST --prefix="$SYSROOT/tools/usr" --with-sysroot="$SYSROOT/tools" --with-mpc="$MPC_SOURCE_DIR" --with-mpfr="$MPFR_SOURCE_DIR" --with-gmp="$GMP_SOURCE_DIR" --disable-multilib --with-arch=$MARCH --with-abi=$MABI --enable-languages=c,c++ --disable-libstdcxx --enable-checking=none
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: GCC Stage 2"
	exit 1
fi
make install

# Linux Headers Stage 2
cd "$LINUX_SOURCE_DIR"
make ARCH=riscv INSTALL_HDR_PATH="$SYSROOT/usr" headers_install

# MUSL Stage 2
cd "$LIBC_SOURCE_DIR"
rm -rf ./build2
mkdir ./build2
cd ./build2
./../configure --host=$TGT3 --build=$HOST --prefix="$SYSROOT/usr" --disable-wrapper CROSS_COMPILE=$TGT2- CFLAGS="-march=$MARCH -mabi=$MABI -O3 -pipe -fpic -g"
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: MUSL Stage 2"
	exit 1
fi
make install

# Binutils Stage 3
cd "$BINUTILS_SOURCE_DIR"
rm -rf ./build3
mkdir ./build3
cd ./build3
./../configure --target=$TGT3 --host=$HOST --prefix="$SYSROOT/usr" --with-sysroot="$SYSROOT" --disable-werror
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: Binutils Stage 3"
	exit 1
fi
make install

# GCC Stage 3
cd "$GCC_SOURCE_DIR"
rm -rf ./build3
mkdir ./build3
cd ./build3
./../configure --target=$TGT3 --host=$HOST --prefix="$SYSROOT/usr" --with-sysroot="$SYSROOT" --with-mpc="$MPC_SOURCE_DIR" --with-mpfr="$MPFR_SOURCE_DIR" --with-gmp="$GMP_SOURCE_DIR" --disable-multilib --with-arch=$MARCH --with-abi=$MABI --enable-languages=c,c++ --enable-default-pie --enable-checking=none
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: GCC Stage 3"
	exit 1
fi
make install

rm -rf "$SYSROOT/tools"

exit 0
