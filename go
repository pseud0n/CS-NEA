#!/bin/sh

# ls go -l
# chmod u+x go

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

motivate () {
	shuf -n 1 text/motivation.txt
}


clear

echo "STARTING"

make

# 0: successful build/nothing updated
# 2: failure

if [ $? -eq 0 ]
then
	echo "${green}BUILD: SUCCESS${reset}"
	./a.out
	if [ $? -eq 0 ]
	then
		echo "${green}EXECUTION: SUCCESS :)"
	else
		echo "${red}EXECUTION: FAILURE; EXIT CODE $? 😞${reset}"
		motivate
	fi
else
	echo "${red}BUILD: FAILURE; EXIT CODE $? 😞${reset}"
	motivate
fi