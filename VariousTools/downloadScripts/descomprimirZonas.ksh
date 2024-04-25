
decomprimir costeros
mapas

function unzipTiles(){
Lat=$1
Long=$4
for i in `seq ${2} ${3}`
do	
 for j in `seq ${5} ${6}`
 do   
  fichero="${Lat}$(printf %02d ${i})${Long}$(printf %03d ${j}).gz"
  if [ -f $fichero ]
  then
    gunzip $fichero
  fi
 done
done
}

europa
#unzipTiles N 35 90 W 0 10
#unzipTiles N 35 90 E 0 50
canarias
#unzipTiles N 26 30 W 10 20
		