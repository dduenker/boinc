#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2017 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#
#
# Script to build the different targets in the BOINC xcode project using a
# combined install directory for all dependencies
#
# Usage: ./mac_build/buildTravis.sh [--cache_dir PATH] [--debug] [--clean]
#
# The PATH specified with --cache_dir needs to be the path where the
# dependencies are installed.
# If --debug is given the script builts the debug version of all targets
# the --clean argument will force a full rebuild.

# check working directory because the script needs to be called like: ./mac_build/buildTravis.sh
if [ ! -d "mac_build" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

# Delete any obsolete paths to old build products
rm -fR ./zip/build
rm -fR ./mac_build/build

cache_dir="$(pwd)/3rdParty/buildCache/mac"
style="Deployment"
config=""
doclean=""
beautifier="cat" # we need a fallback if xcpretty is not available
allatonce=""
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        --debug|-dev)
        style="Development"
        config="-dev"
        ;;
        # non public option
        --allatonce)
        allatonce="yes"
        ;;
    esac
    shift # past argument or value
done

if [ ! -d "$cache_dir" ] || [ ! -d "$cache_dir/lib" ] || [ ! -d "$cache_dir/include" ]; then
    echo "${cache_dir} is not a directory or does not contain dependencies"
fi

XCPRETTYPATH=`xcrun -find xcpretty`
if [ $? -eq 0 ]; then
    beautifier="xcpretty"
fi

cd ./mac_build
retval=0

if [ ${allatonce} = "yes" ]; then
    ## If all targets can share the same header and library search paths
    libSearchPathDbg=""
    if [ "${style}" == "Development" ]; then
        libSearchPathDbg="${cache_dir}/lib/debug"
    fi
    source BuildMacBOINC.sh ${config} -all -setting HEADER_SEARCH_PATHS "../clientgui ${cache_dir}/include ../samples/jpeglib ${cache_dir}/include/freetype2" -setting USER_HEADER_SEARCH_PATHS "" -setting LIBRARY_SEARCH_PATHS "./build/Development $libSearchPathDbg ${cache_dir}/lib ../lib" | $beautifier; retval=${PIPESTATUS[0]}
    if [ $retval -ne 0 ]; then cd ..; exit 1; fi

    return 0
fi

## targets need different header or library search paths, so build individually
if [ "${doclean}" = "yes" ]; then
    ## clean all targets
    xcodebuild -project boinc.xcodeproj -target Build_All  -configuration ${style} clean | $beautifier; retval=${PIPESTATUS[0]}
    if [ $retval -ne 0 ]; then cd ..; exit 1; fi

    ## clean boinc_zip which is not included in Build_All
    xcodebuild -project ../zip/boinc_zip.xcodeproj -target boinc_zip -configuration ${buildStyle} clean | $beautifier; retval=${PIPESTATUS[0]}
    if [ $retval -ne 0 ]; then cd ..; exit 1; fi
fi

## Target mgr_boinc also builds dependent targets SetVersion and BOINC_Client
libSearchPathDbg=""
if [ "${style}" == "Development" ]; then
    libSearchPathDbg="${cache_dir}/lib/debug"
fi
source BuildMacBOINC.sh ${config} -noclean -target mgr_boinc -setting HEADER_SEARCH_PATHS "../clientgui ${cache_dir}/include" -setting LIBRARY_SEARCH_PATHS "${libSearchPathDbg} ../${cache_dir}/lib" | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then cd ..; exit 1; fi

## Target gfx2libboinc also build dependent target jpeg
source BuildMacBOINC.sh ${config} -noclean -target gfx2libboinc -setting HEADER_SEARCH_PATHS "../samples/jpeglib" | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target libboinc | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target api_libboinc | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target PostInstall | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target switcher | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target gfx_switcher | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target Install_BOINC | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

libSearchPath="./build/Deployment"
if [ "${style}" == "Development" ]; then
    libSearchPath="./build/Development"
fi
source BuildMacBOINC.sh ${config} -noclean -target ss_app -setting HEADER_SEARCH_PATHS "../api/ ../samples/jpeglib/ ${cache_dir}/include .${cache_dir}/include/freetype2"  -setting LIBRARY_SEARCH_PATHS "${libSearchPath} ${cache_dir}/lib" | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target boinc_opencl | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target setprojectgrp | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target cmd_boinc | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target Uninstaller | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target SetUpSecurity | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

source BuildMacBOINC.sh ${config} -noclean -target AddRemoveUser | $beautifier; retval=${PIPESTATUS[0]}
if [ $retval -ne 0 ]; then cd ..; exit 1; fi

cd ..
