# Guion de Exposición Oral y Demostración en Vivo (Listo para Leer)

**Asignatura**: Computación Gráfica I (`IF455AIN`) - UNSAAC `2026-02`  
**Docente**: PhD. Carlos Fernando Montoya Cubas  
**Expositores**:
- **Ronal** (`CHOQUENAIRA GARCIA, Ronal Franklin` - `174941`)
- **Lucian** (`FERNANDEZ BACA CASTRO, Lucian Neptalí` - `184197`)

**Presentación base**: [Presentacion_Tema2.pdf](Presentacion_Tema2.pdf) / [Presentacion_Tema2.pptx](Presentacion_Tema2.pptx) (12 diapositivas)

---

## Resumen de distribución equitativa (50% / 50%)

La exposición cubre las 6 orientaciones del enunciado en estricto orden de diapositivas (`1` a `12`), con una duración objetivo de **11 minutos y 15 segundos**, repartida equitativamente y dejando la implementación, la ejecución en vivo del programa y el cierre en manos de **Lucian**:

| Bloque | Expositor | Diapositivas | Contenido principal | Tiempo estimado |
| :--- | :--- | :--- | :--- | :--- |
| **Bloque 1** | **Ronal** | Diapositivas `1` a `4` | Carátula breve, planteamiento visual, convención $p'=Mp$, derivación $\mathrm{Ref}_\phi$, papel de $\det(A)$ e invariantes | `3 min 15 s` |
| **Bloque 2** | **Lucian** | Diapositiva `5` | Estructura del código C++, funciones propias y flujo en rejilla `160x120` | `0 min 50 s` |
| **Bloque 3** | **Ronal** | Diapositivas `6` a `8` | Resultados cuantitativos ($H_x$, $\mathrm{Ref}_y$), contraste manual en $V_4$, variación de área y caso singular | `2 min 30 s` |
| **Bloque 4** | **Lucian** | Diapositivas `9` a `12` + **Demo en vivo** | Defensa técnica, limitación raster, **ejecución interactiva del programa C++**, conclusiones y cierre general de fuentes | `4 min 40 s` |
| **Total** | **Ronal / Lucian** | **12 diapositivas + Demo** | **Ronal: `5 min 45 s` (51%) / Lucian: `5 min 30 s` (49%)** | **`11 min 15 s`** |

---

## Preparación técnica (1 minuto antes de iniciar)

1. Abrir la presentación (`Presentacion_Tema2.pdf` o `Presentacion_Tema2.pptx`) en pantalla completa.
2. Tener lista una terminal de WSL2 (`Ubuntu`) en `src/` (o `src_unico/` si el docente pide archivo único) con el ejecutable ya compilado:
   ```bash
   cmake -S . -B build && cmake --build build -j
   ./build/test_experiments ../experimentos
   ```
3. Al llegar a la Diapositiva `10`, cambiar a WSL2 y ubicar la ventana gráfica `1280x960` a la izquierda y la consola a la derecha (cada pulsación limpia y actualiza el menú textual y la matriz $3\times 3$ en el mismo sitio).

---

## Guion completo para lectura (con transiciones y cambios de diapositiva)

### BLOQUE 1 - RONAL (Diapositivas 1 a 4 | ~3 min 15 s)

#### [DIAPOSITIVA 1 - Carátula | ~15 segundos]
> **Ronal:**  
> "Buenos días profesor, buenos días compañeros. Somos Ronal Choquenaira García y Lucian Fernández Baca Castro, y hoy presentaremos nuestro trabajo sobre **Transformaciones afines, deformación y propiedades geométricas en 2D** para el curso de Computación Gráfica I. Pasemos directamente al problema que motiva este estudio."

**[CAMBIO A DIAPOSITIVA 2]**

#### [DIAPOSITIVA 2 - Planteamiento del problema: ¿por qué no basta la apariencia visual? | ~45 segundos]
> **Ronal:**  
> "En computación gráfica suele ocurrir que una figura transformada nos engaña a simple vista. Observen este cuadrado de cuatro por cuatro unidades, con ángulos de noventa grados y un área de dieciséis unidades cuadradas.  
> Al aplicarle un cizallamiento horizontal con factor cero punto siete, el ángulo interno cae de noventa grados a cincuenta y cinco punto cero un grados y el lado lateral crece un veintidós punto uno por ciento, pasando de cuatro a cuatro punto ochenta y ocho unidades. Visualmente parece que la figura se agrandó; sin embargo, como el determinante del cizallamiento vale uno, su área sigue siendo exactamente dieciséis unidades cuadradas.  
> Por eso no basta dibujar figuras estáticas ni confiar en la intuición visual: nuestro objetivo fue construir un laboratorio interactivo en C++17 sobre un polígono asimétrico de seis vértices, una cuadrícula transformable y dos segmentos paralelos para verificar cada propiedad de manera cuantitativa. Veamos ahora la formalización matemática que sostiene este laboratorio."

**[CAMBIO A DIAPOSITIVA 3]**

#### [DIAPOSITIVA 3 - Convención matemática y derivación clave de $\mathrm{Ref}_\phi$ | ~1 min 15 s]
> **Ronal:**  
> "Trabajamos con la convención estándar de vectores columna homogéneos, donde cada punto se representa como $p = (x, y, 1)^T$ y se transforma mediante el producto $p' = M p$. Bajo esta convención, cuando componemos varias matrices en $M = A \cdot B \cdot C$, la multiplicación se lee de derecha a izquierda: actúa primero $C$, luego $B$ y al final $A$.  
> Respecto al sistema de coordenadas, la proyección ortográfica `glOrtho(0, 160, 0, 120)` fija el eje $Y$ creciendo hacia arriba, exactamente igual que en el plano cartesiano matemático; y al exportar capturas PPM, invertimos explícitamente la fila vertical para mantener esa coherencia.  
> En la parte inferior mostramos la deducción de una composición clave del tema: la reflexión respecto a una recta oblicua por el origen con ángulo $\phi$, ejemplificada para treinta grados. Como se aprecia en las cuatro etapas, primero rotamos la escena menos treinta grados para alinear la recta con el eje $X$; segundo, aplicamos la reflexión elemental $\mathrm{Ref}_x$; y tercero, rotamos más treinta grados para devolver el eje a su posición original. El producto $R(\phi)\,\mathrm{Ref}_x\,R(-\phi)$ entrega la matriz cerrada con $\cos(2\phi)$ y $\sin(2\phi)$, cuyo determinante es exactamente menos uno. Esto nos lleva a analizar qué controla exactamente el determinante."

**[CAMBIO A DIAPOSITIVA 4]**

#### [DIAPOSITIVA 4 - Papel de $\det(A)$ e invariantes afines | ~1 min 00 s]
> **Ronal:**  
> "El determinante de la parte lineal $A$ determina por completo tres comportamientos geométricos que vemos en pantalla.  
> Primero, cuando $\det(A)$ es positivo, la orientación del recorrido de los vértices se conserva y el área física escala exactamente por $|\det(A)|$.  
> Segundo, cuando $\det(A)$ es negativo, como en una reflexión, el área física sigue siendo positiva porque depende del valor absoluto, pero la orientación se invierte: un polígono antihorario pasa a horario.  
> Y tercero, cuando $\det(A)$ es cero, estamos ante una matriz singular: el plano bidimensional colapsa hacia una recta o un punto, el área cae a cero y se pierden la inyectividad y la invertibilidad.  
> En resumen, mientras $\det(A)$ sea distinto de cero, toda transformación afín conserva rectas, colinealidad, paralelismo y razones de segmentos colineales; en cambio, longitudes y ángulos se deforman salvo en isometrías.  
> Para ver cómo trasladamos todo este modelo al código sin usar funciones automáticas de OpenGL, le doy el pase a mi compañero Lucian."

**[CAMBIO A DIAPOSITIVA 5 - PASE A LUCIAN]**

---

### BLOQUE 2 - LUCIAN (Diapositiva 5 | ~50 segundos)

#### [DIAPOSITIVA 5 - Implementación en C++: flujo matricial y rasterización `160x120` | ~50 segundos]
> **Lucian:**  
> "Gracias, Ronal. En esta diapositiva se muestra el flujo completo de nuestra implementación en C++17, donde distinguimos claramente lo reutilizado del curso y lo construido por el equipo.  
> Del entorno de laboratorio del curso reutilizamos las rutinas base de rasterización: `putPixel()` y `putPoint()` para celdas individuales, el algoritmo de Bresenham en los ocho octantes, `drawPolygon()` y la rejilla visible `drawGrid()`.  
> Todo el núcleo matemático y analítico fue codificado desde cero por nosotros sin usar `glTranslatef`, `glRotatef`, `glScalef` ni bibliotecas externas: las seis transformaciones elementales en matrices $3\times 3$, la composición en orden de aplicación, la guarda de singularidad con tolerancia $10^{-9}$, el área con signo mediante la fórmula del zapatero y la métrica de paralelismo con vectores directores.  
> Además, las métricas se calculan siempre en doble precisión antes de proyectar los vértices a la rejilla discreta de $160 \times 120$ celdas a cuatro píxeles por unidad. Ahora Ronal nos mostrará los resultados cuantitativos obtenidos con el programa y el contraste con el cálculo manual."

**[CAMBIO A DIAPOSITIVA 6 - PASE A RONAL]**

---

### BLOQUE 3 - RONAL (Diapositivas 6 a 8 | ~2 min 30 s)

#### [DIAPOSITIVA 6 - Pruebas de cizallamiento horizontal $H_x(0.7)$ y reflexión vertical $\mathrm{Ref}_y$ | ~45 segundos]
> **Ronal:**  
> "A la izquierda tenemos la captura real del programa para el cizallamiento horizontal $H_x(0.7)$. Los dos segmentos paralelos originales pasan a $L'_1$ y $L'_2$ compartiendo exactamente el mismo vector director $u' = v' = (10.8, 4.0)$, por lo que el seno normalizado entre directores es cero y el paralelismo se conserva de forma exacta. El área del polígono permanece en veinticuatro unidades cuadradas, pero el ángulo recto en $V_0$ desciende a cincuenta y cinco punto cero un grados y la arista vertical se alarga de cinco a seis punto diez unidades.  
> A la derecha, la reflexión respecto al eje vertical invierte el signo de todas las abscisas: el determinante es menos uno, el área con signo pasa de más veinticuatro a menos veinticuatro indicando el cambio de sentido antihorario a horario, mientras el área física sigue intacta en veinticuatro. Ahora veamos qué ocurre cuando componemos dos transformaciones e invertimos su orden."

**[CAMBIO A DIAPOSITIVA 7]**

#### [DIAPOSITIVA 7 - Efecto del orden: no conmutatividad y contraste manual en $V_4 = (5, 7)$ | ~1 min 00 s]
> **Ronal:**  
> "Para comprobar la no conmutatividad comparamos $R(45^\circ)\,H_x(0.7)$ en azul frente a $H_x(0.7)\,R(45^\circ)$ en naranja sobre el vértice de referencia $V_4 = (5, 7)$, verificando el resultado a mano.  
> En el primer orden, el cizallamiento lleva $(5, 7)$ a $(9.9, 7.0)$, y al rotarlo cuarenta y cinco grados con $\frac{\sqrt{2}}{2} \approx 0.7071$ obtenemos $(2.0506,\; 11.9501)$.  
> En el segundo orden, la rotación lleva primero $(5, 7)$ a $(-1.4142,\; 8.4853)$, y al sumar $0.7 \times 8.4853$ a la abscisa obtenemos $(4.5255,\; 8.4853)$.  
> Ambos órdenes coinciden exactamente con lo reportado por nuestro programa y ambos conservan el área en veinticuatro unidades cuadradas, pero la posición final de $V_4$ difiere en una distancia euclidiana de **cuatro punto veinticinco setenta y nueve unidades**, además de producir ángulos y longitudes de arista distintos. Pasemos a la prueba de variación de área y al caso degenerado."

**[CAMBIO A DIAPOSITIVA 8]**

#### [DIAPOSITIVA 8 - Variación de área ($|\det|=1.20$) y caso degenerado singular ($\det=0$) | ~45 segundos]
> **Ronal:**  
> "A la izquierda aplicamos una composición de cuatro transformaciones: cizallamiento $H_x(0.5)$, escalamiento $S(1.5, 0.8)$, rotación $R(25^\circ)$ y traslación $T(2, -2)$. El determinante de la matriz compuesta resulta exactamente uno punto veinte, y el área calculada por la fórmula del zapatero da veintiocho punto ochenta unidades cuadradas, coincidiendo con $1.2 \times 24$ con un error de máquina de $7.11 \times 10^{-15}$.  
> En contraste, a la derecha aplicamos el escalamiento singular $S(1.0, 0.0)$, donde el determinante se anula. Toda la figura y ambos segmentos paralelos colapsan sobre el eje $y = 0$: el área cae a cero, $L'_1$ y $L'_2$ dejan de ser rectas distintas al superponerse sobre el mismo segmento, y el programa bloquea cualquier cálculo de inversa.  
> Con esto le devuelvo el pase a Lucian para cerrar el análisis crítico y mostrar el programa funcionando en vivo."

**[CAMBIO A DIAPOSITIVA 9 - PASE A LUCIAN]**

---

### BLOQUE 4 - LUCIAN (Diapositivas 9 a 12 + Demo en vivo | ~4 min 40 s)

#### [DIAPOSITIVA 9 - Casos engañosos, limitaciones de rasterización y defensa técnica | ~55 segundos]
> **Lucian:**  
> "Antes de ejecutar el programa, resumimos tres preguntas frecuentes donde la intuición suele fallar y una limitación real del trazado raster.  
> Primero: ¿puede una transformación conservar el área y alterar los ángulos? Sí, siempre que $|\det(A)| = 1$ pero $A$ no sea ortogonal, exactamente como comprobamos con $H_x(0.7)$ donde el área sigue en veinticuatro mientras el ángulo en $V_0$ baja a cincuenta y cinco grados.  
> Segundo: ¿por qué un determinante negativo no significa área negativa en sentido físico? Porque el área física siempre es no negativa y depende de $|\det(A)|$; el signo negativo únicamente indica inversión de orientación, de antihorario a horario.  
> Tercero: ¿por qué una matriz singular requiere tratamiento especial? Porque al colapsar la dimensión destruye la inyectividad, y dividir entre $\det(A) = 0$ produciría divisiones por cero o valores indefinidos en el código; por eso verificamos $|\det(A)| < 10^{-9}$ antes de invertir.  
> Finalmente, reportamos honestamente la restricción de discretización: aunque nuestras métricas operan en doble precisión con error de $10^{-15}$, al rasterizar en la rejilla de $160 \times 120$ se redondea cada vértice a la celda entera más próxima, introduciendo un desplazamiento puramente visual de hasta media celda, equivalente a cero punto ciento veinticinco unidades. Ahora pasemos a la demostración en vivo del programa."

**[CAMBIO A DIAPOSITIVA 10]**

#### [DIAPOSITIVA 10 + EJECUCIÓN EN VIVO DEL PROGRAMA C++ | ~2 min 55 s]
> **Lucian (breve introducción sobre la Slide 10 - ~15 segundos):**  
> "En esta lámina están resumidos los controles de nuestro laboratorio. Voy a cambiar ahora a la terminal en WSL2 para ejecutar primero las pruebas automatizadas y luego abrir el laboratorio gráfico interactivo."

**[CAMBIAR A VENTANA WSL2 + OPENGL]**

> **Lucian (paso a paso mientras opera la terminal y la ventana gráfica - ~2 min 40 s):**
> 1. *(En terminal ejecuta `./build/test_experiments ../experimentos`)*:  
>    "Primero corremos la validación numérica sin ventana: todas las comprobaciones pasan con error menor a $10^{-9}$ y exportan los resultados reproducibles."
> 2. *(Ejecuta `./build/lab_afines`, queda en modo `0` - Base)*:  
>    "Ahora abrimos el laboratorio gráfico. A la izquierda está nuestra ventana de $1280 \times 960$ píxeles sobre la rejilla discreta de $160 \times 120$, y a la derecha el panel en consola que se limpia en cada pulsación para mostrar siempre el menú textual de todas las transformaciones junto a la matriz $3\times 3$ y las métricas en vivo. En el modo cero vemos la escena base: el polígono asimétrico de seis vértices con área más veinticuatro, la cuadrícula y los segmentos paralelos $L_1$ y $L_2$."
> 3. *(Muestra las transformaciones elementales con sus accesos directos: `T`, `9`, `K`, `1`, `2`, `3`, `4`)*:  
>    - *(Pulsa `T` y mueve con `Flechas`)*: "Con `T` aplicamos **traslación** pura y vemos cómo cambia la tercera columna de la matriz manteniendo área y ángulos intactos."
>    - *(Pulsa `9` y gira con `Q`/`E`)*: "Con `9` aplicamos **rotación** pura, que conserva área, orientación y longitudes."
>    - *(Pulsa `K` y ajusta con `Z`/`X` y `W`/`S`)*: "Con `K` aplicamos **escalamiento** puro, donde el área varía según $s_x \cdot s_y$."
>    - *(Pulsa `1`, varía un paso con `+` y vuelve con `-` a `0.7`)*: "Con `1` vemos el **cizallamiento horizontal** $H_x(0.7)$: el paralelismo sigue en seno cero, el área se mantiene en veinticuatro, pero el ángulo en $V_0$ cae a cincuenta y cinco punto cero un grados y la arista $V_5 V_0$ sube a seis punto diez. Con `2` ocurre lo equivalente en **cizallamiento vertical**."
>    - *(Pulsa `3`, luego `F` para alternar $\mathrm{Ref}_x$, y luego `4`)*: "Con `3` aplicamos la **reflexión** respecto al eje vertical, donde el área con signo cambia a menos veinticuatro e invierte la orientación a horaria; con `F` alternamos al eje horizontal, y con `4` vemos la reflexión sobre la recta oblicua de treinta grados."
> 4. *(Pulsa `5`, luego `V` un par de veces y vuelve a `V4`)*:  
>    "Con `5` superponemos los dos órdenes de composición: $R(45^\circ)H_x(0.7)$ en azul y $H_x(0.7)R(45^\circ)$ en naranja. En consola aparece la brecha exacta de $4.2579$ unidades en $V_4$, y con la tecla `V` podemos rotar el vértice seleccionado para inspeccionar cualquier vértice del polígono."
> 5. *(Pulsa `6`)*:  
>    "Con `6` aplicamos la composición de cuatro pasos con determinante uno punto veinte, confirmando en vivo el área de veintiocho punto ochenta igual a $|\det(A)|$ por veinticuatro."
> 6. *(Pulsa `7`, luego `G` para alternar rejilla)*:  
>    "Con `7` vemos el caso degenerado $S(1, 0)$: la figura y los segmentos colapsan sobre $y = 0$, el ángulo en $V_0$ queda marcado como indefinido y la consola emite la advertencia de matriz singular sin calcular inversa. Y pulsando `G` podemos activar o desactivar la rejilla discreta de píxeles."
> 7. *(Pulsa `8`, mueve `A`/`D`, baja `S` hasta `0.0` y pulsa `O`)*:  
>    "Por último, en el modo `8` combinamos todas las transformaciones en vivo: podemos variar el cizallamiento, bajar $s_y$ hasta cero para provocar el colapso singular en tiempo real y pulsar `O` para invertir el orden de la cadena. Si se requiere una secuencia personalizada de tres o más pasos arbitrarios, la tecla `C` abre el constructor interactivo en consola. Regresemos a la presentación para cerrar."

**[VOLVER A LA PRESENTACIÓN - CAMBIO A DIAPOSITIVA 11]**

#### [DIAPOSITIVA 11 - Conclusiones | ~35 segundos]
> **Lucian:**  
> "Para concluir, destacamos cuatro puntos técnicos centrales:  
> Primero, las coordenadas homogéneas en matrices $3\times 3$ unifican todas las transformaciones afines en un único modelo verificable a precisión de máquina.  
> Segundo, toda transformación afín invertible preserva paralelismo, colinealidad y razones de segmentos, pero altera longitudes y ángulos fuera de las isometrías, y su composición no es conmutativa.  
> Tercero, el determinante de la parte lineal controla simultáneamente el escalamiento de área, la orientación y la invertibilidad; cuando es nulo, la matriz singular exige bloquear la inversión.  
> Y cuarto, separar el cálculo continuo en doble precisión de la rasterización discreta en rejilla $160 \times 120$ garantiza coherencia exacta entre los gráficos y las métricas."

**[CAMBIO A DIAPOSITIVA 12]**

#### [DIAPOSITIVA 12 - Fuentes bibliográficas y cierre | ~15 segundos]
> **Lucian:**  
> "Todo el desarrollo matemático, las definiciones geométricas y los criterios de rasterización se respaldan en la bibliografía oficial de computación gráfica y matemáticas para gráficos del curso, que se muestra listada en pantalla junto con sus enlaces en el informe.  
> Eso sería todo por nuestra parte. Muchas gracias por su atención y quedamos atentos a sus preguntas."

---

## Notas rápidas para preguntas del docente

- **Si el docente pide ejecutar la versión en un solo archivo**: entrar a `src_unico/`, compilar con `cmake -S . -B build && cmake --build build -j` y correr `./build/lab_afines_unico --self-test` o `./build/lab_afines_unico` (contiene exactamente las mismas matrices, menú textual y controles).
- **Si el docente pide construir una composición nueva de 3 o más pasos en vivo**: pulsar **`C`**, escribir `3` (o `4`) y elegir cualquier combinación de `[1:T  2:R  3:S  4:Hx  5:Hy  6:Ref_x  7:Ref_y  8:Ref_phi]` para ver de inmediato el orden aplicado, la matriz $3\times 3$ resultante, $\det(A)$, área y paralelismo.
