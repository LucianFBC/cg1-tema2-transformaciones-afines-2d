# Guion de Exposición y Demostración en Vivo — Tema 2

**Asignatura**: Computación Gráfica I (`IF455AIN`) · UNSAAC `2026-02`  
**Docente**: PhD. Carlos Fernando Montoya Cubas  
**Equipo**:
- **CHOQUENAIRA GARCIA, Ronal Franklin** (`174941`)
- **FERNANDEZ BACA CASTRO, Lucian Neptalí** (`184197`)

**Presentación en Canva**: `Presentacion_Tema2_Transformaciones_Afines_UNSAAC_2026_02` (Diseño `DAHbsW56mtM`, 11 diapositivas + notas de presentador).

---

## Preparación previa a la exposición (1 minuto antes)

1. Abrir una terminal en WSL2 (`Ubuntu`) dentro de `src/` y verificar compilación limpia:
   ```bash
   cmake -S . -B build && cmake --build build -j
   ./build/test_experiments ../experimentos
   ```
2. Dejar abierta la ventana de la presentación en Canva (o el PDF/PPTX exportado como respaldo offline) y la terminal WSL lista para lanzar `./build/lab_afines`.
3. Colocar la ventana gráfica de OpenGL a la izquierda y la terminal con los reportes numéricos a la derecha (pantalla dividida) para que el docente vea **simultáneamente gráfico y consola**.

---

## Guion cronometrado (sugerencia: 10–12 minutos)

| Tiempo | Diapositiva / Acción | Responsable sugerido | Qué explicar / demostrar |
| :--- | :--- | :--- | :--- |
| `0:00 – 0:45` | **Slide 1–2** (Carátula + Problema) | Ronal | Presentar el equipo y el Tema 2. Mostrar con el cuadrado $4\times 4$ bajo $H_x(0.7)$ por qué la apariencia engaña (los ángulos bajan de $90^\circ$ a $55.01^\circ$, pero el área es exactamente $16.0\text{ u}^2$). |
| `0:45 – 2:30` | **Slide 3–4** (Convención, $\mathrm{Ref}_\phi$, $\det(A)$ e invariantes) | Ronal | Explicar $p'=Mp$ (columna), $M=ABC$ ($C\to B\to A$), eje $+Y$ hacia arriba (`glOrtho`), derivación de $\mathrm{Ref}_\phi = R(\phi)\,\mathrm{Ref}_x\,R(-\phi)$ y los tres regímenes de $\det(A)$ ($>0$, $<0$, $=0$). |
| `2:30 – 3:45` | **Slide 5** (Arquitectura C++ y flujo) | Lucian | Mostrar la separación `math_affine.hpp` / `raster.hpp` / `experiments.hpp` / `main.cpp` / `test_experiments.cpp`. Recalcar reutilización de `putPoint()`, Bresenham y `drawPolygon()` sin `glTranslate`/`glRotate`/`glScale`/`GL_POLYGON`. |
| `3:45 – 5:30` | **Slide 6–8** (Resultados E1–E5 + cálculo manual $V_4$) | Ronal + Lucian | Mostrar las capturas reales y **contrastar a mano $V_4=(4,5)$ en E3** ($R(45^\circ)H_x(0.7)V_4 = (1.7678, 8.8388)$ vs. $H_x(0.7)R(45^\circ)V_4 = (3.7477, 6.3640)$, $\|\Delta V_4\|=3.1694$), E4 ($|\det|=1.2 \implies 12.00\text{ u}^2$) y E5 ($S(1,0)$ singular). |
| `5:30 – 9:00` | **Slide 10 — DEMO EN VIVO (`./build/lab_afines`)** | Lucian (opera) + Ronal (explica números) | Secuencia exacta: `0` (base, Área $+10$) $\to$ `1` (**E1**, señalar $u'=v'=(5.4,2.0)$, $\sigma=0$, $\angle V_0=55.01^\circ$) $\to$ `2` (**E2**, señalar $A'_{\text{signo}}=-10$, $|A'|=10$, CW) $\to$ `5` (**E3** superpuesto, señalar $\|\Delta V_4\|=3.1694$) $\to$ `6` (**E4**, 4 pasos, error $3.55\times 10^{-15}$) $\to$ `7` (**E5**, colapso a $y=0$, alerta singular) $\to$ `8` ($\mathrm{Ref}_{30^\circ}$) $\to$ `I` (modo interactivo: variar `A/D`, `Q/E`, bajar `S` hasta $s_y=0$ en vivo para ver el colapso y pulsar `O` para invertir el orden). Si el docente pide una cadena propia, pulsar `C`. |
| `9:00 – 10:30` | **Slide 9 y 11** (Defensa, limitaciones y cierre) | Ambos | Responder proactivamente las 3 preguntas de defensa, mencionar el redondeo visual de $\pm 0.05\text{ u}$ en Bresenham frente a `double`, cerrar con conclusiones, contribuciones y bibliografía del sílabo. |

---

## Respuestas rápidas para la ronda de preguntas del docente

1. **¿Puede una transformación conservar el área y alterar los ángulos?**  
   Sí: cualquier matriz con $|\det(A)|=1$ que no sea ortogonal ($A^TA\neq I$). Ejemplo ejecutado: E1 ($H_x(0.7)$) conserva área $=10\text{ u}^2$ y baja $\angle V_0$ de $90^\circ$ a $55.01^\circ$. Otro ejemplo: $S(2, 0.5)$.
2. **¿Por qué el determinante negativo no implica área negativa en sentido físico?**  
   El área física es siempre no negativa y escala por $|\det(A)|$. El signo de $\det(A)$ indica orientación (quiralidad): $\det(A)<0$ invierte el sentido de recorrido de antihorario (CCW) a horario (CW), como en E2 ($A'_{\text{signo}}=-10$, $|A'|=10$).
3. **¿Por qué una matriz singular requiere un tratamiento especial?**  
   Porque $\det(A)=0 \implies \mathrm{rango}(A)<2$: colapsa el plano en una recta o un punto, elimina la inyectividad e invertibilidad y hace que dividir por $\det(A)$ produzca `NaN`/`Inf`. Nuestro programa verifica $|\det(A)| < 10^{-9}$ antes de intentar invertir y bloquea la operación con una advertencia clara.
