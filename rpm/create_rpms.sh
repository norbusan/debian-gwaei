#!/bin/bash

# Description : Script to build rpm from a source distribution
# Parameter :
# $1 = Script started directory.
# $2 = Program name
# $3 = Version
# $4 = Build for "distribution name" or "all".

# Set global variables
topdir=$1
scriptdir=$(dirname $0)
scriptname=$(basename $0)
prog=$2
version=$3
tarpkgname=$prog-$version.tar.gz
rpmtimestamp=$(LANG=C && date "+%a %b %d %Y")

distro=$(find $scriptdir -maxdepth 1 -type d | grep -v "^$scriptdir$" | sed 's#^\./##')


# Create rpm for distros
for i in $distro
do
	# Define variables
	toprpmdir=$topdir/$distro
	echo "Processing : $toprpmdir"

	# 1-Create build environment
	# 1a- Define rpmmacros
	if [ -f ~/.rpmmacros ]
		then
		# Backup the original env
		typeset -i backup=1
		cp -f ~/.rpmmacros ~/.rpmmacros.backup
	fi

	echo "%_topdir      $toprpmdir" > ~/.rpmmacros
	echo "%_smp_mflags  -j3" >> ~/.rpmmacros
	echo "%__arch_install_post   /usr/lib/rpm/check-rpaths   /usr/lib/rpm/check-buildroot" >> ~/.rpmmacros

	# 1b- Create all rpmbuild directories
	mkdir -p $toprpmdir/SOURCES $toprpmdir/BUILD $toprpmdir/BUILDROOT $toprpmdir/RPMS $toprpmdir/SRPMS $toprpmdir/log

	# 1c- Move sources and patches
	cp -f $topdir/$tarpkgname $toprpmdir/SOURCES
	cp -f $toprpmdir/patches/* $toprpmdir/SOURCES

	# 1d- Insert time stamp into spec file
	sed -i "s/<DATE>/$rpmtimestamp/" $toprpmdir/SPECS/$prog.spec

	# 2-Now build
	rpmbuild -ba $toprpmdir/SPECS/$prog.spec 2>&1 | tee $toprpmdir/log/build.log
	
	# 3-Restore original rpmmacros if needed
	if [ $backup -eq 1 ]
		then 
		cp -f ~/.rpmmacros.backup ~/.rpmmacros
		rm -f ~/.rpmmacros.backup
	fi

	# 4-Check result	
	grep "Erreur de construction de RPM" $toprpmdir/log/build.log
	if [ "$?" -eq 0 ]
		then
		echo "Error building rpm"
		exit 1
	fi
	echo "Done"
done
exit

# Example from Makefile.am
#rpm: dist
#        @ make clean > /dev/null
#        @ touch $(HOME)/.rpmmacros
#        @ cp -f $(HOME)/.rpmmacros $(HOME)/.rpmmacros.backup
#        @ echo "%_topdir $(PWD)/rpm" > $(HOME)/.rpmmacros
#        @ mkdir -p $(top_srcdir)/rpm/SOURCES $(top_srcdir)/rpm/SPECS $(top_srcdir)/rpm/BUILD $(top_srcdir)/rpm/RPMS/@ARCH@ $(top_srcdir)/rpm/SRPMS/@ARCH@
#        @ cp @PACKAGE@-@VERSION@.tar.gz $(top_srcdir)/rpm/SOURCES
#        @ ${RPMBUILD} -ba --target @ARCH@ $(top_srcdir)/rpm/@PACKAGE@.spec
#        @ cp $(RPM_RPMS)/@ARCH@/@PACKAGE@-@VERSION@-@RELEASE@.@ARCH@.rpm $(top_srcdir)
#        @ rm -rf $(top_srcdir)/rpm/SOURCES $(top_srcdir)/rpm/SPECS $(top_srcdir)/rpm/BUILD $(top_srcdir)/rpm/RPMS $(top_srcdir)/rpm/SRPMS
#        @ rm -f $(HOME)/.rpmmacros
#        @ mv -f $(HOME)/.rpmmacros.backup $(HOME)/.rpmmacros
#        @ echo "Success! Finished creating the rpm packge. Please have a sugary day."
#
#fedora-rpm:
#        @ $(top_srcdir)/configure --prefix /usr --sysconfdir /etc --docdir=/usr/share/doc/@PACKAGE@ > /dev/null
#        @ make rpm

