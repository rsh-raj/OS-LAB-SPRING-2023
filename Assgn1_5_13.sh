#!/bin/bash
function filter_comment() {
    name=$(basename "$1")
    ext="${1#*.}"
    python="py"

    if [[ "$ext" = "$python" ]]; then
        echo "---------------------------------------------------------------------------------------------"
        echo ""
        echo "NAME  :  $name"  
        echo "PATH  :  $1"
        line_num=0
        temp=0
        echo ""
        echo "Comments in the file : "

        while read -r line; do
            line_num=$((line_num+1))
            
            if grep -q \".*'#'.*\" <<< $line; then
            continue

            elif grep -q '#' <<< $line; then
                echo $line_num "#" `cut -d "#" -f2 <<< $line`
                continue

            elif grep -q ['(''=']\"\"\" <<< $line; then   
                continue

            elif grep -q \"\"\"')' <<< $line; then       
                continue

            elif grep -q \"\"\".*\"\"\" <<< $line; then  
                echo $line_num $line
                continue

            elif grep -q \"\"\" <<< $line; then           
                temp=$((1-$temp))
                if [[ $temp == 1 ]]; then                   
                    echo $line_num $line
                else                                   
                    echo $line
                fi
                continue
            fi 

            if [[ $temp -eq 1 ]]; then
                echo $line
            fi
        done < $1

        echo ""
        echo ""

    fi
}

function explore() {
    # $1 is the directory/file name
    # if $1 is a file
    if [ -f $1 ]; then
        filter_comment $1
        exit
    fi

    for file in $1/*
    do
        if [ -d $file ]; then 
            explore $file
        fi
        
        if [ -f $file ]; then
            filter_comment $file
        fi
    done
}
# explore
# if it is a directory then call explore it
    # iterate through all files
    # if directory explore dirctory
    # if file
        # if python file
            # print path
            # print line and content of all comments
        
        # return

rootdir=$1
explore "$rootdir" 


