#!/bin/bash

mkdir -p AMD_EDF
cd AMD_EDF

repo init -u https://github.com/Xilinx/yocto-manifests.git -b rel-v2025.2

repo sync