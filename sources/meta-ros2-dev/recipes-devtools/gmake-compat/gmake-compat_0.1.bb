SUMMARY = "Provide a gmake compatibility symlink to make"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

RDEPENDS:${PN} = "make"

do_install() {
    install -d ${D}${bindir}
    ln -sf make ${D}${bindir}/gmake
}

FILES:${PN} += "${bindir}/gmake"
