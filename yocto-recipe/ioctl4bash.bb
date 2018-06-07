###############################################################################
##                                                                           ##
##                 Yocto-recipe to building ioctl for bash                   ##
##                                                                           ##
##---------------------------------------------------------------------------##
## Author:   Ulrich Becker                                                   ##
## Date:     07.06.2018                                                      ##
## Revision:                                                                 ##
###############################################################################

SUMMARY          = "Tool to perform the c- function ioctl() in a console"

SECTION          = "tools"
S                = "${WORKDIR}/git"
LICENSE          = "GPLv3"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=84dcc94da3adb52b53ae4fa38fe49e5d"
SRC_URI          = "git://github.com/UlrichBecker/ioctl4bash.git;branch=master"
SRCREV           = "${AUTOREV}"
PV               = "1.0-git${SRCPV}"
PR               = "r0"
INSTALL_DIR      = "${bindir}"
BIN_NAME         = "ioctl"

FILES_${PN} += "${INSTALL_DIR}"
FILES_${PN} += "${INSTALL_DIR}/${BIN_NAME}"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile() {
   oe_runmake BASEDIR="${S}" CFLAGS="-Os" EXE_NAME="${BIN_NAME}"
}

do_install() {
   install -d ${D}${INSTALL_DIR}
   install -m 0755 ${S}/${BIN_NAME} ${D}${INSTALL_DIR}
}

#=================================== EOF ======================================
