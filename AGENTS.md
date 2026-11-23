# Reglas del Proyecto — Computación Gráfica I (Segunda Parcial · Tema 2)

## Contexto del curso, equipo y entregable

- **Universidad**: Universidad Nacional de San Antonio Abad del Cusco (UNSAAC).
- **Facultad**: Facultad de Ingeniería Eléctrica, Electrónica, Informática y Mecánica (FIEEIM).
- **Escuela Profesional**: Ingeniería Informática y de Sistemas (EPIIS).
- **Asignatura**: Computación Gráfica I (`IF455AIN`) · Semestre `2026-02` · Docente: PhD. Carlos Fernando Montoya Cubas.
- **Integrantes del equipo**:
  - `CHOQUENAIRA GARCIA, Ronal Franklin` — Código: `174941`
  - `FERNANDEZ BACA CASTRO, Lucian Neptalí` — Código: `184197`
- **Trabajo**: Investigación y aplicación de la Segunda Parcial — **Tema 2: Transformaciones afines, deformación y propiedades geométricas**.
- **Objetivo específico**: Implementar un **laboratorio interactivo de transformaciones afines 2D** en C++ sobre un polígono asimétrico ($\ge 5$ vértices), una cuadrícula y dos segmentos originalmente paralelos, verificando visual y cuantitativamente paralelismo, colinealidad, área, orientación, distancias y casos degenerados bajo diferentes matrices y composiciones.
- **Materiales oficiales en el workspace**:
  - `Materiales/Trabajos_Segunda_Parcial_Computacion_Grafica.pdf` (condiciones comunes en pág. 1, Tema 2 en págs. 4–5, rúbrica de 20 pts, orientaciones de exposición y lista de comprobación en págs. 8–9).
  - `Materiales/Silabo_Computacion_Grafica_UNSAAC_2026-02 (1).pdf` (contenidos de Unidad II y bibliografía oficial).
  - `Materiales/Plan_Sesiones_Computacion_Grafica_2026-02 (1).pdf` (cronograma del semestre).
- **Referencias técnicas y visuales**:
  - Código base del curso: `E:\Lucian\CG1\Examen_02\Examen_02_Solucion` (OpenGL 2.1 + GLFW3 + CMake + `putPoint()` + Bresenham).
  - Modelo de carátula del informe LaTeX: inspirado en `E:\Lucian\Tesis\01_PLAN_TESIS\05_documento\partes\01caratula.tex` (logo institucional copiado en `informe/figuras/institucional/logo.png`).
  - Modelo de carátula de presentación: Canva design `DAHXvv2xQDU` (`https://www.canva.com/design/DAHXvv2xQDU/LoF1u-QHNOVEm7LdMNiesg/edit`, página 1).

---

## Requisitos NO negociables ("sí o sí") del enunciado

### A. Reglas técnicas obligatorias (C++ y OpenGL)

1. **Entorno y compilación**:
   - Lenguaje **C++17**, compilado con **CMake + g++** en **WSL2 (`Ubuntu`)** con `OpenGL` y `GLFW 3` (WSLg habilitado para ventanas gráficas).
   - El código debe compilar limpiamente desde las instrucciones del `README.md`, sin depender de rutas absolutas locales ni archivos ocultos.
2. **Rasterización propia obligatoria**:
   - Reutilizar el entorno de rasterización del curso: `putPixel()` / `putPoint()`, `drawLine()` (Bresenham en todos los octantes) y `drawPolygon()`.
   - **Prohibido** delegar transformaciones o primitivas evaluadas a OpenGL o librerías externas: NO usar `glTranslatef`, `glRotatef`, `glScalef`, `glMultMatrix*`, `GL_POLYGON`, `GL_TRIANGLES`, GLM ni motores de escena para los cálculos geométricos del tema.
3. **Convención matemática obligatoria**:
   - Vectores columna homogéneos: $p = (x, y, 1)^T$, transformados mediante $p' = M p$.
   - Concatenación: en $M = A B C$, se aplica primero $C$, luego $B$ y finalmente $A$.
   - Matrices homogéneas $3\times 3$ implementadas explícitamente (`Matrix3`): identidad, traslación $T(t_x, t_y)$, rotación $R(\theta)$, escalamiento $S(s_x, s_y)$, cizallamiento horizontal $H_x(k)$, cizallamiento vertical $H_y(k)$, reflexión (respecto a ejes X, Y y recta por el origen con ángulo $\phi$ mediante $R(\phi)\,\mathrm{Ref}_x\,R(-\phi)$), multiplicación $3\times 3$ y aplicación a puntos.
   - **Sistema de coordenadas**: Documentar explícitamente la orientación del eje vertical. Con `glOrtho(xMin, xMax, yMin, yMax, -1, 1)` el eje $y$ crece hacia arriba (coherente con el plano cartesiano matemático); si en algún componente o captura se usa pantalla con $y$ hacia abajo, indicar explícitamente cómo se corrige.
4. **Manejo estricto de matrices singulares**:
   - Calcular explícitamente $\det(A)$ de la submatriz lineal $2\times 2$.
   - Nunca calcular ni afirmar existencia de inversa cuando $|\det(A)| < \varepsilon$ (p. ej. $\varepsilon = 10^{-9}$). Mostrar advertencia clara de transformación singular/degenerada y analizar su efecto (colapso de área, pérdida de inyectividad e invertibilidad).

### B. Investigación y formalización obligatoria (Sección 2.3)

1. Definir transformación afín $p' = Ap + t$ ($A \in \mathbb{R}^{2\times 2}$) y escribirla en forma de matriz homogénea $3\times 3$.
2. Derivar las matrices de cizallamiento horizontal $H_x(k)$ y vertical $H_y(k)$.
3. Investigar reflexiones respecto de los ejes coordenados y de una recta por el origen con ángulo $\phi$, justificando $\mathrm{Ref}_\phi = R(\phi)\,\mathrm{Ref}_x\,R(-\phi)$.
4. Explicar el papel de $\det(A)$: factor de cambio de área $|\det(A)|$, inversión de orientación si $\det(A) < 0$ y pérdida de invertibilidad si $\det(A) = 0$.
5. Establecer con precisión propiedades conservadas bajo transformación afín invertible (rectas, colinealidad, paralelismo, razones de segmentos colineales) y explicar con demostración o contraejemplo por qué longitud y ángulos no se conservan en general.

### C. Requisitos funcionales mínimos de implementación (Sección 2.4)

1. Dibujar un **polígono asimétrico de al menos 5 vértices**, una **cuadrícula transformable**, **dos segmentos originalmente paralelos** y referencias suficientes (ejes coordenados).
2. Implementar con matrices propias: cizallamiento horizontal, cizallamiento vertical, reflexión, rotación, escalamiento y traslación.
3. Permitir **introducir o modificar parámetros** y **concatenar al menos 3 transformaciones**, mostrando el **orden aplicado** y la **matriz compuesta resultante $3\times 3$**.
4. Visualizar **figura original y transformada** con estilos/colores distinguibles, mostrando **coordenadas de vértices seleccionados**.
5. Calcular numéricamente en el programa: **área del polígono antes y después** (fórmula del zapatero, con signo y absoluta), **determinante de la parte lineal** $\det(A)$ y **medida de paralelismo** basada en vectores directores.
6. Contemplar **matrices singulares** ($\det(A)=0$) y evitar afirmar que existe inversa.

### D. Los 5 experimentos obligatorios (E1–E5) y 3 preguntas de defensa (Sección 2.5)

- **E1 (Cizallamiento)**: $H_x(0.7)$ — verificar con coordenadas la conservación del paralelismo de los segmentos seleccionados.
- **E2 (Reflexión)**: Reflexión respecto al eje vertical ($x \mapsto -x$) — comparar área con signo, área absoluta y orientación del recorrido de los vértices.
- **E3 (Orden)**: Comparar $R(45^\circ)H_x(0.7)$ vs $H_x(0.7)R(45^\circ)$ sobre la misma figura y cuantificar la diferencia en un vértice elegido.
- **E4 (Cambio de área)**: Transformación compuesta con $|\det(A)| \neq 1$ y contraste entre el área calculada y $|\det(A)|\cdot \text{Área}_{\text{original}}$.
- **E5 (Caso degenerado)**: Escalamiento con uno de sus factores igual a $0$ (p. ej. $S(1, 0)$ o $S(0, 1)$) — explicar qué propiedades pierden su interpretación y por qué la transformación no es invertible.
- **Preguntas de defensa obligatorias**:
  1. ¿Puede una transformación conservar el área y alterar los ángulos?
  2. ¿Por qué el determinante negativo no implica área negativa en sentido físico?
  3. ¿Por qué una matriz singular requiere un tratamiento especial?

---

## Rigor académico, decisiones de herramientas y entregables

- **Criterio de rigor**: Un resultado visual atractivo nunca reemplaza una implementación correcta ni una explicación matemática. Toda discrepancia o limitación (p. ej. redondeo a píxel en Bresenham) debe reportarse y explicarse explícitamente.
- **Bibliografía verificable**: Prohibido inventar referencias. Priorizar fuentes del sílabo:
  - Hearn, D., Baker, M. P., & Carithers, W. (2011). *Computer Graphics with OpenGL* (4th ed.). Pearson.
  - Foley, J. D., van Dam, A., Feiner, S. K., & Hughes, J. F. (2014). *Computer Graphics: Principles and Practice* (3rd ed.). Addison-Wesley.
  - Shirley, P., et al. (2021). *Fundamentals of Computer Graphics* (5th ed.). CRC Press.
  - Vince, J. (2012). *Mathematics for Computer Graphics*. Springer.
  - Angel, E., & Shreiner, D. (2015). *Interactive Computer Graphics: A Top-Down Approach with Shader-Based OpenGL*. Pearson.
- **Herramientas acordadas por entregable**:
  1. **Código fuente C++ + demo en vivo**: C++17 + CMake + OpenGL 2.1 + GLFW 3 en WSL2 (`Ubuntu`) bajo `src/`, con validación headless y control de versiones en Git.
  2. **Informe breve (5–8 páginas sin portada ni anexos)**: **LaTeX (MiKTeX `pdflatex` + TeXworks)** en `informe/`, usando el modelo de carátula institucional (`01caratula.tex` adaptado al curso).
  3. **Presentación para exposición (6 orientaciones de pág. 8)**: **Flujo híbrido sin límites visuales** — base en **Canva MCP** clonando la carátula de `DAHXvv2xQDU` (con ambos logos institucionales) enriquecida con figuras de alta precisión generadas localmente (Python `matplotlib`/`numpy`, TikZ y capturas reales de OpenGL para transformaciones, flujos de algoritmos y transiciones), exportable a PDF/PPTX.
  4. **Control de versiones**: Repositorio Git en la raíz del proyecto con `.gitignore` excluyendo artefactos de compilación (`build/`, auxiliares LaTeX).

---

## Skills del proyecto disponibles en `.agents/skills/`

- `wsl-cpp-opengl`: Compilación, ejecución y pruebas en WSL2 (`Ubuntu`) con CMake/g++/GLFW3.
- `experimentos-tema2`: Diseño, validación numérica (E1–E5) y preguntas de defensa del Tema 2.
- `informe-latex`: Redacción y compilación del informe técnico en LaTeX con MiKTeX (`pdflatex`) y carátula institucional UNSAAC.
- `presentacion-expo`: Diseño de diapositivas en Canva + figuras locales de alta precisión (flujos, matrices, transiciones) y guion de demo en vivo.
