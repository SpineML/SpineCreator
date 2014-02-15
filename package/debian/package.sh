#!/bin/bash

################################################################################
#
# Making a debian package of spinecreator
#
#

# Before you start, here are the dependencies:
# sudo apt-get install build-essential autoconf automake autotools-dev
#                      dh-make debhelper devscripts fakeroot xutils
#                      lintian pbuilder cdbs
#
# You also need to modify changelog.spinecreator, to say why the code
# is being packaged again.
#


# Some parameters.
export DEBEMAIL="seb.james@sheffield.ac.uk"
export DEBFULLNAME="Sebastian Scott James"
PACKAGE_MAINTAINER_GPG_IDENTITY="DEBFULLNAME <$DEBEMAIL>"
CURRENT_YEAR=`date +%Y`

# Check we're being called the right way.
if [ -z "$1" ]; then
    echo "Usage: package.sh <versionnum>"
    exit 1
fi

VERSION="$1"

# Get git revision information
pushd ../SpineCreator
GIT_BRANCH=`git branch| grep \*| awk -F' ' '{ print $2; }'`
GIT_LAST_COMMIT_SHA=`git log -1 --oneline | awk -F' ' '{print $1;}'`
GIT_LAST_COMMIT_DATE=`git log -1 | grep Date | awk -F 'Date:' '{print $2;}'| sed 's/^[ \t]*//'`
popd

# How many processors do we have?
PROCESSORS=`grep "^physical id" /proc/cpuinfo | sort -u | wc -l`
CORES_PER_PROC=`grep "^core id" /proc/cpuinfo | sort -u | wc -l`
CORES=$((PROCESSORS * CORES_PER_PROC))

# Ensure spec file exists
pushd ../SpineCreator
qmake-qt4 neuralNetworks.pro -r -spec linux-g++
make clean
popd

################################################################################
#
# Setting up the package. See http://www.debian.org/doc/manuals/maint-guide/first.en.html
#
#

# The deb source directory will be created with this directory name
DEBNAME=spinecreator-$VERSION

# The "orig" tarball will have this name
DEBORIG=spinecreator_$VERSION.orig

# Clean up generated tarballs and files
rm -rf $DEBNAME
rm -f $DEBNAME.tar.gz
rm -f $DEBORIG.tar.gz
rm -f spinecreator_$VERSION-[0-9].debian.tar.gz
rm -f spinecreator_$VERSION-[0-9].dsc
rm -f spinecreator_$VERSION-[0-9]_amd64.changes
rm -f spinecreator_$VERSION-[0-9]_i386.changes
rm -f spinecreator_$VERSION-[0-9]_amd64.deb
rm -f spinecreator_$VERSION-[0-9]_i386.deb

# Create our "upstream" tarball from the git repo
rm -rf ../$DEBNAME
cp -Ra ../SpineCreator ../$DEBNAME # Note: SpineCreator tarball has to be spinecreator-0.9.3
tar czf $DEBNAME.tar.gz --exclude-vcs -C.. $DEBNAME

# Clean up our source directory and then create it and pushd into it
mkdir -p $DEBNAME
pushd $DEBNAME

# Run dh_make.
dh_make -s -f ../$DEBNAME.tar.gz

################################################################################
#
# Modifying the source. See http://www.debian.org/doc/manuals/maint-guide/modify.en.html
#
#

# Set up quilt
alias dquilt="quilt --quiltrc=${HOME}/.quiltrc-dpkg"
complete -F _quilt_completion $_quilt_complete_opt dquilt
QUILTRC_DPKG="$HOME/.quiltrc-dpkg"
if [ ! -f $QUILTRC_DPKG ]; then
    cat > $QUILTRC_DPKG <<EOF
d=. ; while [ ! -d \$d/debian -a `readlink -e \$d` != / ]; do d=\$d/..; done
if [ -d \$d/debian ] && [ -z \$QUILT_PATCHES ]; then
    # if in Debian packaging tree with unset \$QUILT_PATCHES
    QUILT_PATCHES="debian/patches"
    QUILT_PATCH_OPTS="--reject-format=unified"
    QUILT_DIFF_ARGS="-p ab --no-timestamps --no-index --color=auto"
    QUILT_REFRESH_ARGS="-p ab --no-timestamps --no-index"
    QUILT_COLORS="diff_hdr=1;32:diff_add=1;34:diff_rem=1;31:diff_hunk=1;33:diff_ctx=35:diff_cctx=33"
    if ! [ -d \$d/debian/patches ]; then mkdir \$d/debian/patches; fi
fi
EOF
fi

# NB: We should have no upstream bugs to fix, as we ARE the upstream maintainers.

################################################################################
#
# Debian files. See http://www.debian.org/doc/manuals/maint-guide/dreq.en.html
#
#

# Remove example files
rm -rf debian/*.ex
rm -rf debian/*.EX

# We don't need a README.Debian file to describe special instructions
# about running this softare on Debian.
rm -f debian/README.Debian

# Create the correct control file
# Figure out the dependencies using:
# objdump -p /path/to/spinecreator | grep NEEDED
# And for each line dpkg -S library.so.X
#
# NB: I'll add Brahms to the Recommends line, when I've created a debian package for it.
#
cat > debian/control <<EOF
Source: spinecreator
Section: x11
Priority: optional
Maintainer: $PACKAGE_MAINTAINER_GPG_IDENTITY
Build-Depends: debhelper (>= 8.0.0), qt4-qmake, libc6-dev, libstdc++-dev, libglu1-mesa-dev, libqt4-dev, libqt4-opengl-dev, libgvc5, libgraph4
Standards-Version: 3.9.3
Homepage: http://bimpa.group.shef.ac.uk/SpineML/index.php/SpineCreator_-_A_Graphical_Tool

Package: spinecreator
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}
Recommends: xsltproc, gcc
Description:  GUI for SpineML.
 Create, visualise and simulate networks of point spiking neural models.
 For use with the SpineML XML format and compatible simulators.
EOF

# A function for copying files in from the source tree (ones stored in package/debian/)
copyin()
{
    if [ -z "$1" ]; then
        echo "Call copyin with 1 argument."
        exit
    fi
    THEFILE="$1"
    if [ ! -f ../../SpineCreator/package/debian/$THEFILE ]; then
        echo "You need to create/update the $THEFILE file (in the SpineCreator source)"
        exit
    fi
    cat ../../SpineCreator/package/debian/$THEFILE > debian/$THEFILE
}

# Copy in the changelog
copyin "changelog"

# and the manpage
copyin "spinecreator.1"

# menu
cat > debian/menu <<EOF
?package(spinecreator):needs="X11" section="Applications/Science/Biology"\
  title="spinecreator" command="/usr/bin/spinecreator"

EOF

# The copyright notice
cat > debian/copyright <<EOF
Format: http://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: spinecreator
Source: https://github.com/SpineML/SpineCreator

# Upstream copyright:
Files: *
Copyright: 2013-2014 Alex Cope <a.cope@sheffield.ac.uk>
           2014 Seb James <seb.james@sheffield.ac.uk>
License: GPL
 This package is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 .
 This package is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 .
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>
 .
 On Debian systems, the complete text of the GNU General
 Public License version 3 can be found in "/usr/share/common-licenses/GPL-3".

# Copyright in the package files:
Files: debian/*
Copyright: $CURRENT_YEAR $DEBEMAIL
License: GPL
 This package is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 .
 This package is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 .
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>
 .
 On Debian systems, the complete text of the GNU General
 Public License version 3 can be found in "/usr/share/common-licenses/GPL-3".
EOF

# The source readme
cat > debian/README.source <<EOF
spinecreator for Debian
-----------------------

This package was produced from a source tarball built from the git repository
at https://github.com/SpineML/SpineCreator.

The git commit revision is: $GIT_LAST_COMMIT_SHA of $GIT_LAST_COMMIT_DATE on
the $GIT_BRANCH branch.
EOF

# The rules for building. Note - using cdbs here.
cat > debian/rules <<EOF
#!/usr/bin/make -f
include /usr/share/cdbs/1/rules/debhelper.mk

# The following is:
#  include /usr/share/cdbs/1/class/qmake.mk
# with https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=695367 applied
# and the comments stripped out.

_cdbs_scripts_path ?= /usr/lib/cdbs
_cdbs_rules_path ?= /usr/share/cdbs/1/rules
_cdbs_class_path ?= /usr/share/cdbs/1/class

ifndef _cdbs_class_qmake
_cdbs_class_qmake = 1

include \$(_cdbs_class_path)/makefile.mk\$(_cdbs_makefile_suffix)

# FIXME: Restructure to allow early override
DEB_MAKE_EXTRA_ARGS = \$(DEB_MAKE_PARALLEL)

DEB_MAKE_INSTALL_TARGET = install INSTALL_ROOT=\$(DEB_DESTDIR)
DEB_MAKE_CLEAN_TARGET = distclean

QMAKE ?= qmake

ifneq (,\$(filter nostrip,\$(DEB_BUILD_OPTIONS)))
DEB_QMAKE_CONFIG_VAL ?= nostrip
endif

common-configure-arch common-configure-indep:: common-configure-impl
common-configure-impl:: \$(DEB_BUILDDIR)/Makefile
\$(DEB_BUILDDIR)/Makefile:
	cd \$(DEB_BUILDDIR) && \$(QMAKE) \$(DEB_QMAKE_ARGS) \$(if \$(DEB_QMAKE_CONFIG_VAL),'CONFIG += \$(DEB_QMAKE_CONFIG_VAL)') 'QMAKE_CC = \$(CC)' 'QMAKE_CXX = \$(CXX)' 'QMAKE_CFLAGS_RELEASE = \$(CPPFLAGS) \$(CFLAGS)' 'QMAKE_CXXFLAGS_RELEASE = \$(CPPFLAGS) \$(CXXFLAGS)' 'QMAKE_LFLAGS_RELEASE = \$(LDFLAGS)'

clean::
	rm -f \$(DEB_BUILDDIR)/Makefile \$(DEB_BUILDDIR)/.qmake.internal.cache

endif

EOF

popd


################################################################################
#
# Unpack debian orig source code files
#
#

echo "unpacking $DEBORIG.tar.gz:"
tar xvf $DEBORIG.tar.gz

# Set up compiler dpkg-buildflags
export CPPFLAGS=`dpkg-buildflags --get CPPFLAGS`
export CFLAGS=`dpkg-buildflags --get CFLAGS`
export CXXFLAGS=`dpkg-buildflags --get CXXFLAGS`
export LDFLAGS=`dpkg-buildflags --get LDFLAGS`
export DEB_BUILD_HARDENING=1

echo "Ready to build..."
pushd $DEBNAME
echo " dpkg-buildpackage -j$CORES -rfakeroot"
dpkg-buildpackage -j$CORES -rfakeroot
popd
