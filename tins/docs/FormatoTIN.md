# Formato de ficheros TIN

**Mayo de 2021**

  - [Codificación](#Codificacion)
  - [Cabecera](#Cabecera)
  - [Puntos y triángulos](#Puntos-y-triangulos)
    - [Puntos en retícula](#Puntos-en-reticula)
    - [Puntos en icrementos](#Puntos-en-icrementos)
    - [Puntos individuales](#Puntos-individuales)
    - [Triángulos con vértices en progresión aritmética](#Triangulos-con-vertices-en-progresion-aritmetica)
    - [Triángulos en icrementos](#Triangulos-en-icrementos)
    - [Triángulos individuales](#Triangulos-individuales)
  - [Estilos](#Estilos)
    - [Estilos de los puntos](#Estilos-de-los-puntos)
    - [Estilos de los triángulos](#Estilos-de-los-triangulos)
    - [Clase a la que pertenece cada punto](#Clase-a-la-que-pertenece-cada-punto)
    - [Clase a la que pertenece cada triángulo](#Clase-a-la-que-pertenece-cada-triangulo)
    - [Estilos individuales de los triángulos](#Estilos-individuales-de-los-triangulos)
- [Formatos concretos de ejemplo](#Formatos-concretos-de-ejemplo)
  - [Tin plano con datos float](#Tin-plano-con-datos-float)
  - [Ficheros generados por 3dCarving (I)](#Ficheros-generados-por-3dCarving-i)
  - [Ficheros generados por 3dCarving (II)](#Ficheros-generados-por-3dCarving-ii)

## Codificación

Todos los campos se encuentran alineados a 4 bytes. Una unidad de 4 bytes se llama **palabra**. El orden de los bytes en los datos que ocupen más de un byte es little-endian. Tipos de datos:

|   |   |
|---|
| uint16 / sint16 | half (2 bytes)
| uint32 / sint32 | float (4 bytes)
| | double (8 bytes)

En los tipos enteros con signo los números negativos se representan en comlemento a 2. Los tipos _half_, _float_ y _double_ son los tipos de coma flotante de las especificaciones IEEE-754, salvo porque no se permiten infinitos ni NAN's, y los
números denormalizados pueden tratarse como cero. Cuando un **uint32** ha de almacenar un color, será en el formato `0x00RRGGBB`. El byte más alto _puede_ emplearse para
la transparencia, pero es algo que este formato no exige.

Las posiciones de los campos a partir del comienzo del elemento que los contiene se indican en palabras (4-bytes).
Dentro de una palabra, si hay que especificar los bytes que la componen, se hará como b0, b1, b2, b3, siendo b0 el más bajo (y por tanto
el primero almacenado en memoria) y b3 el más alto. Cuando se indica un carácter como el valor de un byte, por ejemplo 'T', el valor es el indicado en la codificación ASCII.

## Cabecera

**W0:**  b0,b1,b2: 'T', 'I', 'N'.  b3: _número de versión del formato_. De momento, 0.

**W1:**
  * b0: Tipo de dato empleado para las coordenadas de los puntos:
	* 's': sint16
	* 'S': uint16
	* 'i': sint32
	* 'h': half
	* 'f': float
	* 'd': double
  * b1: Modo de indicar las unidades de las coordenadas (que están en&nbsp;[W2]).
	* 0: metros (float) (e.d., [W2] es un float)
	* 1: micras (uint32)
	* 2: mm (uint32)
	* 3: metros (uint32)
	* 4: Fracción de mm (uint32)
	* 5: Fracción de metro (uint32)
	* 6: Fracción de km (uint32)

**W2:** Las unidades de las coordenadas almacenadas en el fichero, en el tipo de dato e interpretación según se indica en [W1-b1].

**W3-W7:** Libres

Los siguientes elementos indican la posición desde el comienzo del fichero de ciertos datos. En todos ellos, si la posición indicada es `0xFFFFffff`
quiere decir que el elemento no existe. Este valor especial se representará mediante `Я`.

**W8:** Posición de las definiciones de retículas de puntos

**W9:** Número total de puntos definidos en las retículas

**W10:** Posición de los puntos con coordenadas definidas mediante incrementos

**W11:** Número total de puntos definidos en las definiciones “de incrementos”

**W12:** Posición de las definiciones individuales de puntos

**W13:** Número de puntos definidos en las definiciones individuales

Los números de puntos se empiezan a contar en&nbsp;`0`. Así, si el fichero define en total 80 puntos sus números irán de `0` a&nbsp;`79`.
El prmer punto de los que se encuentran en _puntos con sus coordenadas definidas mediante incrementos_ tiene por número
el indicado en&nbsp;[W9]. Por tanto, si este número es por ejempo 100, el primer punto de estas definiciones (si es que hay alguno)
será el número&nbsp;`100`. En un fichero correcto el valor de&nbsp;[W9] tiene que coincidir con el número de puntos realmente definidos
en las retículas. Si es menor, los puntos definidos en las retículas a mayores del valor de&nbsp;[W9] serán inaccesibles, ya que los
números de punto desde&nbsp;[W9] en adelante harán referencia a los puntos definidos “por incrementos” y a las definiciones
individuales. Si es mayor, por ejemplo [W9]=100 pero solamente se definen 80 puntos, los números de punto entre `80` y&nbsp;`99`
no se podrán usar y no deben aparecer en los triángulos.

El valor de&nbsp;[W9] es siempre el número de punto del primer punto de las definiciones por incrementos, incluso aunque no haya
definiciones de puntos en retícula ([W8]=`Я`).

El primer punto de las _definiciones individuales de puntos_, si es que hay alguno, tiene por número de punto [W9]+[W11],
con independencia de si [W8] o&nbsp;[W10] valen&nbsp;`Я`.
 
**W14:** Posición de las definiciones de triángulos con vértices en progresiones aritmética

**W15:** Número total de triángulos definidos en esas definiciones

A estos triángulos se les llamará en lo sucesivo triángulos en malla, porque suele ser el caso que forman una malla
regular.

**W16:** Posición de los triángulos con números de vértices definidas mediante incrementos

**W17:** Número total de triángulos definidos en las definiciones “de incrementos”

**W18:** Posición de las definiciones individuales de triángulos

**W19:** Número de triángulos en las definiciones individuales

Se aplica a los números de triángulo lo mismo que a los puntos: El primer triángulo definido en “los incrementos” tendrá
por número&nbsp;[W15]. El primero de las definiciones individuales tendrá por número [W15]+[W17]. Alguno de estos números
o los dos pueden ser cero.

**W20:** Posición de definiciones de estilos de puntos

**W21:** Posición de definiciones de estilos de triángulos

Estas definiciones de estilos definen el estilo de cada _clase_. Así, si un punto pertenece a la clase 3, se le aplicará
la definición 3ª (comenzando a contar en la 0ª) de las presentes en las definiciones de estilos de puntos.

**W22:** Posición de la clase a la que pertenece cada punto

**W23:** Posición de la información individual de estilo de puntos

**W24:** Posición de la clase a la que pertenece cada triángulo

**W25:** Posición de la información individual de estilo de triángulos

Las definiciones globales de estilos definen el estilo para todos los puntos o triángulos en función de la
clase a la que pertenecen, que se indica en la propia definición global. La información individual permite
asignar estilos particulares a puntos o triángulos concretos.

**W26-W39:** Libres

Por tanto el valor mínimo válido de _Posición_ en alguna de las entradas anteriores es 40.

Resumen de W8-W23:

| | |
|---|---|
| **W8:** Puntos, retícula (_pos_) | **W9:** puntos, retícula (_n_)
| **W10:** Puntos, incrementos (_pos_) | **W11:** puntos, incrementos (_n_)
| **W12:** Puntos, individuales (_pos_) | **W13:** puntos, individuales (_n_)
| **W14:** Triángulos, malla (_pos_) | **W15:** triángulos, malla (_n_)
| **W16:** Triángulos, incrementos (_pos_) | **W17:** triángulos, incrementos (_n_)
| **W18:** Triángulos, individuales (_pos_) | **W19:** triángulos, individuales (_n_)
| **W20:** Estilos de puntos, definiciones (_pos_) | **W21:** Estilos de triángulos, definiciones (_pos_)
| **W22:** Clase a la que pertenece cada punto (_pos_) | **W23:** Estilos de puntos, indivi. (_pos_)
| **W24:** Clase a la que pertenece cada triángulo (_pos_) | **W25:** Estilos de triángulos, indivi. (_pos_)

Si en cualquiera de los bloques anteriores el último dato ocupa parte de una palabra, el resto de la palabra ha de
estar presente en el fichero. Por tanto, no puede estar el final del fichero ahí mismo sin los bytes de alineación
a una palabra completa.


## Puntos y triángulos

Cada punto son tres coordenadas. Cada triángulo queda definido por los números de punto de sus vértices; es decir,
por tres números &ge; 0. Los vértices se indican en sentido antihorario, vistos desde la cara visible del triángulo.

### Puntos en retícula

Consta este bloque de varias definiciones de retículas de puntos. Cada una de estas es como sigue:

`m n sig dato X0 Y0 Z0 ΔX1 ΔY1 ΔZ1 ΔXn ΔYn ΔZn ΔX ΔY ΔZ `<_desplazamientos_>

Los cuatro campos `m`, `n`, `sig` y&nbsp;`dato` ocupan una palabra cada uno. `sig` indica la posición de la
siguiente retícula, contando desde el comienzo de esta. Por ejempo, si esta retícula comienza en la palabra 120
del fichero y `sig` es&nbsp;40, la siguiente retícula comenzará en la palabra 160. Un valor&nbsp;`Я` en `sig`
indica que no hay siguiente. Un valor de&nbsp;`Я` en la posición de&nbsp;`m` es una marca de final, y el
último bloque fue el anterior.

Se porporcionan pues dos maneras alternativas de indicar el final: a través de `sig` o a través de&nbsp;`m`. En
el primer caso el último de los bloques indica que él mismo es el último, mediante `sig`=`Я`; en el segundo, el
último bloque indicará un valor válido en&nbsp;`sig`, y en la posición ahí indicada se encontrará el valor&nbsp;`Я`.

La retícula es un rectángulo de `m`&nbsp;x&nbsp;`n` puntos una de cuyas esquinas tiene coordenadas P0=(`X0,Y0,Z0`).
Estos tres valores son del tipo indicado en [W1-b0]. Este es el primer punto de la retícula. Los desplazamientos entre
columnas y filas son Δ1=(`ΔX1,ΔY1,ΔZ1`) y Δn=(`ΔXn,ΔYn,ΔZn`); el tipo de dato de estos seis valores es el tipo
con signo correspondiente a [W1-b0]; es decir, sint16 si ese es uint16, e igual a [W1-b0] en cualquier otro caso.
Es irrelevante a qué se llame «filas» y a qué «columnas». Del primer punto al siguiente el desplazamiento es&nbsp;Δ1,
mientras que del primer punto al punto _n_-ésimo el desplazamiento es&nbsp;Δn. Las coordenadas de los puntos
son por tanto
```
P0,	P0+Δ1, P0+2Δ1, ... , P0+(n-1)Δ1,
P0+Δn,	P0+Δn+Δ1,   ... , P0+Δn+(n-1)Δ1,
P0+2Δn,		   ... , P0+2Δn+(n-1)Δ1,
...
P0+(m-1)Δn,    ... , P0+(m-1)Δn+(n-1)Δ1.
```

Si los tres valores `ΔX ΔY ΔZ` son&nbsp;`0` no hay nada más tras ΔZ, la definición termina ahí y no existe el bloque
<_desplazamientos_>. En caso contrario este bloque contiene m x n valores. El tipo de dato se determina a partir del tipo
de datos del fichero, [W1-b0], y lo indicado en `dato`. Si el bit bajo de esta palabra es&nbsp;`0` los valores son lo que
se indica en [W1-b0]. Si es&nbsp;`1`, son _la mitad_, si existe,  de acuerdo al siguiente cuadro:

|  W1-b0  | `ΔX,ΔY,ΔZ` | |  W1-b0  | `ΔX,ΔY,ΔZ` |
|:---------:|:-------------:|---| :---------:|:-------------:| 
| double			| float		| | uint16 | uint16
| float, half	| half		| | sint32, sint16 | sint16

Estos valores indican múltiplos del vector Δ=(`ΔX ΔY ΔZ`) desde el punto de la cuadrícula. Por ejemplo, si un
punto de la cuadrícula tiene coordenadas (100, 65, -12), el vector Δ es (1, -2, 0) y el número de desplazmiento para
ese punto es&nbsp;`7.5`, el punto tendrá por coordenadas

(100, 65, -12) + 7'5*(1,-2,0) = (107'5, 50, -12)

Como siempre, si el último dato ocupa la primera mitad de una palabra, la mitad alta estará vacía.

El primero de los puntos de estas definiciones tiene por número de punto&nbsp;`0`.

### Puntos en icrementos

Consta este bloque de varias definiciones de puntos “en incrementos”. Cada una de estas es como sigue:

``
n sig X0 Y0 Z0  ΔX ΔY ΔΖ  ΔX ΔY ΔΖ  ...
``

(`X0,Y0,Z0`) son las coordenadas del primer punto. Los puntos sucesivos se definen dando su diferencia
de coordenadas respecto al inmediato anterior. Estos incrementos, `ΔX,ΔY,ΔZ`, están codificados en
_la mitad_ del tipo de dato en el que se indican las coordenadas de los puntos, siempre que sea posible,
y son siempre con signo, de acuerdo al siguiente cuadro:

|  W1-b0  | `ΔX,ΔY,ΔZ` | |  W1-b0  | `ΔX,ΔY,ΔZ` |
|:---------:|:-------------:|---| :---------:|:-------------:| 
| double			| float		| | sint32 | sint16
| float, half	| half		| | uint16, sint16 | sint16

`n` y&nbsp;`sig` son **uint32**. `n`es el número de puntos definidos (incluyendo el inicial (`X0,Y0,Z0`)).
`sig` es la posición del siguiente bloque, en words contados desde el comienzo del presente bloque. Si
`sig`es&nbsp;`Я` significa que no hay siguiente bloque. Si&nbsp;`n` es&nbsp;`Я` significa que ya se acabó.
Por tanto en ese caso el valor&nbsp;`Я`, en la posición en la que iría&nbsp;`n`, no es ningún número de puntos
sino simplemente una marca de final.

Se porporcionan pues dos maneras alternativas de indicar el final: a través de `sig` o a través de&nbsp;`n`.
En el primer caso el último de los bloques indica que él mismo es el último; en el segundo, el último bloque
indicará un valor válido en `sig`, y en la posición ahí indicada se encontrará el valor&nbsp;`Я`.

El primero de los puntos definido en estos bloques tiene por número de punto el número indicado en&nbsp;[W9].

### Puntos individuales

``
X Y Z  X Y Z  X Y Z  ...
``

El primer punto definido en este bloque tiene por número de punto [W9]+[W11]. El número de ternas `X,Y,Z` presentes
es el indicado en&nbsp;[W13]. El último punto presente tiene por tanto por número de punto  [W9]+[W11]+[W13]-1.

Cada coordenada está en el tipo indicado en [W1-b0]. Si son **double**, ocupará 8 bytes. Si son **float**
o **sint32** ocupará 4 bytes. Si son **uint16** o **sint16** ocupará 2 bytes.

### Triángulos con vértices en progresión aritmética

Consta este bloque de varias definiciones de series de triángulos. Cada una de estas es como sigue:

`m n a0 b0 c0 Δa1 Δb1 Δc1 Δan Δbn Δcn`

`m`, `n`, `a0`, `b0` y&nbsp;`c0` son **uint32**. `Δa1`, ...  `Δcn` son **sint32**. Se definen m x n triángulos. Sea T0=(`a0,b0,c0`),
Δ1=(`Δa1,Δb1,Δc1`) y Δn=(`Δan,Δbn,Δcn`). Los números de vértice de los m x n triángulos son:
```
T0,	T0+Δ1, T0+2Δ1, ... , T0+(n-1)Δ1,
T0+Δn,	P0+Δn+Δ1,   ... , T0+Δn+(n-1)Δ1,
T0+2Δn,		   ... , T0+2Δn+(n-1)Δ1,
...
T0+(m-1)Δn,    ... , T0+(m-1)Δn+(n-1)Δ1.
```

Habitualmente los vértices de estos triángulos son los definidos en una retícula de puntos. En esos casos cada cuadradito
de la retícula se divide en dos triángulos según una diagonal del cuadrado. Los triángulos así determinados formarán
dos series de este tipo, conteniendo una de las series los que están de un lado de la diagonal y la otra serie los que están
del otro lado.

Las definiciones se suceden una a continuación de otra. Un valor de&nbsp;`m` de&nbsp;`Я`indica el final. No es por tanto
un valor de&nbsp;`m` sino una marca de final, tras la cual no hay nada más.

El primer triángulo definido en estos bloques tiene por número de triángulo&nbsp;`0`.

### Triángulos en icrementos

Consta este bloque de varias definiciones de triángulos &ldquo;en incrementos&rdquo;. Cada una de estas es como sigue:

``
n sig a0 Δb Δc  Δa Δb Δc  Δa Δb Δc  ...
``

`a0` es el primer vértice del primer triángulo; es un **uint32**. Los vértices sucesivos se definen indicando el incremento
respecto al anterior. Estos incrementos son de tipo **sint16**. Por ejemplo, si los números desde a0 en adelante son

``
200 -100 1 0 100 -1 1 -100 1 ...
``

los números de punto de los vértices de los triángulos así definidos son (`200,100,101`), (`101,201,200`),
(`201,101,102`), ...

`n` y `sig` son **uint32**. `n`es el número de triángulos definidos. `sig` es la posición del siguiente bloque,
en words contados desde el comienzo del presente bloque. Si `sig`es&nbsp;`Я` significa que no hay siguiente bloque.
Si&nbsp;`n` es&nbsp;`Я` significa que ya se acabó. Hay pues dos manera posibles de indicar el final.

El primer triángulo definido en estos bloques tiene por número de punto el indicado en&nbsp;[W15].

### Triángulos individuales

``
a b c  a b c  a b c  ...
``

El número de triángulos es el indicado en&nbsp;[W19]. Cada triángulo está especificado por sus tres vértices, `a b c`, escritos
en sentido antihorario, vistos desde la cara visible del triángulo. Cada vértice es un **uint32**, que indica el número de punto.
El número de triángulo del primer triángulo almacenado aquí es [W15]+[W17], y el del último es [W15]+[W17]+[W19]-1.


## Estilos

### Estilos de los puntos

Por definir

### Estilos de los triángulos

``
tipo_def nestilos estilo0 estilo1 ...
``

Este bloque define los estilos de triángulos. `tipo_def` es un número que indica el tipo de definición presente.
Los estilos pueden ser más o menos ricos, contener una u otra información. Se definen por ello distintas
estructuras de estilo, cada una de ellas con una información y un cierto tamaño. `nestilos` es el número de
definiciones de estilo que siguen. Estas se sitúan una a continuación de otra. Ocupan siempre, cada una, un
número entero de palabras.

Tipos de estilos: Información definida en cada uno de ellos y formato de la estructura _estilo_ correspondiente:

| Tipo de estilo|Información que contiene |
|:---:|---|
| 1 | W0: color
| 2 | W0: material
| 3 | W0: material. W1: color

Por ejemplo, si `tipo_def` es&nbsp;`3`, cada `estilo` ocupa dos palabras. 

El `estilo<n>` se aplica a los triángulos que pertenecen a la clase _n_ (v. infra _Clase a la que pertenece cada
triángulo_), salvo por las excepciones indicadas en _Estilos individuales de los triángulos_.

### Clase a la que pertenece cada punto
``
bits_per_class  cl cl cl cl ...
``

La clase a la que pertenece cada punto es un entero que puede ocupar un número de bits potencia de dos. Por
ejemplo, si solamente se definen 12 clases se puede escribir cada número de clase en cuatro bits. Este número
es `bits_per_class`, que ocupa una palabra. Sus valores permitidos son `0, 1, 2, 4, 8, 16`. Sea
`b` este valor.

A continuación vienen `b*n_puntos` bits (más bits vacíos si es necesario para completar la última palabra).
El valor `n_puntos` es la suma de los definidos en todos los modos posibles, de acuerdo a lo indicado en la
cabecera; es decir, [W9]+[W11]+[W13]. Si `b` es&nbsp;`0` no sigue por tanto ninguna palabra. En ese caso se entiende
que todos los puntos pertenecen a la clase 0. Esto es útil si todos los puntos tienen el mismo estilo salvo por unos
pocos. Resulta más sencillo especificar el estilo de estos pocos en el bloque _Estilos individuales de los puntos_
que tener que indicar aquí la clase a la que pertenece cada uno de todos los puntos del fichero.

### Clase a la que pertenece cada triángulo
``
bits_per_class  cl cl cl cl ...
``

La clase a la que pertenece cada triángulo es un entero que puede ocupar un número de bits potencia de dos. Por
ejemplo, si solamente se definen 12 clases se puede escribir cada número de clase en cuatro bits. Este número
es `bits_per_class`, que ocupa una palabra. Sus valores permitidos son `0, 1, 2, 4, 8, 16`. Sea
`b` este valor.

A continuación vienen `b*n_triángulos` bits (más bits vacíos si es necesario para completar la úlima
palabra). El valor `n_triángulos` es la suma de los definidos en todos los modos posibles, de acuerdo a lo
indicado en la cabecera; es decir, [W15]+[W17]+[W19]. Si `b` es&nbsp;`0` no sigue por tanto ninguna palabra. En
ese caso se entiende que todos los triángulos pertenecen a la clase 0. Esto es útil si todos los triángulos del TIN
tienen el mismo estilo salvo por unos pocos. Resulta más sencillo especificar el estilo de estos pocos en el bloque
_Estilos individuales de los triángulos_ que tener que indicar aquí la clase a la que pertenece cada uno de todos
los triángulos del TIN.

### Estilos individuales de los triángulos

Consta de varios bloques. Cada uno es así

W0: `sig`, W1: `t_estilo` W2- ... : `...`

`sig` indica la posición del siguiente bloque, en words contados desde el comienzo del presente bloque.
`t_estilo` es, salvo por los dos bits más altos, el tipo de definición de estilo presente.

Si los dos bits mayores de `t_estilo` son `00`, lo que sigue es `estilo t1 t2 t3 ...` : la definición del estilo,
en el formato indicado por `t_estilo`, seguida de los triángulos a los que se aplica, cerrado por&nbsp;`Я`.

Si los dos bits altos de `t_estilo` son `01`, lo que sigue es `t est t est ...` : pares triángulo/estilo, hasta
un `Я` de cierre. Cada `est` sigue el formato indicado en `t_estilo`.

Si los dos bits altos de `t_estilo` son `10`, lo que sigue es `n t1 est est ....`, siendo&nbsp;`n` el número de estilos
que van a seguir, `t1` el primer triángulo, y a continuación `n` definiciones de estilo, en el formato `t_estilo`,
que se corresponden con los triángulos de número t1, t1+1, t1+2... t1+(n-1).

Después sigue otro bloque. Así, hasta que el valor de `sig` sea&nbsp;`Я` (que indica que no hay siguiente
bloque) o el valor de `t_estilo` sea&nbsp;`Я` (que indica que no hay presente bloque).


# Formatos concretos de ejemplo

## Tin plano con datos float

Se llama tin plano a aquel cuya información geométrica consta únicamente de puntos individuales y
triángulos individuales y, si tiene información de estilos, esta también es "plana", e. d., consta sólo de
estilos individuales, tantos como triángulos hay.

### Cabecera

Las posiciones se indican en palabras (4-bytes) desde el comienzo del fichero.

**W0:**  0x004E4954 = 0, 'N', 'I', 'T'

**W1:**  = 0x00000469 = 0, 0, 3, 'f'

**W2:**  Un valor float que indica las unidades del fichero, en metros. Por ejemplo 1 (metros) o 0'001 (mm).

**W3-W7:** Vacíos

**W8-W11:** `Я`, 0, `Я`, 0.

**W12:** Posición de los puntos

**W13:** Número de puntos: `np`

**W14-W17:** `Я`, 0, `Я`, 0.

**W18:** Posición de los triángulos

**W19:** Número de triángulos: `nt`

**W20-W24:** `Я`, `Я`, `Я`, `Я`, `Я`

**W25:** Posición de la información de color / material

**W24-W39:** Vacíos

Si no existe información de color / material, el valor de la palabra W25 será `Я`.

### Puntos

```
X Y Z  X Y Z  X Y Z  ...
```

Hay en total 3*`np` valores, definiendo `np` puntos. Cada coordenada es un **float**. Estos puntos se identificarán por un
número de punto, que va de `0` a `np-1`.

### Triángulos

```
a b c  a b c  a b c  ...
```

Cada triángulo está definido por los números de punto de sus vértices. Cada valor es un **uint32**. Hay en total 3*`nt` enteros
definiendo `nt` triángulos.

### color / material de los triángulos

``
0x80000001 / 0x80000002  nt 0 c1 c2 ... c(nt)
``

Los valores `c1` ... `c(nt)` son **uint32**. Si la primera palabra de este bloque es `0x80000001` son colores en la forma
0x00RRGGBB; si es `0x80000002` son números de material. El formato TIN no define cómo se interpretan estos números.



## Ficheros generados por 3dCarving (I)

En lo sucesivo `m` y `n` representan el número de filas y de columnas de la parte tallada. Si de un dato no se dice nada se
entiende que es un **sint32** si se trata de una coordenada (o incremento de coordenadas), y **uint32** en cualquier otro caso.
Las posiciones se indican en palabras (4-bytes) desde el comienzo del fichero.

### Cabecera

**W0:**  0x004E4954 = 0, 'N', 'I', 'T'

**W1:**  = 0x00000469 = 0, 0, 4, 'i'

**W2:** `f`. `f` unidades en el sistema de coordenadas del fichero hacen un milímetro.

**W3-W7:** Vacíos

**W8:** Posición de la definición de la retícula de puntos

**W9:** Número total de puntos definidos en la retícula (`m` x `n`)

**W10, W11:** `Я`, 0.

**W12:** Posición de las definiciones individuales de puntos

**W13:** Número de puntos definidos en las definiciones individuales

**W14, W15:** `Я`, 0.

**W16:** Posición de los triángulos con números de vértices definidos en modo incremental

**W17:** Número total de triángulos definidos &ldquo;en incrementos&rdquo; (`2*(m-1)(n-1)`)

**W18:** Posición de las definiciones individuales de triángulos

**W19:** Número de triángulos en las definiciones individuales

**W20:** `Я`

**W21:** Posición de las definiciones de estilos de triángulos

**W22, W23:** `Я`, `Я`

**W24:** Posición de la definición global de estilo de triángulos

**W25:**`Я`

**W24-W39:** Vacíos

### Puntos en retícula

```
m n Я 1  X0 Y0 0  ΔX 0 0 0 ΔY 0 0 0 1  Zs[m*n]
```

`X0, Y0` son las coordenadas de una esquina de la matriz de `m` x `n` puntos. `ΔX, ΔY` son respectivamente
los incrementos al pasar de un punto al siguiente dentro de una fila y al pasar de una fila a la siguiente. `Zs` es un
array de `m*n` enteros de 16 bits con signo, con los valores de `Z` para los puntos de la retícula.

Los números de punto de estos puntos van de `0`  a `m*n-1`; es decir, de `0` a [W9]-1.

### Puntos individuales

```
X Y Z  X Y Z  X Y Z  ...
```

El primer punto definido en este bloque tiene por número de punto&nbsp;[W9]. El número de ternas `X,Y,Z` presentes
es el indicado en&nbsp;[W13]. El último punto presente tiene por tanto por número de punto  [W9]+[W13]-1. Cada coordenada
es un entero de 32 bits con signo.

### Triángulos en icrementos


```
nti Я  a0 Δb Δc  Δa Δb Δc  Δa Δb Δc  ...
```

`nti` es el número de triángulos definidos, y es por tanto igual al valor&nbsp;[W17]. `a0` es el primer vértice del primer
triángulo; es un **uint32**. Los vértices sucesivos se definen indicando el incremento respecto al anterior. Estos incrementos
son de tipo **sint16**. Por ejemplo, si los números desde a0 en adelante son

``
200 -100 1 0 100 -1 1 -100 1 ...
``

los números de punto de los vértices de los triángulos así definidos son (`200,100,101`), (`101,201,200`),
(`201,101,102`), ...

El número total de incrementos presentes es pues `3*nti-1`. El primero de los triángulos definidos aquí tiene por
número de triángulo `0`, minetras que el último tiene por número de triángulo `nti-1`.

### Triángulos individuales

```
a b c  a b c  a b c  ...
```

El número de triángulos es el indicado en&nbsp;[W19]. Cada triángulo está especificado por sus tres vértices, `a b c`, escritos
en sentido antihorario, vistos desde la cara visible del triángulo. Cada vértice es un **uint32**, que indica el número de punto.
El número de triángulo del primer triángulo almacenado aquí es&nbsp;[W17]; el del último es [W17]+[W19]-1.

### Estilos de los triángulos

```
1 3  color0 color1 color2
```

El entero `3` indica que siguen tres valores de color. `color0`, etc. son los colores con los que se han de mostrar
los triángulos según su clase. Si en el futuro se añaden más clases se tendrá otro número en lugar del `3`, seguido
de los correspondientes colores.

### Clase a la que pertenece cada triángulo

Sea `nt` el número total de triángulos del modelo (igual a [W17]+[W19]).

```
2  clases[(nt+15)/16]
```

`clases` es un array de _words_ (**uint32**) conteniendo la clase a la que pertence cada triángulo. Cada una de estas clases
ocupa dos bits y puede valer `0` (tierra), `1` (agua) y `2` (marco). Por tanto en el array clases hay que leer `nt` valores
de 2 bits cada uno. Para almacenar esto son necesarios `(nt+15)/16` **uint32**'s.



## Ficheros generados por 3dCarving (II)

Sigue siendo `m` y `n` el número de filas y de columnas de la matriz tallada. Este formato es como el anterior, con las siguientes variaciones:

**W14:** Posición de los triángulos en malla

**W15:** Número total de triángulos de la malla (`2*(m-1)(n-1)`)

**W16, W17** `Я`, 0.

El número de triángulos `nti` que antes estaba en&nbsp;[W17] ahora están en&nbsp;[W15]. Los triángulos en “incrementos” no existen
y en su lugar se encuentran los

### Triángulos en malla

```
μ ν  a0 b0 c0  1 1 1 n n n
μ ν  a1 b1 c1  1 1 1 n n n
Я
```

Los enteros `μ` y `ν` son iguales a `m-1` y `n-1` respectivamente. Los valores `a0,b0,c0` son los vértices del primer triángulo
de la primera serie, que en los ficheros que genera 3dCarving son siempre `n,n+1,0`. Los valores `a1,b1,c1` son los vértices del
primer triángulo de la segunda serie, que en los ficheros que genera 3dCarving son siempre `1,0,n+1`. Estos dos triángulos por tanto
cubren el primer cuadrado de la malla, de esquinas `0,1,n,n+1`. Los restantes triángulos cubren los restantes cuadrados de la malla.

En total hay [W15] = `μν = (m-1)(n-1)` triángulos en cada serie. Los números de vértice de los triángulos de la primera serie son
_k_+`a0`, _k_+`b0`, _k_+`c0`; los de la segunda serie son _k_+`a1`, _k_+`b1`, _k_+`c1`, en donde

_k_ = 0 ... ν-1,` `n ... n+ν-1,` `2n ... 2n+ν-1,` `... ,` `(μ-1)n ... (μ-1)n+ν-1
