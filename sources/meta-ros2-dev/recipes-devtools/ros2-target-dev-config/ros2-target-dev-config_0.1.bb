SUMMARY = "Target-side ROS 2 colcon defaults for embedded development"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://defaults.yaml \
    file://ros2-target-dev-config.sh \
"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${sysconfdir}/colcon
    install -d ${D}${sysconfdir}/profile.d
    install -m 0644 ${WORKDIR}/defaults.yaml ${D}${sysconfdir}/colcon/defaults.yaml
    install -m 0755 ${WORKDIR}/ros2-target-dev-config.sh ${D}${sysconfdir}/profile.d/ros2-target-dev-config.sh
}

FILES:${PN} += " \
    ${sysconfdir}/colcon/defaults.yaml \
    ${sysconfdir}/profile.d/ros2-target-dev-config.sh \
"
