# [LIG Nex1] PetaLinux Team Project

This repository provides the PetaLinux build environment for the LIG Nex1 team project on the Zybo Z7-10 platform.

## Features
1. PetaLinux is built using the Yocto Project workflow.
2. Docker support is enabled in this PetaLinux environment.
3. The system uses `systemd` as the init system.
4. The Linux image includes support for the ipTIME A3000 Mini Wi-Fi dongle.

## Repository Layout

```text
.
├── AMD_EDF/
├── build_zybo-z7-10/
└── sources/
    └── meta-zybo-z7-10/
```


## Initial Setup

AMD EDF is based on the Xilinx `yocto-manifests` workflow and uses the
`repo` tool to fetch the full layer stack.

Install the required host packages for the Yocto build environment:
```bash
sudo apt install build-essential chrpath cpio debianutils diffstat file gawk gcc git iputils-ping libacl1 liblz4-tool locales python3 python3-git python3-jinja2 python3-pexpect python3-pip python3-subunit socat texinfo unzip wget xz-utils zstd
```

Make sure the `en_US.UTF-8` locale is available:
```bash
locale --all-locales | grep en_US.utf8
````

If you run into issues while preparing the host environment, refer to the
[Yocto Project system requirements documentation](https://docs.yoctoproject.org/scarthgap/ref-manual/system-requirements.html).

Install `repo`:

```bash
curl https://storage.googleapis.com/git-repo-downloads/repo > repo
chmod a+x repo
mkdir -p ~/bin
mv repo ~/bin/
export PATH=~/bin:$PATH
repo --help
```

Sync the AMD EDF workspace:

```bash
./setupEDF.sh
```

Initialize the Zybo Z7-10 layer submodule:

```bash
git submodule update --init --recursive
```


## Build Environment

Set up the Yocto build environment:

```bash
cd AMD_EDF
source setupsdk ../build_zybo-z7-10
```

The build is configured to use:

- `MACHINE = "zybo-z7-10"`
- the local `meta-zybo-z7-10` layer
- a local `system.xsa` through `external-hdf`
- a Digilent-based kernel recipe


## Build

After sourcing the environment, build an image with BitBake as usual. For
example:

```bash
bitbake petalinux-image-minimal
```

Use `bitbake -e virtual/kernel` or `bitbake -e device-tree` to confirm the
selected kernel and device tree configuration when debugging board bring-up.

## Install Linux

If `bmaptool` is not installed on your host system, install it first:

```bash
sudo apt install bmaptool
```

The output image files are generated under
`./build_zybo-z7-10/tmp/deploy/images/zybo-z7-10/`.
The main image produced by Yocto is
`petalinux-image-minimal-zybo-z7-10.wic`.

IMPORTANT: Double-check the target device before running the write command.
Writing the image to the wrong device will permanently overwrite its data.

```bash
# Identify the target device node for the SD card
lsblk

# Example output
NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
nvme0n1 259:0   0 476.9G  0 disk
├─nvme0n1p1
│      259:1   0   512M  0 part /boot/efi
├─nvme0n1p2
│      259:2   0   100G  0 part /
└─nvme0n1p3
       259:3   0 376.4G  0 part /home
sde      8:64  1  29.7G  0 disk
└─sde1   8:65  1  29.7G  0 part /media/user/SDCARD

# In this example, the SD card appears as /dev/sde, so that device is used below.
sudo bmaptool copy ./build_zybo-z7-10/tmp/deploy/images/zybo-z7-10/petalinux-image-minimal-zybo-z7-10.wic /dev/sde
```

## Notes

- `meta-zybo-z7-10` contains the board-specific machine, kernel, and device
  tree customizations.
- `build_zybo-z7-10/conf/` contains the active local build configuration.
- The AMD EDF workspace may generate `.repo/` metadata; this is workspace
  state, not board support metadata.
- Before developing a device driver or building an out-of-tree kernel module
  on the target, prepare the kernel source tree with:
  `cd /usr/src/kernel && make scripts prepare modules_prepare`
