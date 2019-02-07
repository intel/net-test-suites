#!/usr/bin/env bash

DEST_DIR=~/titan

if [ ! -z "${1}" ]; then
    DEST_DIR=${1}
fi

if [ -f /etc/lsb-release ]; then
    FILE=ttcn3-6.5.pl0-linux64-gcc5.4.0-ubuntu16.04.tgz
elif [ -f /etc/redhat-release ]; then
    FILE=ttcn3-6.5.pl0-linux64-gcc4.8.2-RHEL7.tgz
fi

if [ -z "${FILE}" ]; then
    echo "Unable to detect Linux distribution"
    return
fi

wget http://download.eclipse.org/titan/${FILE}

[ ! -f ${FILE} ] && return

mkdir ${DEST_DIR}

tar xf ${FILE} -C ${DEST_DIR}

rm -fv ${FILE}


