#!/bin/bash

number_A=9
while [ $number_A -lt 18 ]; do
	number_B=0
	while [ $number_B -lt 5 ]; do
		echo "Empezando $number_A_$number_B"
	    (time ./simsort -i floats_134217728.raw -o giga.raw -L $number_A) &> "time_"$number_A"_"$number_B".txt"
	    echo "Terminada $number_A_$number_B"
	    number_B=$((number_B + 1))
	done
number_A=$((number_A + 1))
done