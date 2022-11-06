#!/bin/bash
# Itay Sova 313246209
# Checks if the number of given parameters by the user is less than 2, if so prints as an error "Not enough parameters" and exits the script with 1 as error flag.
if [[ $# -lt 2 ]] 
then
	echo 1>&2 "Not enough parameters"
	exit 1
fi
# init the variable for the script
directory=$1
word=$2
flag=$3
CWD=$(pwd)
cd $directory
# saving the path to the script to use in the recursion
script_path=$(find ~ -name "gccfind.sh")
for file in $(ls)
do
	# going over the files in the dir and removing all .out files
	# if the file is type *.out
	if [[ "${file##*.}" == "out" ]]
		then
		#Then it will delete the file.
		rm $file
	fi
done

# loop for compiling c files
for file in ./*
	do
	# going over the files and if a file is a dir and the flag is -r the script activate
	# itself with the path we saved earlier
	if [[ -d $file ]] && [[ $flag == "-r" ]]
		then
			$script_path $file $word $flag
	else 
		# if the file is type *.c we check the next condition we compile it if the condition
		# is true
		if [[ "${file##*.}" == "c" ]]
			then
			# using grep to find if the file contains given word.
				file_contain_word=$(grep -w -i -l $word $file)
				if [ "$file_contain_word" ]
					then
					filename="$file"
					gcc $file -w -o "${file%.*}".out
				fi
		fi
	fi
done
cd $CWD

