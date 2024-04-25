ATcrt=${ATlibs}libATcrt${LETRA}.a

Name=MDT5Peninsula
Objs=$ATcrt
source GenericoUtilidad.sh

Name=MDT5Canarias
Objs=$ATcrt
source GenericoUtilidad.sh

Name=Lidar-geog___UTM
Objs="$ATcrt ${ATlibs}libATsistemas${LETRA}.a"
source GenericoUtilidad.sh

Name=MDT03
Objs="$ATcrt ${ATlibs}libATsistemas${LETRA}.a"
source GenericoUtilidad.sh

