#!/bin/bash
for files in $1/*.jsonl
do  
    # make a csv file in the required location
    sx=$(basename "$files") 
    name="${sx%.*}"
    path="$2/$name.csv"

    touch $path
    
    temp=0

    # file=$(cat $files)
    # each line will have a record in the csv file
    while read -r line
    do

    # filters out the fields of the json file
    record=""
    argument=""
    i=0
    for arg in $@
    do

    i=$((i+1))
    case $i in 0|1|2) continue;;esac;
    # $arg contains the command line parameters

    # make of string of arguments and append in the csv file
    argument="$argument,$arg"

    # filter json as per the $arg
    s=".$arg"
    x=$(echo $line | jq $s)

    # if has " " then remove it
    if [[ $x =~ \".*\" ]]; then
        x="${x:1:${#x}-2}"
    fi

    # make a list of x and append them in the csv file
    record="$record,$x"
    done

    record="${record:1}"
    argument="${argument:1}"

    # append in the csv file
    if [ $temp -eq 0 ]; then
    echo $argument > $path
    temp=1
    fi

    echo $record >> $path
    # break
    done < "$files"
    # break
done