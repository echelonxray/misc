#!/bin/bash

# Start: Configuration

MARCH=armv7e-m
MABI=aapcs
MFPU=fpv4-sp-d16
MFLOATABI=hard
TGTNAME=max32690

SYSROOT="/home/echelon/devel/prefixes/arm_max32690"

BINUTILS_SOURCE_DIR="/home/echelon/devel/git/binutils-gdb"
GCC_SOURCE_DIR="/home/echelon/devel/git/gcc"
MPC_SOURCE_DIR="/home/echelon/devel/mpc/mpc"
MPFR_SOURCE_DIR="/home/echelon/devel/mpfr/mpfr"
GMP_SOURCE_DIR="/home/echelon/devel/gmp/gmp"
LINUX_SOURCE_DIR="/home/echelon/devel/git/linux/linux-echelon"
LIBC_SOURCE_DIR="/home/echelon/devel/git/musl"

TGT1=arm-"$TGTNAME"s1-eabi
TGT2=arm-"$TGTNAME"s2-linux-musleabihf
TGT3=arm-"$TGTNAME"-linux-musleabihf

HOST=x86_64-pc-linux-gnu

echo "TGT1: $TGT1"
echo "TGT2: $TGT2"
echo "TGT3: $TGT3"

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
./../configure --target=$TGT1 --host=$HOST --prefix="$SYSROOT/tools/usr" --with-sysroot="$SYSROOT/tools" --with-mpc="$MPC_SOURCE_DIR" --with-mpfr="$MPFR_SOURCE_DIR" --with-gmp="$GMP_SOURCE_DIR" --with-newlib --without-headers --enable-initfini-array --disable-nls --disable-decimal-float --disable-threads --disable-libatomic --disable-libgomp --disable-libquadmath --disable-libssp --disable-libvtv --disable-libstdcxx --enable-languages=c,c++ --with-multilib-list=rmprofile --disable-softfloat
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
./../configure --host=$TGT2 --build=$HOST --prefix="$SYSROOT/tools/usr" --disable-wrapper --disable-shared CROSS_COMPILE=$TGT1- CFLAGS="-march=$MARCH -mabi=$MABI -mfpu=$MFPU -mfloat-abi=$MFLOATABI -O3 -pipe -fpic -g"
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
./../configure --target=$TGT2 --host=$HOST --prefix="$SYSROOT/tools/usr" --with-sysroot="$SYSROOT/tools" --with-mpc="$MPC_SOURCE_DIR" --with-mpfr="$MPFR_SOURCE_DIR" --with-gmp="$GMP_SOURCE_DIR" --enable-languages=c,c++ --disable-libstdcxx --disable-libgomp --disable-multilib --enable-default-pie --with-arch=$MARCH --with-abi=$MABI --with-fpu=$MFPU --disable-softfloat
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
./../configure --host=$TGT3 --build=$HOST --prefix="$SYSROOT/usr" --disable-wrapper CROSS_COMPILE=$TGT2- CFLAGS="-march=$MARCH -mabi=$MABI -mfpu=$MFPU -mfloat-abi=$MFLOATABI -O3 -pipe -fpic -g"
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
./../configure --target=$TGT3 --host=$HOST --prefix="$SYSROOT/usr" --with-sysroot="$SYSROOT" --with-mpc="$MPC_SOURCE_DIR" --with-mpfr="$MPFR_SOURCE_DIR" --with-gmp="$GMP_SOURCE_DIR" --enable-languages=c,c++ --enable-default-pie --disable-libsanitizer --disable-multilib --with-arch=$MARCH --with-abi=$MABI --with-fpu=$MFPU --disable-softfloat --disable-libstdcxx --disable-libgomp
make -j$(nproc) all
CODE=$?
if [ $CODE -ne 0 ]; then
	echo "Failed: GCC Stage 3"
	exit 1
fi
make install

rm -rf "$SYSROOT/tools"

exit 0
