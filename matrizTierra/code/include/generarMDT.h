/* matrizmarcada___userspecs

Esta función crea una matriz con valores de alturas a partir de los datasets, de la siguiente manera:

1º: Obtiene la superficie.
2º: Determina qué es lago y qué no, guardando además cierta información para cada uno de los detectados.
2º: Crea el fondo de mar y lagos allí donde haya datos.
3º: Crea matriz->cielo teniendo en cuenta los edificios.
4º: Asigna matriz->zmax_agua_borde

Los punteros matriz->suelo, matriz->cielo y matriz->i.flags si no son NULL tras la llamada a la función habrán de liberarse
en algún momento.

Params:
	matriz: En matriz->suelo se almacenará el puntero a la memoria reservada para la matriz que la función crea.
		En matriz->zbounds los valores mín. y máx. de z de la zona y los parámetros que permiten extraer valores
		de z en metros a partir de lo almacenado en la matriz (v. infra).

	transf: Guarda la información para pasar de coordenadas geográficas a posición dentro de la matriz
		Ha de cumplirse exactamente transf.λmin + transf->npλ*transf->pix_λ = transf.λmax
											 transf.φmin + transf->npφ*transf->pix_φ = transf.φmax

	besquina: Si es false se calculan coordenadas para los centros de los píxeles, si es true, para las esquinas.
		Si es false se reserva memoria para npφ*npλ elementos. Si es true, para (npφ+1)*(npλ+1).

	datasets: Los datasets a emplear. Se escogerán un dataset u otro según el tamaño de píxel.

	clip: Puede ser NULL. Array de polígonos para aplicar un clip. Las coordendas son (λ,φ). No es const porque el
		programa modificará estos valores, transformándolos a un sistema de coordenadas para la matriz.
	npos: Número de polígonos de array clip.

	abst: Opciones para la generación de la matriz. Véase la descripción de esta estructura en su definición.

	npix: El dataset a emplear se determina en función del parámetro pixeljump de la estructura 'abst'. Se explica en el archivo
		de configuración. Para esta determinación hay que conocer el tamaño de píxel en metros de matriz. Se trata de un
		tamaño nominal, ya que no puede mantenerse constante a lo ancho de toda la zona a generar. Eso se obtiene a partir
		de la transformación transf. Pero como es posible que se ordene a la función obtener una matriz "sobrepixelada" para el
		fin a que se destine, la función recibe también el número de píxeles de la matriz que hacen un píxel de resolución real
		deseada. Es este parámetro. Normalmente será >=1. Por ejemplo, si el píxel de la matriz es 4 metros y se pasa 3.0 en npix,
		se entiende que se quiere una matriz con resolución en sus datos de 12 m. La matriz generada tendrá un paso de 4m,
		pero estará "borrosa".
		Si no se desea una matriz sobrepixelada, pásese 1.
		Un valor <1 obliga a buscar datasets más precisos de lo que sería necesario pero no se traduce en una mejora de la precisión
		de la matriz, y por tanto no sirve para nada, salvo que queramos obligar a emplear unos datasets en particular.

Devuelve una instancia de la estructura MatrizMarcadaReturn.
	nret: 0
		AT_NOMEM
		MTIERRA_TooManyLakes

		Puede ser simplemente que en la zona no hay edificios o no nay mar. Si el bit de suelo está a 1 el resultado no es correcto

	nret_suelo, etc: los códigos devueltos por las funciones que crean la superficie principal (el suelo), el fondo del mar
		y los edificios respectivamente. Pueden ser:
		0
		AT_NOMEM
		MTIERRA_...	Códigos definidos en MatrizMarcadaReturn.h
		TILE_...			Códigos definidos en MatrizMarcadaReturn.h

		Si la función no se llega a llamar se establece el nret correspondiente a MTIERRA_DSnotsearched. Por ejemplo,
		    si no se pide fondo del mar será nret_fondo = MTIERRA_DSnotsearched.
		Si alguno de ellos es AT_NOMEM entonces .nret también será AT_NOMEM

	ds_suelo, etc: El dataset seleccionado en cada una de las funciones. NULL si no se seleccionó ninguno.

	bflags: Indican si hay algún punto dentro de la matriz es del tipo en cuestión (uno por flag). V. las definiciones siguientes.
*/

//npix: nº de píxeles que abarca el píxel de resolución real que se desea. Si
MatrizMarcadaReturn matrizmarcada___userspecs(MatrizTierra *matriz, const Matriz___Tierra *transf, bint besquina, const DataSets *datasets, Polígono_xy *clip, u8int npols, const OpcionesMDTierra *abst, float npix);


/*****         Funciones parciales         *****/

//matrizmarcada___userspecs llama a estas tres funciones, que se pueden emplear individualmente

/* Crea una matriz de dimensiones npx * npy, o bien (npx+1) * (npy+1) en donde cada dato es un uEarthHeight (una z con
offset) a partir de las especificaciones del usuario. La matriz es un modelo digital de la región de la Tierra especificada en
el rectángulo "geográficas". En realidad el usuario no especificará npx, pny, pix_λ y pix_φ. Estos valores se deducen a partir
de un Tablero y un rectángulo de coordenadas geográficas llamando a calcula_matriz___tierra y llevando a cabo los ajustes
necesarios.
	La función obtiene los valores de alturas buscando por los conjuntos de datos adecuados según los tamaños de
pix_λ y pix_φ y la zona de la Tierra. Si para alguna zona no hay datos se rellena con ceros. Nunca almacenará el valor 0xFFFF
en ningún punto.
	Reserva memoria para la matriz y el puntero que apunta al bloque de memoria reservado se almacena en matriz->suelo.
En matriz->cielo siempre se asigna NULL. La matriz devuelta es necesario liberarla con free_null_if_MatrizTierra.

Params:
	debug: Estructura en la que la función escribe información.

	matriz: Ha de haberse inicializado con MatrizTierra_setNULL.
		Si matriz->suelo es NULL, en él se almacenará el puntero a la memoria reservada para la matriz que la función crea.
			En caso contrario se supone que ya apunta a un bloque de memoria adecuado. En este caso además matriz->nox,
			->noy y ->npuntos tienen que ser correctos.
		Si matriz->i.flags es NULL, ahí se almacenará el puntero a la memoria reservada y se inicializa a cero. Si no, se supone
			que apunta a las flags. Se pondrá .fuera=1 para los puntos fuera de la Tierra o del área abarcada por los datasets.
			Si no era NULL al llamar a la función, los puntos que tuviesen .fuera=1 se ignoran para calcular zbounds. no
			obstante esos puntos se procesan para detectar correctamente los lagos.
		En matriz->zbounds los valores mín. y máx. de z de la zona (ignorando los puntos con .fuera=1) y los parámetros
		que permiten extraer valores de z en metros a partir de lo almacenado en la matriz (v. infra).

	transf: Guarda la información para pasar de coordenadas geográficas a posición dentro de la matriz
		Ha de cumplirse exactamente transf.λmin + transf->npλ*transf->pix_λ = transf.λmax
											 transf.φmin + transf->npφ*transf->pix_φ = transf.φmax

	besquina: V. infra.
	datasets: Los datasets a emplear. Se escogerán un dataset u otro según el tamaño de píxel.
	abst: Véase la descripción de la estructura Opciones_Abstractas.

	Si besquina es false se calculan coordenadas para los centros de los píxeles, si es true, para las esquinas.
	Si besquina es false se reservan npφ*npλ*sizeof(uEarthHeight) bytes de memoria. El puntero a esta memoria se guarda
en *matriz. Si besquina es true se reservan (npφ+1)*(npλ+1) píxeles.
	En cualquier caso la matriz abarca una región de npλ*pix_λ  x  npφ*pix_φ grados. Las coordenadas del primer
punto almacenado en matriz así como npx y npy son:

	Si besquina es false:
		(transf.λmin + 0.5*pix_λ, transf.φmax - 0.5*pix_φ),	npx=npλ, npy=npφ		si transf->b90 es 0
		(transf.λmin + 0.5*pix_λ, transf.φmin + 0.5*pix_φ),	npx=npφ, npy=npλ		si transf->b90 es 1

	Si besquina es true:
		(transf.λmin, transf.φmax),		npx=npλ+1, npy=npφ+1		si transf->b90 es 0
		(transf.λmin, transf.φmin),		npx=npφ+1, npy=npλ+1		si transf->b90 es 1

	Si transf->b90 es false, tras el primer píxel almacenado vendrán valores para la misma φ e incrementos de λ de transf->pix_λ,
hasta completar npx valores. A continuación otros npx valores correspondientes a un valor de φ pix_φ menor que el primero,
y así sucesivamente hasta (φmin + 0.5*pix_φ, λmax - 0.5*pix_λ) o bien (φmin,λmax), según besquina.
	Si transf->b90 es true, tras el primer píxel almacenado vendrán valores para la misma λ e incrementos de φ de transf->pix_φ,
hasta completar npx valores. A continuación otros npx valores correspondientes a un valor de λ pix_λ mayor que el primero,
y así sucesivamente hasta (φmax - 0.5*pix_φ, λmax - 0.5*pix_λ) o bien (φmax,λmax), según besquina.

	El paso de un valor de z almacenado en matriz a un valor de Z en metros respecto al nivel cero de los datos es:

				Z = z/zbounds.escala + zbounds.offset
	o bien		Z = (z + zbounds.escala*zbounds.offset)/zbounds.escala

	Por tanto los puntos cuya z almacenada sea zbounds.escala*zbounds.offset son los que están a cota cero en la Tierra.
Esto solo es posible si zbounds.offset<=0. En caso contrario toda la matriz está por encima del nivel del mar.


Return values:
	0: todo bien
	AT_NOMEM				//No hay memoria suficiente para crear la matriz
	Otro código de error de los definidos en MatrizMarcadaReturn.h
*/
int matrizsuelo___userspecs(Debug_matrizsuelo *debug, MatrizTierra *matriz, const Matriz___Tierra *transf, bint besquina, const DataSets *datasets, const OpcionesMDTierra *abst);

/*Modifica o crea matriz->suelo para añadir el fondo del mar y lagos allí donde lo haya.

	matriz: Ha de haberse inicializado con MatrizTierra_setNULL o bien contener datos válidos, por ejemplo de una llamada
		a matrizsuelo___userspecs.

Return
	0: Si todo está bien y hay algún valor
	MTIERRA_DSnoextradata: Si no hay ningún punto por debajo del terreno que ya existe
	Otro código de error de los definidos en MatrizMarcadaReturn.h

    Si al llamar a la función matriz->suelo es NULL, la función se comporta como matrizsuelo___userspecs, salvo porque eligirá
otros datasets. En particular, reservará memoria para matriz->suelo y, si matriz->flags es NULL, reservará para este y lo pondrá
a cero, salvo por .fuera.
    Véase la descripción de matrizsuelo___userspecs para más detalles.
*/
int matriz_fondomar(Debug_matrizsuelo *debug, MatrizTierra *matriz, const Matriz___Tierra *transf, bint besquina, const DataSets *datasets, const OpcionesMDTierra * abst);

/*Pone edificios por encima de matriz->suelo, guardando el resultado en matriz->cielo. Si matriz->cielo es NULL
reserva memoria para ello. Si no, entiende que ya apunta a un espacio de memoria y que los valores son buenos,
de manera que sólo se actualizarán si el nuevo valor es > que el anterior.

Return:
	0: Se ha añadido algún punto de edificio
	AT_NOMEM
	MTIERRA_DSfolder_notfound
	TILE_BadSize
	MTIERRA_DSnooverlap: Ningún tile con información de edificios se solapa con la zona de la tierra a generar
	MTIERRA_DSnoextradata: Algún tile entra en la zona pero ningún edificio entra en la zona.

Los edificios que están soblre láminas de agua son pantalanes, puentes, embarcaciones, etc. En esos puntos se elimina
la marca de lago; e. .d., se hace flags[i]->lago=0.
    flags[]->cielo ni se lee ni se modifica.
*/
int matriz_edificios(Debug_matrizedificios *debug, MatrizTierra *matriz, const Matriz___Tierra *transf, bint besquina, const OpcionesMDTierra *abst, const PointsDataSet *dset);
