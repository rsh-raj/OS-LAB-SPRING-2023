#!/bin/bash
if [ -z $2 ] || [ -z $1 ]; then
    echo "Input or output directory is missing"
    exit
fi
if [ ! -d $2 ]; then
    `mkdir $2`
fi
for letter in {a..z};do
outputFile=$2"/"$letter".txt"
if [ ! -f outputFile ]; then
            `touch $outputFile`
    
fi
done
for file in `ls $1 | grep .*"\.txt$"`; do
    file="$1/$file"
    while read -r line; do
        outputFile=$2"/"${line:0:1}".txt"
        echo $line >> $outputFile 
    done<$file
done
cd $2
for file in `ls | grep .*"\.txt$"`; do
    `sort -o $file $file`
done