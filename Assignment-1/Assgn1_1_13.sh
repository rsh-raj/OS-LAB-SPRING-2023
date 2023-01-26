#!/bin/bash
reverse(){
    rev=""
    for ((i=${#1}-1;i>=0;i--));do
    rev="$rev${1:$i:1}"
    done
    echo "$rev"
};

gcd(){
    case "$2" in 
    0) echo "$1";;
    *) echo `gcd $2 $(($1%$2))`;;
    esac
};

file=$(cat $1)
LCM=1
for line in $file
do
    num=`reverse $line`;
    LCM=$((num*LCM/`gcd num LCM`))
done
echo $LCM
