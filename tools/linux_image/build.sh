#!/bin/sh
# One-command, cached Linux image build for the Kathryn RV64 nommu SoC.
#   tools/linux_image/build.sh [kernel|dtb|all]
# Artifacts land in tools/linux_image/artifacts/ (gitignored):
#   Image        — kernel binary (load at 0x80000000)
#   virt_nommu.dtb — device tree  (load at 0x87E00000, matches boot_regs a1)
set -e
cd "$(dirname "$0")"
unset LD_LIBRARY_PATH      # buildroot refuses to run with it set (CUDA profile export)

KVER=6.6.32
KURL="https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-$KVER.tar.xz"
CROSS=riscv64-unknown-elf-
JOBS=$(nproc)

mkdir -p artifacts build
what="${1:-all}"

# Kernel kconfig needs flex + bison; if absent, build them locally (no root).
HOSTTOOLS="$PWD/build/hosttools"
export PATH="$HOSTTOOLS/bin:$PATH"

ensure_hosttools() {
    command -v flex  >/dev/null || build_host flex  https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz
    command -v bison >/dev/null || build_host bison https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.gz
}

build_host() {
    name=$1; url=$2
    echo "building host tool: $name"
    mkdir -p build/src && cd build/src
    tarball=$(basename "$url")
    [ -f "$tarball" ] || wget -q "$url"
    dir=$(tar -tf "$tarball" | head -1 | cut -d/ -f1)
    [ -d "$dir" ] || tar -xf "$tarball"
    cd "$dir"
    ./configure --prefix="$HOSTTOOLS" >/dev/null && make -j"$(nproc)" >/dev/null && make install >/dev/null
    cd ../../..
}

build_dtb() {
    dtc -I dts -O dtb -o artifacts/virt_nommu.dtb virt_nommu.dts
    echo "built artifacts/virt_nommu.dtb"
}

build_kernel() {
    ensure_hosttools
    if [ ! -d "build/linux-$KVER" ]; then
        [ -f "build/linux-$KVER.tar.xz" ] || wget -q -O "build/linux-$KVER.tar.xz" "$KURL"
        tar -C build -xf "build/linux-$KVER.tar.xz"
    fi
    cd "build/linux-$KVER"
    make ARCH=riscv CROSS_COMPILE=$CROSS nommu_virt_defconfig
    ./scripts/kconfig/merge_config.sh -m .config ../../kernel_fragment.config
    make ARCH=riscv CROSS_COMPILE=$CROSS olddefconfig
    make ARCH=riscv CROSS_COMPILE=$CROSS -j"$JOBS" Image
    cd ../..
    cp "build/linux-$KVER/arch/riscv/boot/Image" artifacts/Image
    echo "built artifacts/Image"
}

BRVER=2024.02.3
BRURL="https://buildroot.org/downloads/buildroot-$BRVER.tar.gz"

build_rootfs() {
    ensure_hosttools
    if [ ! -d "build/buildroot-$BRVER" ]; then
        [ -f "build/buildroot-$BRVER.tar.gz" ] || wget -q -O "build/buildroot-$BRVER.tar.gz" "$BRURL"
        tar -C build -xf "build/buildroot-$BRVER.tar.gz"
    fi
    cd "build/buildroot-$BRVER"
    make qemu_riscv64_nommu_virt_defconfig
    # Our core is rv64ima soft-float: switch the ISA choice from G (imafdc) to
    # custom I+M+A, lp64 ABI — no FP or compressed instructions anywhere in
    # userspace. We build only the rootfs (the kernel comes from build_kernel).
    {
        echo '# BR2_riscv_g is not set'
        echo 'BR2_riscv_custom=y'
        echo 'BR2_RISCV_ISA_RVM=y'
        echo 'BR2_RISCV_ISA_RVA=y'
        echo '# BR2_RISCV_ISA_RVF is not set'
        echo '# BR2_RISCV_ISA_RVD is not set'
        echo '# BR2_RISCV_ISA_RVC is not set'
        echo 'BR2_RISCV_ABI_LP64=y'
        echo '# BR2_RISCV_ABI_LP64D is not set'
        echo '# BR2_LINUX_KERNEL is not set'
        echo 'BR2_TARGET_ROOTFS_CPIO=y'
        echo '# BR2_TARGET_ROOTFS_INITRAMFS is not set'
        echo '# BR2_PACKAGE_HOST_QEMU is not set'      # our "qemu" is the Kathryn core
    } >> .config
    make olddefconfig
    make -j"$JOBS"
    cd ../..
    cp "build/buildroot-$BRVER/output/images/rootfs.cpio" artifacts/rootfs.cpio
    echo "built artifacts/rootfs.cpio"
}

# Re-link the kernel with the initramfs embedded (INITRAMFS_SOURCE).
embed_initramfs() {
    [ -f artifacts/rootfs.cpio ] || { echo "no artifacts/rootfs.cpio — run 'rootfs' first"; exit 1; }
    cd "build/linux-$KVER"
    ./scripts/config --set-str CONFIG_INITRAMFS_SOURCE "../../artifacts/rootfs.cpio" \
                     --enable CONFIG_BLK_DEV_INITRD
    make ARCH=riscv CROSS_COMPILE=$CROSS olddefconfig
    make ARCH=riscv CROSS_COMPILE=$CROSS -j"$JOBS" Image
    cd ../..
    cp "build/linux-$KVER/arch/riscv/boot/Image" artifacts/Image
    echo "built artifacts/Image (with embedded initramfs)"
}

case "$what" in
    dtb)    build_dtb ;;
    kernel) build_kernel ;;
    rootfs) build_rootfs ;;
    embed)  embed_initramfs ;;
    all)    build_dtb; build_kernel ;;
    *)      echo "usage: build.sh [kernel|dtb|rootfs|embed|all]"; exit 2 ;;
esac
