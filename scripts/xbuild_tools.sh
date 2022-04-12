#!/bin/bash

# Start: Configuration

MARCH=rv64ia
MABI=lp64

PREFIX_DIR="/home/quantum/devel/prefixes/try_musl_1"

BINUTILS_SOURCE_DIR="/home/quantum/devel/git/binutils-gdb"
GCC_SOURCE_DIR="/home/quantum/devel/git/gcc"
LINUX_SOURCE_DIR="/home/quantum/devel/git/linux"
MUSL_SOURCE_DIR="/home/quantum/devel/git/musl"

TGT1=riscv64-ex1-elf
TGT2=riscv64-ex2-linux-musl
TGT3=riscv64-ex3-linux-musl

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

export PATH=$PREFIX_DIR/bin:$PREFIX_DIR/usr/bin:$PREFIX_DIR/tools/bin:$PREFIX_DIR/tools/usr/bin:$PATH

rm -rf "$PREFIX_DIR"/*
rm -rf "$PREFIX_DIR"/.*

# Binutils Stage 1
cd "$BINUTILS_SOURCE_DIR"
rm -rf ./build1
mkdir ./build1
cd ./build1
./../configure --target=$TGT1 --host=$HOST --prefix="$PREFIX_DIR/tools/usr" --with-sysroot="$PREFIX_DIR/tools" --disable-nls --disable-werror
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
./../configure --target=$TGT1 --host=$HOST --prefix="$PREFIX_DIR/tools/usr" --with-sysroot="$PREFIX_DIR/tools" --with-newlib --without-headers --enable-initfini-array --disable-nls --disable-multilib --disable-decimal-float --disable-threads --disable-libatomic --disable-libgomp --disable-libquadmath --disable-libssp --disable-libvtv --disable-libstdcxx --enable-languages=c,c++ --with-arch=$MARCH --with-abi=$MABI
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: GCC Stage 1"
	exit 1
fi
make install

# Linux Headers Stage 1
cd "$LINUX_SOURCE_DIR"
make ARCH=riscv INSTALL_HDR_PATH="$PREFIX_DIR/tools/usr" headers_install

# MUSL Stage 1
cd "$MUSL_SOURCE_DIR"
rm -rf ./build1
mkdir ./build1
cd ./build1
./../configure --host=$TGT2 --build=$HOST --prefix="$PREFIX_DIR/tools/usr" --disable-wrapper --disable-shared CROSS_COMPILE=$TGT1- CFLAGS="-march=$MARCH -mabi=$MABI -O3 -pipe -fPIC"
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
./../configure --target=$TGT2 --host=$HOST --prefix="$PREFIX_DIR/tools/usr" --with-sysroot="$PREFIX_DIR/tools" --disable-nls --disable-werror
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
./../configure --target=$TGT2 --host=$HOST --prefix="$PREFIX_DIR/tools/usr" --with-sysroot="$PREFIX_DIR/tools" --disable-multilib --with-arch=$MARCH --with-abi=$MABI --enable-languages=c,c++ --disable-libstdcxx
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: GCC Stage 2"
	exit 1
fi
make install

# Linux Headers Stage 2
cd "$LINUX_SOURCE_DIR"
make ARCH=riscv INSTALL_HDR_PATH="$PREFIX_DIR/usr" headers_install

# MUSL Stage 2
cd "$MUSL_SOURCE_DIR"
rm -rf ./build2
mkdir ./build2
cd ./build2
./../configure --host=$TGT3 --build=$HOST --prefix="$PREFIX_DIR/usr" --disable-wrapper CROSS_COMPILE=$TGT2- CFLAGS="-march=$MARCH -mabi=$MABI -O3 -pipe -fPIC"
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
./../configure --target=$TGT3 --host=$HOST --prefix="$PREFIX_DIR/usr" --with-sysroot="$PREFIX_DIR" --disable-werror
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
./../configure --target=$TGT3 --host=$HOST --prefix="$PREFIX_DIR/usr" --with-sysroot="$PREFIX_DIR" --disable-multilib --with-arch=$MARCH --with-abi=$MABI --enable-languages=c,c++ --disable-libsanitizer
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: GCC Stage 3"
	exit 1
fi
make install

rm -rf "$PREFIX_DIR/tools"

exit 0
