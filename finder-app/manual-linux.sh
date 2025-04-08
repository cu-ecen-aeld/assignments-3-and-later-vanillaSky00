#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v6.10.2
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
SCRIPT_PATH=$(pwd)

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR} || { echo "Failed to create outdir"; exit 1; }

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    #git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

#copy the compiled kernel Image to OUTDIR
echo "Adding the Image in outdir"
cp $OUTDIR/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/ 

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir -p ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p /usr/bin usr/lib usr/sbin
mkdir -p /var/log

cd ${OUTDIR}
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} 
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

cd ${OUTDIR}/rootfs
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
libcpath=$(${CROSS_COMPILE}gcc --print-sysroot)
echo $libcpath

cp $libcpath/lib/ld-linux-aarch64.so.1 $OUTDIR/rootfs/lib/
cp $libcpath/lib64/libm.so.6 $OUTDIR/rootfs/lib64/
cp $libcpath/lib64/libresolv.so.2 $OUTDIR/rootfs/lib64/
cp $libcpath/lib64/libc.so.6 $OUTDIR/rootfs/lib64/

# TODO: Make device nodes: /dev/mull and /dev/console
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1

# TODO: Clean and build the writer utility
cd $FINDER_APP_DIR 
echo "SCRIPT_PATH is $FINDER_APP_DIR "
echo "CROSS_COMPILE is $CROSS_COMPILE"
make clean
make CROSS_COMPILE=$CROSS_COMPILE

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp -rH "../conf" "$OUTDIR/rootfs/home"
cp *.sh writer $OUTDIR/rootfs/home/

sed -i 's|../conf/|conf/|g' ${OUTDIR}/rootfs/home/finder-test.sh
sed -i 's|writer.sh|writer|g' ${OUTDIR}/rootfs/home/finder-test.sh
sed -i 's|./finder.sh|sh finder.sh|g' ${OUTDIR}/rootfs/home/finder-test.sh

# TODO: Chown the root directory 
sudo chown -R root:root ${OUTDIR}/rootfs

# TODO: Create initramfs.cpio.gz for booting
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ../initramfs.cpio
gzip -f ../initramfs.cpio
