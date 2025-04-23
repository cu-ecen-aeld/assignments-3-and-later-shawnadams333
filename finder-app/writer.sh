#!/usr/bin/env bash

usage_and_exit() {
    echo "USAGE: $0 <file-to-write> <string-to-write>"
    exit 1
}

if [[ $# -lt 2 ]]; then
    echo "you missed at least one argument"
    usage_and_exit
fi

writefile=$1
writestr=$2

dir=$(dirname $writefile)

if [[ ! -e $dir ]]; then
    mkdir -p $(dirname $writefile)
fi

echo $writestr > $writefile

if [[ ! -e $writefile ]]; then
    echo "error writing $writestr to $writefile"
    exit 1
fi

exit 0
