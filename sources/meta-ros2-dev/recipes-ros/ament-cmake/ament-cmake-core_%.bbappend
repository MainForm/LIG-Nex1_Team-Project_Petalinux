do_install:append:class-target() {
    install -d ${D}${ros_prefix}

    _ament_prefix_template_dir="${STAGING_DIR_NATIVE}${ros_prefix}/lib/python${PYTHON_BASEVERSION}/site-packages/ament_package/template/prefix_level"

    install -m 0644 ${_ament_prefix_template_dir}/setup.bash ${D}${ros_prefix}/setup.bash
    install -m 0644 ${_ament_prefix_template_dir}/local_setup.bash ${D}${ros_prefix}/local_setup.bash
    install -m 0644 ${_ament_prefix_template_dir}/setup.zsh ${D}${ros_prefix}/setup.zsh
    install -m 0644 ${_ament_prefix_template_dir}/local_setup.zsh ${D}${ros_prefix}/local_setup.zsh
    install -m 0644 ${_ament_prefix_template_dir}/_local_setup_util.py ${D}${ros_prefix}/_local_setup_util.py

    sed \
        -e 's|@CMAKE_INSTALL_PREFIX@|${ros_prefix}|g' \
        ${_ament_prefix_template_dir}/setup.sh.in \
        > ${D}${ros_prefix}/setup.sh

    sed \
        -e 's|@CMAKE_INSTALL_PREFIX@|${ros_prefix}|g' \
        -e 's|@ament_package_PYTHON_EXECUTABLE@|/usr/bin/python3|g' \
        ${_ament_prefix_template_dir}/local_setup.sh.in \
        > ${D}${ros_prefix}/local_setup.sh

    chmod 0644 ${D}${ros_prefix}/setup.sh ${D}${ros_prefix}/local_setup.sh
}

FILES:${PN}:append:class-target = " \
    ${ros_prefix}/_local_setup_util.py \
    ${ros_prefix}/local_setup.bash \
    ${ros_prefix}/local_setup.sh \
    ${ros_prefix}/local_setup.zsh \
    ${ros_prefix}/setup.bash \
    ${ros_prefix}/setup.sh \
    ${ros_prefix}/setup.zsh \
"
