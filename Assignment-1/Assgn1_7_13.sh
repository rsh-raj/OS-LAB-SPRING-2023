#!/bin/bash
mkdir -p $2

for letter in {a..z}
do
    grep -ih ^$letter $1/*.txt | sort > $2/"$letter".txt
done
    



