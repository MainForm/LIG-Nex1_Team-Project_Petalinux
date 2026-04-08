SUMMARY = "Simple ROS 2 C++ publisher and subscriber example"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://package.xml;beginline=9;endline=9;md5=82f0323c08605e5b6f343b05213cf7cc"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit ros_distro_jazzy

ROS_BUILD_TYPE = "ament_cmake"
ROS_CN = "cpp_pubsub"
ROS_BPN = "cpp_pubsub"

inherit ros_${ROS_BUILD_TYPE}

SRC_URI = "file://cpp-pubsub"
S = "${WORKDIR}/cpp-pubsub"

ROS_BUILD_DEPENDS = " \
    rclcpp \
    std-msgs \
"

ROS_BUILDTOOL_DEPENDS = " \
    ament-cmake-native \
"

ROS_EXEC_DEPENDS = " \
    rclcpp \
    std-msgs \
"

DEPENDS = "${ROS_BUILD_DEPENDS} ${ROS_BUILDTOOL_DEPENDS}"
RDEPENDS:${PN} += "${ROS_EXEC_DEPENDS}"
