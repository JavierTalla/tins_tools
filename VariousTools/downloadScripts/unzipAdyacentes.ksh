ls -1 zip249/ | cut -d'.' -f1 |while read a
do  
   getAdyacentes $a | while read b
   do
       descomprimir $b
   done
done

function getAdyacentes(){
 la=$(echo $1 | sed 's/N/N /g'| sed 's/S/S /g' | sed 's/E/ E /g'| sed 's/W/ W /g' | awk '{print $1}')
 lan=$(echo $1 | sed 's/N/N /g'| sed 's/S/S /g' | sed 's/E/ E /g'| sed 's/W/ W /g' | awk '{print $2}')
 lo=$(echo $1 | sed 's/N/N /g'| sed 's/S/S /g' | sed 's/E/ E /g'| sed 's/W/ W /g' | awk '{print $3}')
 lon=$(echo $1 | sed 's/N/N /g'| sed 's/S/S /g' | sed 's/E/ E /g'| sed 's/W/ W /g' | awk '{print $4}')
 lan1=$(printf %02d $(expr $lan + 1))
 lan2=$(printf %02d $(expr $lan - 1))
 lon1=$(printf %03d $(expr $lon + 1))
 lon2=$(printf %03d $(expr $lon - 1))
 echo ${la}${lan1}${lo}${lon}
 echo ${la}${lan2}${lo}${lon}
 echo ${la}${lan}${lo}${lon1}
 echo ${la}${lan}${lo}${lon2}
}

function descomprimir(){
if [ -f $1.gz ]
then
    ## descomprimimos
	# gunzip $b.gz
	echo "$b.new"
else
    if [ -f $b ]
	then
	    echo "$b.ya"
	else
	    echo "$b.no"
	fi
fi
}