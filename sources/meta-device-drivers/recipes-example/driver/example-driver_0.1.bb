SUMMARY = "Example out-of-tree kernel module"
DESCRIPTION = "Builds the firstModule Linux kernel module with the Yocto module class"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://firstModule.c;beginline=1;endline=1;md5=fcab174c20ea2e2bc0be64b493708266"

inherit module

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI = "file://Makefile \
           file://firstModule.c \
          "

S = "${WORKDIR}"

RPROVIDES:${PN}:append = " kernel-module-firstmodule"
