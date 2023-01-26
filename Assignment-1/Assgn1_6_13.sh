declare -a sieve
k=0
echo "Poplulating sieves array, please wait......"
for ((i = 2; i <= 1000000; ++i)); do
   if [[ -z ${sieve[i]} ]]; then
    for ((j = i*i; j <= 1000000; j+=i)); do
        k=$((k+1))
        if [[ -z ${sieve[j]} ]]; then
            sieve[j]=$i
        fi
    done
    fi
done
for ((i = 2; i <= 1000000; i++)); do
    if [[ -z ${sieve[i]} ]]; then
        sieve[i]=$i
    fi
done
printf "Sieve array successfully poppulated.\n Calculating result.."
printf "" > output.txt #clearing output 
while IFS=$'\n\r' read  a; do
    declare -A primeFactors
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
