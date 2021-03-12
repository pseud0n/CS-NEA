#!/usr/bin/env bash

if [ -z "$(ls ./bytecode-files/)"  ]; then
	echo "No bytecode folders to read"
else
	DIR=$(ls ./bytecode-files/*/ -td | head -1)
	echo "dir:" ${DIR: -3: -1}
	FILES=$(ls $DIR)
	echo ${FILES}
	for binary in ${FILES: 5} "a.bin"; do
		echo ${binary} ":"
		hexyl $DIR/$binary
	done
fi
