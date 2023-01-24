declare -a sieve
sieve[0]=2
echo "Poplulating sieves array, please wait......"
for ((i = 2; i <= 1000000; i++)); do
    for ((j = i; j * i <= 1000000; j++)); do

        if [[ -z ${sieve[i]} ]]; then
            sieve[j * i]=$i
        fi
    done
done
for ((i = 2; i <= 1000000; i++)); do
    if [[ -z ${sieve[i]} ]]; then
        sieve[i]=$i
    fi
done
printf "Sieve array successfully poppulated.\n Calculating result.."
printf "" > output.txt #clearing output 
while IFS=$'\n\r' read  a; do
    if [ $a -gt 10000 ]; then
        continue
    fi
    declare -A primeFactors
    # echo -n "$a = "
    while [ "$a" -gt 1 ]; do
        primeFactors[${sieve[a]}]=1
        a=$((a / sieve[a]))
    done
    for i in "${!primeFactors[@]}"; do
        echo -n "$i " >> output.txt
    done
    unset primeFactors
    echo >> output.txt
    
done<input.txt
