#!/bin/sh

# ls go.sh -l
# chmod u+x go.sh

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

motivate () {
	shuf -n 1 text/motivation.txt
}


clear

echo "Started at $(date +"%T")"

build_start=`date +%s` # start time, seconds

make

# 0: successful build/nothing updated
# 2: failure

if [ $? -eq 0 ]
then
	echo "${green}BUILD: SUCCESS${reset}; $((`date +%s`-build_start))s"
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