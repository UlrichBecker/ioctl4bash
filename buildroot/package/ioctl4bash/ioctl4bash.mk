###############################################################################
##                                                                           ##
##           Makefile for ioctl4bash for integrating in Buildroot            ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:   package/ioctl4bash/ioctl4bash.mk                                  ##
## Author: Ulrich Becker                                                     ##
## Date:   31.03.2025                                                        ##
###############################################################################
IOCTL4BASH_VERSION = master
IOCTL4BASH_SITE = git://github.com/UlrichBecker/ioctl4bash.git
IOCTL4BASH_SITE_METHOD = git
IOCTL4BASH_INSTALL_STAGING = YES

IOCTL4BASH_LICENSE = MIT #GPL-2.0
IOCTL4BASH_LICENSE_FILES = COPYING

IOCTL4BASH_BUILD_DIR = $(BUILD_DIR)/ioctl4bash-$(IOCTL4BASH_VERSION)
IOCTL4BASH_INSTALL_DIR = $(TARGET_DIR)/usr/bin

define IOCTL4BASH_BUILD_CMDS
   $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define IOCTL4BASH_INSTALL_TARGET_CMDS
   $(INSTALL) -d $(IOCTL4BASH_INSTALL_DIR)
   $(INSTALL) -m 0755 $(IOCTL4BASH_BUILD_DIR)/ioctl $(IOCTL4BASH_INSTALL_DIR)
endef

$(eval $(generic-package))
#=================================== EOF ======================================
