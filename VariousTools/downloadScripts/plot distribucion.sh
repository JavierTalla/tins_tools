cat /tmp/lista | grep zip | grep -v " 249 May" | awk '{print $9}' | sed 's/\./ /g' | awk '{print $1}' | while read fichero
do
  LA=$(echo $fichero | cut -c 1)
  la=$(echo $fichero | cut -c 2-3)
  LO=$(echo $fichero | cut -c 4)
  lo=$(echo $fichero | cut -c 5-8)
  echo "${fichero},${LA},${la},${LO},${lo}"
done > distribucion.csv
