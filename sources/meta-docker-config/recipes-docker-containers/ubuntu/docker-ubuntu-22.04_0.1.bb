SUMMARY = "Docker container workspace files"
DESCRIPTION = "Installs Dockerfile and docker-compose configuration"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI = "file://Dockerfile file://docker-compose.yaml"

WS_DIR = "/root/docker-ubuntu"

do_install() {
    install -d ${D}${WS_DIR}

    install -m 0644 ${WORKDIR}/Dockerfile ${D}${WS_DIR}/Dockerfile
    install -m 0644 ${WORKDIR}/docker-compose.yaml ${D}${WS_DIR}/docker-compose.yaml
}

FILES:${PN} += "${WS_DIR}"
