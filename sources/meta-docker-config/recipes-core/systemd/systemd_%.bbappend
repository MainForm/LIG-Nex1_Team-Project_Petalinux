do_install:append() {
    ln -sf /dev/null ${D}${sysconfdir}/systemd/system/systemd-networkd-wait-online.service
}