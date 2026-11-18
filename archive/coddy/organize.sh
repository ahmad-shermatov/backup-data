#!/bin/bash
for file in downloads/*; do 
	filename=$(basename "$file")
	ext=${file##*.}
	mkdir -p "downloads/$ext"
	mv "$file" "downloads/$ext/"
	echo "Moved $filename to $ext/"
done

