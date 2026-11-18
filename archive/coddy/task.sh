#!/bin/bash 
balance=100
week=1
while [ $balance -lt 200 ]; do 
	echo "Week $week: $balance" 
	balance=$((balance+30))
	week=$((week+1))
done
echo "Goal reached: $balance"
