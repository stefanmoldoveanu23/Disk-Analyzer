#!/bin/bash

# I will have to modify the file path to the working directory
FILE=/home/characterme/Disk-Analyzer/onclose/file

case "$1" in
	stop)
	echo "Stop" >> $FILE
	;;
	*)
	echo "Start" >> $FILE
	;;
esac

x=1
while [ $x -le 5 ]
do
	x=1
done
