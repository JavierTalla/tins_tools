ATcrt=${ATlibs}libATcrt${LETRA}.a

Name=SRTM900m_partir
Objs=$ATcrt
source GenericoUtilidad.sh

Name=SRTM900m_cerrar
Objs=$ATcrt
source GenericoUtilidad.sh

Name=SRTM270m
Objs=$ATcrt
source GenericoUtilidad.sh

Name=SRTM3.6k
Objs=$ATcrt
source GenericoUtilidad.sh

Name=SRTM18k
Objs=$ATcrt
source GenericoUtilidad.sh

