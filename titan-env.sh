#!/usr/bin/env bash

export TTCN3_DIR=${1:-~/titan}

[[ $# -gt 0 ]] && echo TTCN3_DIR=$TTCN3_DIR

export PATH=${TTCN3_DIR}/bin:${PATH}
