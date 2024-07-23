
ifeq ($(PACKAGE_TOP),)
ifneq ($(EPICS_PACKAGE_TOP),)
	PACKAGE_TOP=$(EPICS_PACKAGE_TOP)
else
	$(error PACKAGE_TOP or EPICS_PACKAGE_TOP must be provided by the environment or on the command line)
endif
endif

CPSW_DIR=$(PACKAGE_TOP)/cpsw/framework/R4.4.1/src

CCACHE_DISABLE=1

SRCDIR=.
-include $(SRCDIR)/release.mak

include $(CPSW_DIR)/defs.mak

INSTALL_DIR=../

HEADERS += atcaCommon.h
HEADERS += crossbarControlYaml.hh

commonATCA_SRCS += atcaCommon.cc
commonATCA_SRCS += crossbarControlYaml.cc
commonATCA_LIBS = $(CPSW_LIBS)



SHARED_LIBRARIES_YES+=commonATCA
STATIC_LIBRARIES_YES+=commonATCA


# PROGRAMS=

include $(CPSW_DIR)/rules.mak
