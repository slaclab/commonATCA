CPSW_DIR=/afs/slac/g/lcls/package/cpsw/framework/R4.3.1/src

CCACHE_DISABLE=1

SRCDIR=.
-include $(SRCDIR)/release.mak

include $(CPSW_DIR)/defs.mak

INSTALL_DIR=../

HEADERS += atcaCommon.h

commonATCA_SRCS += atcaCommon.cc
commonATCA_LIBS = $(CPSW_LIBS)



SHARED_LIBRARIES_YES+=commonATCA
STATIC_LIBRARIES_YES+=commonATCA


# PROGRAMS=

include $(CPSW_DIR)/rules.mak
