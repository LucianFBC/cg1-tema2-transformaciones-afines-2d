---
name: presentacion-expo
description: >-
  Diseña y construye la presentación para la exposición oral y el guion de demostración
  en vivo del Tema 2 siguiendo las 6 orientaciones del docente, usando un flujo híbrido
  sin limitaciones visuales: base Canva MCP clonada del modelo institucional del equipo
  (DAHXvv2xQDU) + figuras explicativas de alta precisión generadas localmente.
---

# Skill: Presentación y Guion de Exposición en Vivo

## Flujo elegido (sin limitaciones visuales ni matemáticas)

- **Plataforma de presentación**: **Canva vía MCP (`canva_canva`)**, clonando la página 1 del diseño de referencia del equipo:
  - Diseño base: `DAHXvv2xQDU` (`https://www.canva.com/design/DAHXvv2xQDU/LoF1u-QHNOVEm7LdMNiesg/edit`), página 1 (`1920x1080`, fondo azul marino oscuro, escudo UNSAAC a la izquierda, logo EPIIS a la derecha, integrantes `CHOQUENAIRA GARCIA, Ronal Franklin - 174941` y `FERNANDEZ BACA CASTRO, Lucian Neptalí - 184197`).
  - **Corrección institucional en la carátula**: usar el nombre oficial de la facultad del sílabo: `FACULTAD DE INGENIERÍA ELÉCTRICA, ELECTRÓNICA, INFORMÁTICA Y MECÁNICA`.
- **Motor local de gráficos e ilustraciones explicativas** (`presentacion/figuras/`):
  - Generar localmente con **Python (`matplotlib` + `numpy` + `PIL`)** y/o **LaTeX TikZ** (convertido a PNG 300 DPI con fondo transparente/oscuro a juego) todas las imágenes técnicas que Canva no dibuja de forma nativa:
    - Grillas afines deformadas paso a paso (secuencias visuales para animar con *Emparejar y mover / Match & Move* en Canva o PowerPoint).
    - Diagramas de flujo de algoritmos (pipeline `Matrix3` $\to$ `multiply` $\to$ `transformPoint` $\to$ `Bresenham`/`putPoint`, verificación de singularidad $\det(A)=0$).
    - Derivaciones matriciales y composiciones $3\times 3$ con tipografía matemática nítida.
    - Capturas reales de la aplicación C++ OpenGL para E1–E5.
  - Subir cada figura a Canva mediante `create-upload-url` e insertarla con `edit-design` (`insert_fill`), manteniendo consistencia visual total.
  - Al finalizar, el equipo puede activar transiciones nativas en Canva (o exportar a `.pptx` / `.pdf` desde Canva con `export-design`).

## Estructura obligatoria según las 6 orientaciones del docente (pág. 8)

1. **Carátula institucional** (clonada de `DAHXvv2xQDU` pág. 1).
2. **Orientación 1 — Planteamiento del problema**: Mostrar con un ejemplo visual por qué juzgar deformaciones solo por apariencia engaña (p. ej. cizallamiento que parece agrandar la figura pero conserva el área exacta).
3. **Orientación 2 — Convención y derivación matemática clave**:
   - Vectores columna $p' = Mp$, orden $M = ABC$ ($C \to B \to A$), orientación del eje vertical.
   - Derivación visual y algebraica de $\mathrm{Ref}_\phi = R(\phi)\,\mathrm{Ref}_x\,R(-\phi)$ y de la no conmutatividad $R(45^\circ)H_x(0.7) \neq H_x(0.7)R(45^\circ)$.
   - Papel de $\det(A)$ y tabla visual de invariantes afines vs propiedades no conservadas.
4. **Orientación 3 — Arquitectura del código C++ y flujo algorítmico**:
   - Diagrama de módulos (`math_affine.hpp`, `raster.hpp`, `experiments.hpp`, `main.cpp`).
   - Qué funciones implementó el equipo y qué base de rasterización del curso se reutilizó (`putPoint`, Bresenham).
   - Flujo de control para matrices singulares ($\det(A)=0$).
5. **Orientación 4 — Experimentos E1–E5 + contraste manual**:
   - Capturas reales + secuencias comparativas (original vs transformada) + tabla numérica verificada.
   - Contraste paso a paso de un cálculo manual (p. ej., vértice elegido en E3 y cambio de área en E4).
6. **Orientación 5 — Caso límite e interpretación engañosa**:
   - E5 (matriz singular con factor de escala $0$): colapso dimensional, área $0$, pérdida de inyectividad e invertibilidad.
   - Paradoja intuitiva: área constante con ángulos deformados ($H_x(0.7)$) y signo de $\det(A)$ vs área física.
7. **Orientación 6 — Conclusiones técnicas y fuentes bibliográficas verificables**.

## Guion para la demostración en vivo (Entregable 4)

1. Abrir el laboratorio en WSL2/WSLg mostrando escena base (polígono asimétrico + cuadrícula + 2 segmentos paralelos + métricas en consola).
2. Recorrer presets **E1 $\to$ E2 $\to$ E3 $\to$ E4 $\to$ E5** conectando los números en consola con la geometría en pantalla.
3. Variar parámetros en vivo y concatenar 3 transformaciones nuevas para probar interactividad real.
4. Responder las 3 preguntas de defensa y cerrar indicando los aportes de cada integrante.
