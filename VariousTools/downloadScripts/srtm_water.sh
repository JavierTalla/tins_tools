function downloadWaterTile(){
   nombre="$1"
   domain="ftp://topex.ucsd.edu/pub/srtm30_plus/srtm30/data/${nombre}"
    echo $nombre
	echo $domain
	curl  -o "${nombre}"    "$domain" 
	if [ -f "${nombre}" ]
	then 
	    echo si
		gzip ${nombre}
	else    
        echo no
	    curl  -o "${nombre}"   "$domain" 
	fi
}

cat list.txt  | grep -v ".ers" | awk '{print $1}' | while read nombre
do
    downloadWaterTile $nombre
done

