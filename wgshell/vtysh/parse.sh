#!/bin/dash

CMD_INIT_C=cmd_init.c
echo "/* !!!DONT EDIT THIS FILE!!!*/\nvoid cmd_parse_init()\n{" > $CMD_INIT_C

file=`ls cmd/*.c`
for f in $file
do
	func=`basename $f | awk -F "." '{print $1'}`
	echo "\t Adding ${func}_init"
	echo "\t${func}_init();" >> $CMD_INIT_C
done

#echo -e "\tcmd_sort_node();" >> $CMD_INIT_C
echo "}" >> $CMD_INIT_C
