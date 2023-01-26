if [ ! -f main.csv ]; then
    touch main.csv
fi
declare -A flags
i=0
totalArgs=$#
while [ "${1:0:1}" == "-" ]; do
    if [ "${1:1:2}" == "h" ]; then
        flags["$1"]=1
        shift 1
        i=$(($i+1))
        continue

    fi
    if [ -z $2 ]; then
        echo "argument after $1 is missing"
        exit
    fi
    flags["${1}"]=$2
    shift 2
    i=$(($i + 2))
    if [ $i -ge $totalArgs ]; then
        break
    fi
done
totalArgs=$#
if [[ $totalArgs -eq 0 && ${#flags[@]} -eq 0 ]]; then
    flags["-h"]=1
fi
i=0
while [ True ]; do
    if [ $i -ge $totalArgs ]; then
        break
    fi
    if [ -z "$1" -o -z "$2" -o -z "$3" -o -z "$4" ]; then
        printf "One or more column is missing"
        exit
    fi
    printf "$1,$2,$3,$4\n" >>main.csv
    printf "Inserted $1,$2,$3,$4 in main.csv\n"
    shift 4
    sort -r -t- -k2,2 -k1,1 -o main.csv main.csv
    i=$(($i + 4))
done
for option in ${!flags[@]}; do
    case $option in
    "-c") 
            awk -v category=${flags["-c"]}  -F"," 'BEGIN{s=0} {if ($2==category) s+=$3} END { print category":RS " s}' main.csv;;

    "-n") 
        awk -v name=${flags["-n"]} -F"," 'BEGIN{s=0} {if ($4==name) s+=$3} END {print name":RS " s}' main.csv;;

    "-s")
        declare -l categoryName
        categoryName=${flags["$option"]}
        echo $categoryName
        case $categoryName in
        date) ;;
        category)
            sort -t"," -k2,2 -o main.csv main.csv
            ;;
        amount)
            sort -t"," -k3,3 -n -o main.csv main.csv
            ;;
        name)
            sort -t"," -k4,4 -o main.csv main.csv
            ;;
        *)
            echo $option
            printf "Invalid options for sort by column"
            ;;
        esac
        ;;
    "-h") 
    printf "\nNAME\n          Expense Tracker - helps to manage your expenses\n"
    printf "\nSYNOPSIS\n 
            ./Assg_18_ 13.sh [option].....expense\n"
    printf "\nDescription\n 
    Insert the expense in main.csv file, display amount by category and sort the main.csv file according to any column\n
    By default all the rows are in chronological order\n
    -c 'Category': accepts a category and prints the amount of money spend in that category\n
    -n 'name' : accepts a name and print the amount spent by that person\n
    -s 'column': sort the csv by column name\n
    -h : show help prompt
    "
    ;;
    esac
done
