# Disable Python ROS interface generation on this 32-bit Zynq target.
# The Jazzy/meta-ros stack pulls in rosidl_generator_py by default, which
# causes target interface packages to compile generated C against Python.h from
# the native sysroot and fail pyport.h's LONG_BIT sanity check.
ROS_BUILDTOOL_EXPORT_DEPENDS:remove = " rosidl-generator-py-native"
