# Guion de Exposición Oral y Demostración en Vivo (Un Solo Cambio de Expositor)

**Asignatura**: Computación Gráfica I (`IF455AIN`) - UNSAAC `2026-02`  
**Docente**: PhD. Carlos Fernando Montoya Cubas  
**Expositores**:
- **Ronal** (`CHOQUENAIRA GARCIA, Ronal Franklin` - `174941`)
- **Lucian** (`FERNANDEZ BACA CASTRO, Lucian Neptalí` - `184197`)

**Presentación base**: [Presentacion_Tema2.pdf](Presentacion_Tema2.pdf) / [Presentacion_Tema2.pptx](Presentacion_Tema2.pptx) (12 diapositivas)

---

## Resumen de distribución equitativa (50% / 50% con un único cambio)

Cada integrante expone su bloque completo de corrido, respetando estrictamente el orden de las 12 diapositivas y las 6 orientaciones del enunciado:

| Parte | Expositor | Diapositivas | Contenido completo del bloque | Tiempo estimado |
| :--- | :--- | :--- | :--- | :--- |
| **Parte 1** | **Ronal** | Diapositivas `1` a `7` | Carátula breve, planteamiento visual, convención $p'=Mp$, derivación $\mathrm{Ref}_\phi$, papel de $\det(A)$, resultados ($H_x$, $\mathrm{Ref}_y$), contraste manual en $V_4$, cambio de área y caso singular | `5 min 35 s` (50%) |
| **Parte 2** | **Lucian** | Diapositivas `8` a `12` + **Demo en vivo** | Respuestas a las 3 preguntas de defensa, limitación raster, estructura del código C++ (`160x120`), **ejecución interactiva del programa**, conclusiones y mención general de fuentes | `5 min 35 s` (50%) |
| **Total** | **1 solo cambio** | **12 diapositivas + Demo** | **Ronal (`1` a `7`) -> Pase único -> Lucian (`8` a `12` + Demo)** | **`11 min 10 s`** |

---

## Preparación técnica (1 minuto antes de iniciar)

1. Abrir la presentación (`Presentacion_Tema2.pdf` o `Presentacion_Tema2.pptx`) en pantalla completa desde la Diapositiva `1`.
2. Tener lista una terminal de WSL2 (`Ubuntu`) en `src/` (o `src_unico/` si se solicita versión en archivo único) con el programa compilado:
   ```bash
   cmake -S . -B build && cmake --build build -j
   ./build/test_experiments ../experimentos
   ```
3. Al llegar a la Diapositiva `10`, cambiar a WSL2 y ubicar la ventana gráfica `1280x960` a la izquierda y la terminal a la derecha (el menú está agrupado por transformaciones elementales, composiciones y controles en vivo, y se refresca sin acumular texto).

---

## Guion completo listo para leer (con transiciones y cambios de diapositiva)

### PARTE 1 - RONAL (Diapositivas 1 a 7 | ~5 min 35 s)

#### [DIAPOSITIVA 1 - Carátula | ~15 segundos]
> **Ronal:**  
> "Buenos días profesor, buenos días compañeros. Somos Ronal Choquenaira García y Lucian Fernández Baca Castro, y presentaremos el tema **Transformaciones afines, deformación y propiedades geométricas en 2D**. Pasemos de inmediato al problema que motiva nuestro trabajo."

**[CAMBIO A DIAPOSITIVA 2]**

#### [DIAPOSITIVA 2 - Planteamiento del problema: ¿por qué no basta la apariencia visual? | ~45 segundos]
> **Ronal:**  
> "En computación gráfica es fácil equivocarse si evaluamos una transformación solo a simple vista. Observen este cuadrado de cuatro por cuatro unidades, con ángulos de noventa grados y un área de dieciséis unidades cuadradas.  
> Cuando aplicamos el cizallamiento horizontal $H_x(0.7)$, el ángulo interno cae a cincuenta y cinco punto cero un grados y el lado lateral crece un veintidós punto uno por ciento, pasando de cuatro a cuatro punto ochenta y ocho unidades. Aunque visualmente la figura parece más grande, como el determinante vale uno, su área sigue siendo exactamente dieciséis unidades cuadradas.  
> Por ello no basta dibujar figuras estáticas: implementamos un laboratorio interactivo en C++17 sobre un polígono asimétrico de seis vértices, una cuadrícula transformable y dos segmentos paralelos para medir cada propiedad con precisión. Veamos la formalización matemática."

**[CAMBIO A DIAPOSITIVA 3]**

#### [DIAPOSITIVA 3 - Convención matemática y derivación clave de $\mathrm{Ref}_\phi$ | ~1 min 15 s]
> **Ronal:**  
> "Adoptamos la convención de vectores columna homogéneos, donde cada punto es $p = (x, y, 1)^T$ y su imagen es $p' = M p$. En una composición $M = A \cdot B \cdot C$, las transformaciones se aplican de derecha a izquierda: primero $C$, luego $B$ y al final $A$.  
> Respecto a los ejes, la proyección ortográfica `glOrtho(0, 160, 0, 120)` fija el eje $Y$ creciendo hacia arriba, coherente con el plano cartesiano; y al exportar capturas PPM, invertimos explícitamente la fila vertical para conservar esa misma orientación.  
> En la figura inferior mostramos la deducción de una composición clave: la reflexión respecto a una recta por el origen con ángulo $\phi$, ilustrada para treinta grados. Primero rotamos menos treinta grados para alinear la recta con el eje $X$; segundo aplicamos la reflexión elemental $\mathrm{Ref}_x$; y tercero rotamos más treinta grados para devolver la recta a su inclinación original. El producto $R(\phi)\,\mathrm{Ref}_x\,R(-\phi)$ entrega la matriz cerrada con $\cos(2\phi)$ y $\sin(2\phi)$, cuyo determinante es menos uno. Analicemos ahora el rol del determinante."

**[CAMBIO A DIAPOSITIVA 4]**

#### [DIAPOSITIVA 4 - Papel de $\det(A)$ e invariantes afines | ~55 segundos]
> **Ronal:**  
> "El determinante de la submatriz lineal $A$ gobierna tres regímenes bien definidos.  
> Cuando $\det(A)$ es positivo, se conserva la orientación de los vértices y el área física escala exactamente por $|\det(A)|$.  
> Cuando $\det(A)$ es negativo, como en una reflexión, el área física se mantiene positiva porque depende del valor absoluto, pero la orientación se invierte de antihoraria a horaria.  
> Y cuando $\det(A)$ es cero, la matriz es singular: el plano bidimensional colapsa hacia una recta o un punto, el área se anula y no existe transformación inversa.  
> En síntesis, toda transformación afín invertible conserva rectas, colinealidad, paralelismo y razones de segmentos colineales, mientras que longitudes y ángulos se alteran salvo en isometrías. Veamos ahora los resultados experimentales obtenidos con nuestra escena de prueba."

**[CAMBIO A DIAPOSITIVA 5]**

#### [DIAPOSITIVA 5 - Pruebas de cizallamiento horizontal $H_x(0.7)$ y reflexión vertical $\mathrm{Ref}_y$ | ~45 segundos]
> **Ronal:**  
> "A la izquierda tenemos la captura real en rejilla $160\times 120$ del cizallamiento horizontal $H_x(0.7)$. Los segmentos paralelos originales pasan a $L'_1$ y $L'_2$ con el mismo vector director $u' = v' = (10.8, 4.0)$, por lo que el seno normalizado es cero y el paralelismo se conserva de forma exacta. El área sigue en veinticuatro unidades cuadradas, pero el ángulo interno en $V_0$ cae de noventa a cincuenta y cinco punto cero un grados y la arista vertical se alarga de cinco a seis punto diez unidades.  
> A la derecha, la reflexión vertical $\mathrm{Ref}_y$ cambia el signo de las abscisas: el determinante es menos uno, el área con signo pasa de más veinticuatro a menos veinticuatro confirmando el giro horario, mientras el área física permanece intacta en veinticuatro. Pasemos ahora al efecto del orden en la composición."

**[CAMBIO A DIAPOSITIVA 6]**

#### [DIAPOSITIVA 6 - Efecto del orden: no conmutatividad y contraste manual en $V_4 = (5, 7)$ | ~1 min 00 s]
> **Ronal:**  
> "Aquí contrastamos la composición $R(45^\circ)\,H_x(0.7)$ en azul frente a $H_x(0.7)\,R(45^\circ)$ en naranja sobre el vértice elegido $V_4 = (5, 7)$, validándolo con cálculo manual.  
> En el primer orden, el cizallamiento lleva $(5, 7)$ a $(9.9, 7.0)$, y al rotarlo cuarenta y cinco grados multiplicando por $\frac{\sqrt{2}}{2} \approx 0.7071$ llegamos a $(2.0506,\; 11.9501)$.  
> En el segundo orden, la rotación lleva primero $(5, 7)$ a $(-1.4142,\; 8.4853)$, y el cizallamiento suma $0.7 \times 8.4853$ a la abscisa, produciendo $(4.5255,\; 8.4853)$.  
> Ambos valores coinciden exactamente con la consola del programa y ambos conservan el área en veinticuatro, pero existe una brecha euclidiana de **cuatro punto veinticinco setenta y nueve unidades** en $V_4$, además de distintos ángulos y longitudes de arista. Veamos finalmente el cambio de área y el caso singular."

**[CAMBIO A DIAPOSITIVA 7]**

#### [DIAPOSITIVA 7 - Variación de área ($|\det|=1.20$) y caso degenerado singular ($\det=0$) | ~50 segundos]
> **Ronal:**  
> "A la izquierda aplicamos la cadena de cuatro transformaciones $H_x(0.5)$, escalamiento $S(1.5, 0.8)$, rotación $R(25^\circ)$ y traslación $T(2, -2)$. El determinante compuesto es uno punto veinte y el área medida por la fórmula del zapatero da veintiocho punto ochenta unidades cuadradas, coincidiendo con $1.2 \times 24$ con un error de precisión de máquina de $7.11 \times 10^{-15}$.  
> A la derecha probamos el escalamiento singular $S(1.0, 0.0)$, donde el determinante es cero. Todo el polígono y los dos segmentos paralelos colapsan sobre el eje $y = 0$: el área cae a cero, $L'_1$ y $L'_2$ se funden sobre el mismo segmento y la transformación deja de ser invertible.  
> Con esto concluyo la parte matemática y experimental; ahora le doy el pase a mi compañero Lucian, quien presentará las preguntas de defensa, la implementación en C++ y la demostración del programa en vivo."

**[CAMBIO A DIAPOSITIVA 8 - ÚNICO CAMBIO DE EXPOSITOR: PASE A LUCIAN]**

---

### PARTE 2 - LUCIAN (Diapositivas 8 a 12 + Ejecución en Vivo | ~5 min 35 s)

#### [DIAPOSITIVA 8 - Casos engañosos, limitaciones de rasterización y defensa técnica | ~55 segundos]
> **Lucian:**  
> "Gracias, Ronal. En esta lámina respondemos de manera directa las tres preguntas clave de análisis crítico y documentamos nuestra limitación de discretización.  
> Primero: ¿puede una transformación conservar el área y alterar los ángulos? Sí, ocurre siempre que $|\det(A)| = 1$ sin que la matriz lineal sea ortogonal, como vimos en $H_x(0.7)$ donde el área sigue en veinticuatro mientras el ángulo en $V_0$ cae a cincuenta y cinco punto cero un grados.  
> Segundo: ¿por qué un determinante negativo no implica área negativa en sentido físico? Porque el área física siempre es mayor o igual a cero y escala según $|\det(A)|$; el signo negativo únicamente indica inversión de orientación, de antihorario a horario.  
> Tercero: ¿por qué una matriz singular requiere tratamiento especial? Porque reduce la dimensión efectiva, elimina la inyectividad y causaría división por cero al invertir; por ello nuestro programa valida $|\det(A)| < 10^{-9}$ y bloquea cualquier inversión.  
> Además, reportamos con transparencia que, mientras las métricas se calculan en doble precisión con error de $10^{-15}$, la rasterización en rejilla $160 \times 120$ redondea cada vértice a la celda entera más próxima, generando un desplazamiento puramente visual de hasta media celda, es decir cero punto ciento veinticinco unidades. Veamos ahora cómo está estructurada nuestra implementación en C++."

**[CAMBIO A DIAPOSITIVA 9]**

#### [DIAPOSITIVA 9 - Implementación en C++: flujo matricial y rasterización `160x120` | ~50 segundos]
> **Lucian:**  
> "Aquí resumimos la estructura del programa y qué partes fueron reutilizadas frente a lo implementado por el equipo.  
> Del entorno de laboratorio del curso reutilizamos las primitivas base de rasterización: `putPixel()` y `putPoint()` para cada celda discreta, Bresenham en los ocho octantes, `drawPolygon()` y la rejilla visible `drawGrid()`.  
> Todo el motor geométrico fue codificado desde cero por nosotros sin delegar cálculos a `glTranslatef`, `glRotatef`, `glScalef` ni bibliotecas externas: las seis transformaciones afines en matrices $3\times 3$, la composición en orden de aplicación, la guarda de singularidad, la fórmula del zapatero, el indicador de paralelismo y la batería automatizada de pruebas.  
> Separar las métricas continuas de la rasterización sobre nuestra rejilla de $160 \times 120$ a cuatro píxeles por unidad permite mantener nitidez visual y exactitud numérica. Pasemos ahora a la demostración funcional en vivo."

**[CAMBIO A DIAPOSITIVA 10]**

#### [DIAPOSITIVA 10 + EJECUCIÓN EN VIVO DEL PROGRAMA C++ | ~2 min 55 s]
> **Lucian (introducción rápida sobre la Slide 10 - ~15 segundos):**  
> "En pantalla quedan resumidos los modos y controles interactivos del programa. Voy a cambiar a la terminal de WSL2 para ejecutar primero las pruebas automatizadas y luego abrir el laboratorio gráfico."

**[CAMBIAR A VENTANA WSL2 + OPENGL]**

> **Lucian (narración mientras ejecuta el programa en vivo - ~2 min 40 s):**
> 1. *(Ejecuta `./build/test_experiments ../experimentos` en terminal)*:  
>    "Ejecutamos primero las validaciones automáticas sin ventana: todas las pruebas matemáticas y de singularidad pasan con error menor a $10^{-9}$."
> 2. *(Ejecuta `./build/lab_afines`, inicia en modo `0` - Base)*:  
>    "Abrimos ahora el laboratorio gráfico. A la izquierda tenemos la ventana de $1280 \times 960$ sobre la rejilla discreta de $160 \times 120$, y a la derecha el panel limpio en consola organizado en tres bloques: transformaciones elementales, composiciones y controles en vivo. En el modo cero vemos la escena original: el polígono cóncavo de seis vértices con área más veinticuatro, la cuadrícula y los segmentos paralelos $L_1$ y $L_2$."
> 3. *(Demuestra las 6 transformaciones elementales en vivo)*:  
>    - *(Pulsa `T` y mueve con `Flechas`)*: "Con `T` aplicamos **traslación** pura y vemos variar la tercera columna de la matriz conservando área, ángulos y paralelismo."
>    - *(Pulsa `9` y gira con `Q`/`E`)*: "Con `9` aplicamos **rotación** pura, que mantiene intactos área, orientación y longitudes."
>    - *(Pulsa `K` y ajusta con `Z`/`X` y `W`/`S`)*: "Con `K` aplicamos **escalamiento** puro, donde el área escala exactamente por $s_x \cdot s_y$."
>    - *(Pulsa `1` y varía con `A`/`D`, luego vuelve a `0.7`; pulsa `2` y varía con `J`/`L`)*: "Con `1` vemos **cizallamiento horizontal** $H_x(0.7)$: el seno de paralelismo sigue en cero y el área en veinticuatro, pero el ángulo en $V_0$ cae a cincuenta y cinco punto cero un grados y la arista $V_5 V_0$ sube a seis punto diez. Con `2` y las teclas `J` y `L` aplicamos **cizallamiento vertical** $H_y$ en tiempo real."
>    - *(Pulsa `3`, alterna con `F` a $\mathrm{Ref}_x$, luego pulsa `4`)*: "Con `3` aplicamos **reflexión** en el eje vertical, pasando el área con signo a menos veinticuatro con orientación horaria; con `F` alternamos al eje horizontal, y con `4` mostramos la reflexión sobre la recta oblicua de treinta grados."
> 4. *(Pulsa `5`, rota vértice con `V` y vuelve a `V4`)*:  
>    "Con `5` superponemos $R(45^\circ)H_x(0.7)$ en azul y $H_x(0.7)R(45^\circ)$ en naranja, comprobando en vivo la brecha de $4.2579$ unidades en $V_4$; con `V` podemos rotar el vértice seleccionado para inspeccionar cualquier vértice."
> 5. *(Pulsa `6`)*:  
>    "Con `6` aplicamos la composición de cuatro pasos con determinante uno punto veinte y confirmamos en consola el área de veintiocho punto ochenta."
> 6. *(Pulsa `7`, luego `G` para alternar la rejilla)*:  
>    "Con `7` observamos el caso singular $S(1, 0)$: la figura y los segmentos colapsan sobre $y = 0$, el ángulo en $V_0$ pasa a indefinido y la consola advierte que no existe inversa. Con `G` podemos activar o desactivar la rejilla discreta de píxeles."
> 7. *(Pulsa `8`, ajusta `A`/`D` y `J`/`L`, baja `S` hasta `0.0` y pulsa `O`)*:  
>    "Finalmente, en el modo `8` combinamos todas las transformaciones en vivo, incluyendo cizallamiento en $X$ y en $Y$; podemos llevar $s_y$ a cero en tiempo real para provocar el colapso singular y pulsar `O` para invertir el orden de la cadena. Si el profesor desea probar otra secuencia arbitraria de tres o más pasos, la tecla `C` abre el constructor en consola. Volvamos a la presentación para cerrar."

**[VOLVER A LA PRESENTACIÓN - CAMBIO A DIAPOSITIVA 11]**

#### [DIAPOSITIVA 11 - Conclusiones | ~35 segundos]
> **Lucian:**  
> "Para cerrar, resumimos cuatro conclusiones técnicas:  
> Primero, las matrices homogéneas $3\times 3$ unifican todas las transformaciones afines en un modelo algebraico verificable a precisión de máquina.  
> Segundo, toda transformación afín invertible preserva paralelismo, colinealidad y razones de segmentos, pero deforma ángulos y longitudes fuera de las isometrías, y su composición depende estrictamente del orden.  
> Tercero, el determinante controla simultáneamente el factor de área, la orientación y la invertibilidad, exigiendo bloquear la inversión cuando es nulo.  
> Y cuarto, separar el cálculo continuo en doble precisión de la rasterización en rejilla $160 \times 120$ asegura plena coherencia entre los gráficos y las métricas."

**[CAMBIO A DIAPOSITIVA 12]**

#### [DIAPOSITIVA 12 - Fuentes bibliográficas y cierre | ~20 segundos]
> **Lucian:**  
> "Todo el sustento teórico, las derivaciones matriciales y los criterios geométricos se apoyan en la bibliografía básica de computación gráfica y matemáticas para gráficos del curso, que se muestra en formato APA en pantalla y en nuestro informe.  
> Muchas gracias por su atención; quedamos atentos a sus preguntas."

---

## Notas rápidas para defensa

- **Si piden la versión en archivo único**: compilar y ejecutar desde `src_unico/` (`cmake -S . -B build && cmake --build build -j`, luego `./build/lab_afines_unico --self-test` o `./build/lab_afines_unico`).
- **Si piden una cadena nueva de $N \ge 3$ pasos en vivo**: pulsar **`C`**, ingresar `3` (o `4`) y seleccionar cualquier combinación de `[1:T  2:R  3:S  4:Hx  5:Hy  6:Ref_x  7:Ref_y  8:Ref_phi]`.
