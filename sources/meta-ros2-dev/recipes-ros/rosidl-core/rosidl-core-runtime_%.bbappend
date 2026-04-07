# Keep the Jazzy interface runtime free of Python generators on 32-bit Zynq.
# Otherwise interface packages discover rosidl_generator_py during configure and
# try to enable Python/NumPy-based generation again.
ROS_EXPORT_DEPENDS:remove = " rosidl-generator-py"
