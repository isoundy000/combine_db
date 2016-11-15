#! /bin/bash

mysqlcon="mysql -uroot -p123456"
seq=0
guildid=10001000000057
guild_name=('guild_name_1' 'guild_name_2' 'guild_name_3' 'guild_name_4' 'guild_name_5')
chelf_id=(1000100000001 1000100000002 1000100000003 100010000004 1000010000005)
create_time=(1475145680 1455147080 1422247080 1472237080 1472337080)
member_list=('ABCDEFGHIJEJHGUWGUWG' 'KLMNKPOEITRYCNJBXIUEDNBRUBUFIIUREWE' 'ELKHDIUEIUEGDWHWGDREHFHDE' 'WHSIWUSGWUFGDEGDEUDEDGEU' 'WLHSHYUDG;OSJODLWHUGE')
app_list=('JUKWNSBBSUIWSDEEDEWDED' 'DIHEUIDHWBYXBIWHYUEGDNXHBD' 'L;SDSOIHFRTYBNXIWJDEUDG' 'LWNMSIURUNXIOSO;RC=SKJDHE' 'WNDWBFYUEDNDUE4DBDYS')

while test $seq -le 100000
do
	$mysqlcon -e "insert into game.guild values ($guildid, '${guild_name[$RANDOM % 5]}', ${chelf_id[$RANDOM % 5]}, ${create_time[$RANDOM % 5]}, '${member_list[$RANDOM % 5]}', '${app_list[$RANDOM % 5]}');"
	echo "process record $seq"
	let seq=$seq+1
	let guildid=$guildid+1;
done
echo "insert complete"

