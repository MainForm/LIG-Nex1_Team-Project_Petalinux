FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SYSTEMD_AUTO_ENABLE:${PN} = "enable"
SYSTEMD_SERVICE:append:${PN} = " docker.service"

SRC_URI:append= " file://docker.service"