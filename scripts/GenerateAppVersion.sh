#!/bin/bash

# Usage: GenerateAppVersion.sh [git repo dir] [file to generate] [version suffix]

repodir=$1
srcfile=$2
arg_major=$3
arg_minor=$4
arg_patch=$5
arg_tag=$6
arg_metadata=$7

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
git_commit_hash=$(git -C ${repodir}/. rev-parse --short HEAD)

# Process version
git_dirty_tag=''
if [[ "$git_version" == *"-dirty"* ]]; then
	git_dirty_tag='+m'
fi

if [ ! -z "$arg_tag" ]; then
	arg_tag=-${arg_tag}
fi

if [ ! -z "$arg_metadata" ]; then
	arg_metadata=+${arg_metadata}
fi

new_version=${arg_major}.${arg_minor}.${arg_patch}${arg_tag}+${git_commit_hash}${arg_metadata}${git_dirty_tag}

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
