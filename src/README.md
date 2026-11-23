# Laboratorio Interactivo de Transformaciones Afines 2D (Tema 2)

**Universidad Nacional de San Antonio Abad del Cusco (UNSAAC)**  
Escuela Profesional de Ingeniería Informática y de Sistemas  
**Asignatura**: Computación Gráfica I (`IF455AIN`) · Semestre `2026-02`  
**Docente**: PhD. Carlos Fernando Montoya Cubas  
**Integrantes**:
- CHOQUENAIRA GARCIA, Ronal Franklin (`174941`)
- FERNANDEZ BACA CASTRO, Lucian Neptalí (`184197`)

---

## Convención matemática y sistema de coordenadas

- **Vectores columna homogéneos**: $p = (x, y, 1)^T$, transformados mediante $p' = M p$.
- **Composición de matrices**: en $M = A B C$, se aplica primero $C$, luego $B$ y finalmente $A$.
- **Eje vertical**: la proyección ortográfica `glOrtho(0, 280, 0, 210, -1, 1)` define el **eje $Y$ creciendo hacia arriba**, coincidiendo con el plano cartesiano matemático. Al exportar imágenes raster `.ppm` (cuya fila `0` corresponde al borde superior), se aplica explícitamente la inversión $y_{\text{ppm}} = (H - 1) - y_{\text{raster}}$.
- **Rasterización propia**: todo el dibujo pasa por `putPixel()` / `putPoint()`, `drawLine()` (Bresenham en todos los octantes) y `drawPolygon()`. No se emplean `glTranslatef`, `glRotatef`, `glScalef`, `glMultMatrix*`, `GL_POLYGON` ni bibliotecas externas de transformación.
- **Matrices singulares**: si $|\det(A)| < 10^{-9}$, el programa reporta explícitamente la singularidad y bloquea el cálculo de inversa.

---

## Estructura de archivos

- `math_affine.hpp`: `Matrix3`, matrices $T, R, S, H_x, H_y, \mathrm{Ref}_x, \mathrm{Ref}_y, \mathrm{Ref}_\phi$, multiplicación $3\times 3$, $\det(A)$, inversa segura, área con signo/absoluta (zapatero) y medida de paralelismo.
- `raster.hpp`: `RasterCanvas`, `putPixel()`, `putPoint()`, `drawLine()` (Bresenham), `drawPolygon()`, ejes cartesianos, cuadrícula transformable y exportador `.ppm`.
- `experiments.hpp`: escena base (polígono asimétrico de 6 vértices, área $+10.0\text{ u}^2$, cuadrícula y dos segmentos paralelos $L_1, L_2$) y presets `E0`–`E5` + `REF30`.
- `main.cpp`: aplicación gráfica interactiva GLFW + OpenGL 2.1.
- `test_experiments.cpp`: suite de verificación numérica headless (sin ventana gráfica) que valida E1–E5 y exporta resultados reproducibles.

---

## Compilación y ejecución (Linux / WSL2 Ubuntu)

### Requisitos

```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev
```

### Compilar

```bash
cmake -S . -B build
cmake --build build -j
```

### Ejecutar pruebas numéricas (E1–E5)

```bash
./build/test_experiments ../experimentos
```

### Ejecutar laboratorio interactivo

```bash
./build/lab_afines
```

### Exportar capturas exactas del rasterizador (modo headless)

```bash
./build/lab_afines --export-ppm ../experimentos/capturas_ppm
```

---

## Controles de teclado en la aplicación interactiva

| Tecla | Acción |
| :--- | :--- |
| `0` | Escena base (Identidad) |
| `1` | **E1**: Cizallamiento horizontal $H_x(0.7)$ |
| `2` | **E2**: Reflexión respecto al eje vertical $\mathrm{Ref}_y$ |
| `3` | **E3a**: $R(45^\circ)\,H_x(0.7)$ |
| `4` | **E3b**: $H_x(0.7)\,R(45^\circ)$ |
| `5` | **E3 comparativo**: superpone ambos órdenes (azul vs. naranja) |
| `6` | **E4**: Composición de 4 transformaciones con $|\det(A)| = 1.2$ |
| `7` | **E5**: Caso degenerado singular $S(1.0, 0.0)$ ($\det(A) = 0$) |
| `8` | Reflexión sobre recta oblicua $\phi = 30^\circ$: $R(30^\circ)\,\mathrm{Ref}_x\,R(-30^\circ)$ |
| `I` | Activar modo interactivo en vivo (cadena $\ge 3$ transformaciones) |
| `A` / `D` | Disminuir / aumentar cizallamiento horizontal $k_x$ ($\pm 0.1$) |
| `J` / `L` | Disminuir / aumentar cizallamiento vertical $k_y$ ($\pm 0.1$) |
| `Z` / `X` | Disminuir / aumentar escala $s_x$ ($\pm 0.1$) |
| `S` / `W` | Disminuir / aumentar escala $s_y$ ($\pm 0.1$, permite llegar a $0.0$) |
| `Q` / `E` | Rotar $+5^\circ$ / $-5^\circ$ |
| `← ↑ ↓ →` | Modificar traslación $(t_x, t_y)$ ($\pm 0.5$) |
| `F` | Alternar reflexión $\mathrm{Ref}_y$ en la cadena interactiva |
| `O` | Invertir orden de aplicación de la cadena interactiva |
| `C` | Constructor por consola para concatenar $N \ge 3$ transformaciones exactas |
| `G` / `H` | Mostrar u ocultar cuadrícula / figura original |
| `P` / `R` | Imprimir reporte completo en consola / reiniciar parámetros |
| `ESC` | Cerrar ventana |
