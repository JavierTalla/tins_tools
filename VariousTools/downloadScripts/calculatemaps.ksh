function calculatemap() {
ls -1 | grep gz | grep -v 249 | cut -d'.' -f1 | while read f
do
    g=$(echo $f | sed 's/S/S,/g' |sed 's/N/N,/g' | sed 's/E/,E,/g'| sed 's/W/,W,/g')
	echo "$g,1"
done
ls -1 | grep -v gz | grep -v 249 | cut -d'.' -f1 | while read f
do
    g=$(echo $f | sed 's/S/S,/g' |sed 's/N/N,/g' | sed 's/E/,E,/g'| sed 's/W/,W,/g')
	echo "$g,2"
done
ls -1 zip249/ | cut -d'.' -f1 | while read f
do
    g=$(echo $f | sed 's/S/S,/g' |sed 's/N/N,/g' | sed 's/E/,E,/g'| sed 's/W/,W,/g')
	echo "$g,0"
done
}