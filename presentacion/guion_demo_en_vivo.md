# Guion de Exposición y Demostración en Vivo

**Asignatura**: Computación Gráfica I (`IF455AIN`) - UNSAAC `2026-02`  
**Docente**: PhD. Carlos Fernando Montoya Cubas  
**Equipo**:
- CHOQUENAIRA GARCIA, Ronal Franklin (`174941`)
- FERNANDEZ BACA CASTRO, Lucian Neptalí (`184197`)

**Presentación en Canva (12 diapositivas)**: https://www.canva.com/design/DAHbsW56mtM/edit  
*(Respaldos locales: `presentacion/Presentacion_Tema2.pdf` y `presentacion/Presentacion_Tema2.pptx`)*

---

## Preparación previa (1 minuto antes)

1. Abrir una terminal en WSL2 (`Ubuntu`) dentro de `src/` (o `src_unico/` si se solicita archivo único) y confirmar compilación:
   ```bash
   cmake -S . -B build && cmake --build build -j
   ./build/test_experiments ../experimentos
   ```
   *(En la versión de archivo único `src_unico/`: `./build/lab_afines_unico --self-test`)*
2. Colocar la ventana gráfica `1000x750` (rejilla `40x30`) a la izquierda y la terminal a la derecha. Gracias a la limpieza ANSI automática, cada tecla refresca en el mismo lugar el menú de opciones y la matriz $3\times 3$ sin acumular texto.

---

## Guion cronometrado (10 a 12 minutos)

| Tiempo | Diapositiva / Acción | Qué explicar o demostrar |
| :--- | :--- | :--- |
| `0:00 - 0:45` | **Slides 1 y 2** (Carátula y problema) | Presentar el tema y mostrar con el cuadrado $4\times 4$ bajo $H_x(0.7)$ por qué la inspección visual engaña (el ángulo baja a $55.01^\circ$, pero el área es exactamente $16.0\text{ u}^2$). |
| `0:45 - 2:30` | **Slides 3 y 4** (Convención, $\mathrm{Ref}_\phi$ y $\det(A)$) | Explicar vectores columna $p'=Mp$, orden $M=ABC$, eje $+Y$ hacia arriba (`glOrtho`), derivación en 3 pasos de $\mathrm{Ref}_\phi = R(\phi)\,\mathrm{Ref}_x\,R(-\phi)$ y los tres regímenes de $\det(A)$ ($>0$, $<0$, $=0$). |
| `2:30 - 3:30` | **Slide 5** (Implementación y flujo `40x30`) | Mostrar el flujo de cálculo matricial propio en `double`, control de singularidad ($|\det(A)| < 10^{-9}$) y rasterización exclusiva con `putPixel()`/`putPoint()`, Bresenham y `drawPolygon()` sobre rejilla `40x30` (sin funciones automáticas de OpenGL). |
| `3:30 - 5:30` | **Slides 6, 7 y 8** (Pruebas cuantitativas y cálculo manual) | Presentar cizallamiento $H_x(0.7)$, reflexión $\mathrm{Ref}_y$, **cálculo manual de $V_4=(5,7)$** en $R(45^\circ)H_x(0.7)$ vs. $H_x(0.7)R(45^\circ)$ ($\|\Delta V_4\|=4.2579\text{ u}$), composición de 4 pasos ($|\det|=1.20$, área $24.00 \to 28.80\text{ u}^2$) y caso singular $S(1,0)$. |
| `5:30 - 9:00` | **Slide 10 - DEMO EN VIVO (`./build/lab_afines`)** | Recorrer en vivo: `0` (base, área $+24$) -> `1` ($H_x(0.7)$, señalar $u'=v'=(10.8, 4.0)$, $\sigma=0$, $\angle V_0=55.01^\circ$) -> `3` ($\mathrm{Ref}_y$, área signo $-24$, $|A|=24$) -> `4` ($\mathrm{Ref}_{30^\circ}$) -> `5` (orden superpuesto, $\|\Delta V_4\|=4.2579$) -> `6` (4 pasos, área $28.80$) -> `7` (singular $S(1,0)$, alerta sin inversa) -> `G` (mostrar/ocultar rejilla de píxeles) -> `8` (variar parámetros en vivo con `A/D`, `Q/E`, bajar `S` hasta $s_y=0$ y pulsar `O` para invertir el orden). Si el docente pide una cadena propia, pulsar `C`. |
| `9:00 - 11:00` | **Slides 9, 11 y 12** (Defensa, conclusiones y fuentes) | Responder las tres preguntas técnicas clave, comentar la limitación de redondeo visual ($\pm 0.5$ celdas en `40x30` frente a precisión `double`), presentar las 4 conclusiones claras y cerrar con la bibliografía oficial. |
