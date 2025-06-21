#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

if [ ! -e ${OUTDIR} ]; then
    echo "creating ${OUTDIR}"
    mkdir -p ${OUTDIR}
fi

if [ ! -e ${OUTDIR} ]; then
    echo "failed to create ${OUTDIR}"
    exit 1
fi

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
    echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
    git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    # deep clean the kernel build tree, removing .config and any existing configurations
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- mrproper
    # configure for a virtual arm dev board we'll simulate with qemu
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig
    # build a kernel image for booting with qemu
    make -j8 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all
    # build kernel modules
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all
    # build the device tree blob
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dtbs
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "${OUTDIR}"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm -rf rootfs
fi

# TODO: Create necessary base directories
mkdir rootfs
cd rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log
# assignment specific dir in /home
mkdir home/conf

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
cd ${OUTDIR}/rootfs
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
TOOLCHAIN_INSTALL=/opt/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/
cp ${TOOLCHAIN_INSTALL}/aarch64-none-linux-gnu/libc/lib/ld-linux-aarch64.so.1 lib
cp ${TOOLCHAIN_INSTALL}/aarch64-none-linux-gnu/libc/lib64/libm.so.6 lib64
cp ${TOOLCHAIN_INSTALL}/aarch64-none-linux-gnu/libc/lib64/libresolv.so.2 lib64
cp ${TOOLCHAIN_INSTALL}/aarch64-none-linux-gnu/libc/lib64/libc.so.6 lib64

# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1

# TODO: Clean and build the writer utility
pushd $HOME/repos/coursera/sys-prog/assignment-1-shawnadams333/finder-app
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp writer ${OUTDIR}/rootfs/home
cp finder.sh ${OUTDIR}/rootfs/home
cp finder-test.sh ${OUTDIR}/rootfs/home
cp autorun-qemu.sh ${OUTDIR}/rootfs/home
cp conf/assignment.txt ${OUTDIR}/rootfs/home/conf
cp conf/username.txt ${OUTDIR}/rootfs/home/conf
popd

# TODO: Chown the root directory
# handled in the cpio command?

# TODO: Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip -f initramfs.cpio
