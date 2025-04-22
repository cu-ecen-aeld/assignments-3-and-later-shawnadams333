#!/usr/bin/env bash

usage_and_exit() {
    echo "USAGE: $0 <directory-to-search> <search-string>"
    exit 1
}

if [[ $# -lt 2 ]]; then
    echo "you missed at least one argument"
    usage_and_exit
fi

if [[ ! -d $1 ]]; then
    echo "$1 is not a directory"
    usage_and_exit
fi

filesdir=$1
searchstr=$2

counts=$(find $filesdir -type f -exec grep -c $searchstr {} \;)
num_files=0
num_matches=0
for count in $counts; do
    ((num_files++))
    num_matches=$((num_matches + $count))
done

echo "The number of files are $num_files and the number of matching lines are $num_matches"
