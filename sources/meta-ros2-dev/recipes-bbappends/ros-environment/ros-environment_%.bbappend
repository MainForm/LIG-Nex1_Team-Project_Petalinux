do_install:append() {
    ros_env_dir="${D}${ros_prefix}/share/ros_environment/environment"

    # Some generated ros_environment builds carry over "rolling" in the
    # runtime hooks even when this build targets Jazzy. Keep the runtime
    # value aligned with the selected distro.
    if [ -d "${ros_env_dir}" ]; then
        sed -i "s/^export ROS_DISTRO=.*/export ROS_DISTRO=${ROS_DISTRO}/" \
            "${ros_env_dir}/1.ros_distro.sh"
        sed -i "s/^set;ROS_DISTRO;.*/set;ROS_DISTRO;${ROS_DISTRO}/" \
            "${ros_env_dir}/1.ros_distro.dsv"
    fi
}
