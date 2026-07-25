#!/bin/sh
# One-command, cached Linux image build for the Kathryn RV64 nommu SoC.
#   tools/linux_image/build.sh [kernel|dtb|all]
# Artifacts land in tools/linux_image/artifacts/ (gitignored):
#   Image        — kernel binary (load at 0x80000000)
#   virt_nommu.dtb — device tree  (load at 0x87E00000, matches boot_regs a1)
set -e
cd "$(dirname "$0")"

KVER=6.6.32
KURL="https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-$KVER.tar.xz"
CROSS=riscv64-unknown-elf-
JOBS=$(nproc)

mkdir -p artifacts build
what="${1:-all}"

build_dtb() {
    dtc -I dts -O dtb -o artifacts/virt_nommu.dtb virt_nommu.dts
    echo "built artifacts/virt_nommu.dtb"
}

build_kernel() {
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

case "$what" in
    dtb)    build_dtb ;;
    kernel) build_kernel ;;
    all)    build_dtb; build_kernel ;;
    *)      echo "usage: build.sh [kernel|dtb|all]"; exit 2 ;;
esac
