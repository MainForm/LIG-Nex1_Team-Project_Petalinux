# Keep Python ROS interface generation enabled.
# The original workaround removed rosidl_generator_py entirely because Jazzy's
# rosidl_generator_py could pick native Python headers during cross builds on
# 32-bit Zynq + Python 3.12. A local rosidl_generator_py bbappend now patches
# that CMake logic to use target-aware Python3 imported targets instead.
