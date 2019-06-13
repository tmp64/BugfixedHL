#!/bin/bash

# Usage: GenerateAppVersion.sh [git repo dir] [file to generate] [version suffix]

repodir=$1
srcfile=$2
suffix=$3

# Get old version information from appversion.h
if [ -e ${srcfile} ]; then
	old_version=$(cat ${srcfile} | grep -i '#define APP_VERSION ' | sed -e 's/#define APP_VERSION \(.*\)/\1/i')
	old_version=${old_version//\"/}
	# "}		# This fixes xed's coloring
	# fix MC coloring '
	if [ $? -ne 0 ]; then
		old_version=""
	fi
fi

# Get information from GIT repository
git_version=$(git -C ${repodir}/. describe --long --tags --dirty --always)
git_date=$(git -C ${repodir}/. log -1 --format=%ci)

# Process version
new_version=${git_version#v}
new_version=${new_version/-dirty/+m}
new_version=${new_version/-g/+}
new_version=${new_version/-/.}

if [ ! -z "$suffix" ]; then
	# Add suffix
	new_version=${new_version}+${suffix}
fi

if [ "$new_version" != "$old_version" ]; then
	echo "Updating $srcfile, old version \"$old_version\", new version \"$new_version\"."

	echo '#ifndef __APPVERSION_H__' > ${srcfile}
	echo '#define __APPVERSION_H__' >> ${srcfile}
	echo '' >> ${srcfile}

	echo -n '#define APP_VERSION ' >> ${srcfile}
	echo "\"$new_version\"" >> ${srcfile}

	echo -n '#define APP_VERSION_DATE ' >> ${srcfile}
	echo "\"$git_date\"" >> ${srcfile}

	echo '' >> ${srcfile}
	echo '#endif //__APPVERSION_H__' >> ${srcfile}
fi
