SUMMARY = "PCAM HDMI streaming application and boot-time camera initialization"
DESCRIPTION = "Builds the PCAM HDMI streaming binary and installs a systemd service to initialize the PCAM pipeline at boot."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://pcam_hdmi_streaming.c \
           file://init_pcam.sh \
           file://init-pcam.service \
"

S = "${WORKDIR}"

inherit systemd

SYSTEMD_SERVICE:${PN} = "init-pcam.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} ${WORKDIR}/pcam_hdmi_streaming.c -o pcam_hdmi_streaming
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/pcam_hdmi_streaming ${D}${bindir}/pcam_hdmi_streaming

    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/init_pcam.sh ${D}${sbindir}/init_pcam.sh

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/init-pcam.service ${D}${systemd_system_unitdir}/init-pcam.service
}

FILES:${PN} += " \
    ${bindir}/pcam_hdmi_streaming \
    ${sbindir}/init_pcam.sh \
    ${systemd_system_unitdir}/init-pcam.service \
"

RDEPENDS:${PN} += "bash media-ctl v4l-utils"
