#!/bin/bash
sed -e 's:.*\x20'$2'\x20.*:\L\0: ' -e 's:[[:lower:]][^[:alpha:]]*\([[:alpha:]]\|$\):\u&:g' $1
