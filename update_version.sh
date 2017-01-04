#!/bin/bash
if [ $# -ne 1 ];
then
	echo "You need to specify the version number"
	echo "The last version was" `git tag | tail -1`
	exit 1
fi
version=$1
cat << EOF > ./include/uefiop_version.h
/*
 * Copyright (C) 2017 Ivan Hu <ivan.hu@canonical.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
EOF
echo '#define UEFIOP_VERSION "'$version'"' >> ./include/uefiop_version.h
echo '#define UEFIOP_DATE    "'`date --utc "+%F %T"`'"' >> ./include/uefiop_version.h
git add ./include/uefiop_version.h
git commit -s -m"uefiop_version.h - update to $version"
git tag -m'"Version '$1'"' $1
