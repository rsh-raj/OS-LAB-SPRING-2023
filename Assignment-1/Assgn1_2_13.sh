for line in $(cat $1)
do  
    z="NO"
    if [[ $line =~ ^[a-zA-Z]{1}[a-zA-Z0-9]{4,19}$ ]] && [[ $line =~ [0-9] ]]
    then
        z="YES"
        for word in $(cat fruits.txt)
        do
            if grep -qi "$word" <<< "$line"
            then
                z="NO"
                break  
            fi
        done
    fi
        echo $z >> validation_results.txt
done