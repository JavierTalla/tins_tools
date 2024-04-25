#srtm30
https://e4ftl01.cr.usgs.gov/SRTM/SRTMGL30.002/2000.02.11/e020n90.SRTMGL30.dem.zip

function downloadTile(){
nombre="$1"
domain="https://e4ftl01.cr.usgs.gov/SRTM/SRTMGL30.002/2000.02.11/${nombre}"
echo $nombre
curl  -o "${nombre}" -b cookiefile   "$domain"
}

conectar
cat /tmp/list.txt  | grep "zip " | awk '{print $3}' | while read nombre 
do
  downloadTile $nombre
done

