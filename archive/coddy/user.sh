#!/bin/bash
secret=5
guess=0

while [ $secret != $guess ]; do 
	guess=$((guess+1))
	if [ $guess -eq $secret ]; then
		echo "Guess $guess: Correct!"
		break
	fi
	echo "Guess $guess: Too low!"
done

