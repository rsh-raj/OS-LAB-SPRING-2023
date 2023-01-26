#!/bin/bash
gawk '{arr[$2]++}END{for(i in arr)print i,arr[i]}' $1 | sort -k2nr,2 -k1,1
gawk '{stu[$1]++}END{for(i in stu)if(stu[i]>1){{print i} cnt++};print length(stu)-cnt}' $1