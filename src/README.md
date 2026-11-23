# Laboratorio Interactivo de Transformaciones Afines 2D

**Universidad Nacional de San Antonio Abad del Cusco (UNSAAC)**  
Escuela Profesional de Ingeniería Informática y de Sistemas  
**Asignatura**: Computación Gráfica I (`IF455AIN`) - Semestre `2026-02`  
**Docente**: PhD. Carlos Fernando Montoya Cubas  
**Integrantes**:
- CHOQUENAIRA GARCIA, Ronal Franklin (`174941`)
- FERNANDEZ BACA CASTRO, Lucian Neptalí (`184197`)

---

## Convención matemática y rasterización

- **Vectores columna homogéneos**: $p = (x, y, 1)^T$, transformados mediante $p' = M p$.
- **Composición**: en $M = A B C$, actúa primero $C$, luego $B$ y al final $A$.
- **Rejilla y eje vertical**: lienzo discreto `160 x 120` con escala `4 px/unidad` (ventana `1280 x 960`, celdas de `8 x 8 px` con margen `0.06` para distinguir cada píxel). El origen cartesiano `(0, 0)` se ubica en la celda central `(80, 60)` y `glOrtho(0, 160, 0, 120, -1, 1)` mantiene el eje $+Y$ hacia arriba.
- **Rasterización propia**: todo trazado evaluado emplea `putPixel()`, `putPoint()`, `drawLine()` (Bresenham en todos los octantes) y `drawPolygon()`, sin usar `glTranslatef`, `glRotatef`, `glScalef`, `glMultMatrix*` ni `GL_POLYGON`.
- **Panel en consola**: cada cambio de modo o parámetro limpia la terminal y redibuja en el mismo lugar el menú de opciones junto con la matriz compuesta $3\times 3$ y las métricas actualizadas en tiempo real.

---

## Compilación y ejecución (Linux / WSL2 Ubuntu)

```bash
cmake -S . -B build
cmake --build build -j

# Verificación numérica reproducible (sin ventana)
./build/test_experiments ../experimentos

# Laboratorio gráfico interactivo
./build/lab_afines

# Exportación de capturas exactas 1280x960 con rejilla visible
./build/lab_afines --export-ppm ../experimentos/capturas_ppm
```

---

## Controles de teclado

| Tecla | Acción |
| :--- | :--- |
| `0` | Escena base (Identidad, polígono asimétrico de 6 vértices, área $+24.0\text{ u}^2$) |
| `1` / `2` | Cizallamiento horizontal $H_x(k_x)$ / cizallamiento vertical $H_y(k_y)$ |
| `3` / `4` | Reflexión eje vertical $\mathrm{Ref}_y$ / reflexión recta oblicua $\mathrm{Ref}_\phi = R(\phi)\mathrm{Ref}_x R(-\phi)$ |
| `5` | Comparación de orden: $R(\theta)H_x(k_x)$ (azul) frente a $H_x(k_x)R(\theta)$ (naranja) |
| `6` | Composición de 4 pasos con cambio de área $T\cdot R\cdot S\cdot H_x$ ($|\det(A)| = 1.20$) |
| `7` | Caso degenerado singular $S(s_x, 0)$ ($\det(A)=0$, sin inversa) |
| `8` | Modo compuesto interactivo libre ($H_x \to S \to \mathrm{Ref}_y? \to R \to T$) |
| `C` | Constructor por consola para encadenar $N \ge 3$ transformaciones exactas |
| `+` / `-` | Modificar el parámetro principal del modo activo en vivo |
| `A` / `D`, `J` / `L` | Ajustar cizallamiento horizontal $k_x$ y vertical $k_y$ ($\pm 0.1$) |
| `Z` / `X`, `S` / `W` | Ajustar escala $s_x$ y $s_y$ ($\pm 0.1$; permite llevar $s_y$ a $0.0$) |
| `Q` / `E`, `← ↑ ↓ →` | Ajustar rotación $\theta$ (o $\phi$ en modo `4`) y traslación $(t_x, t_y)$ |
| `F` / `O` | Alternar reflexión $\mathrm{Ref}_y$ o invertir el orden de la cadena en modo `8` |
| `G` | Mostrar u ocultar la rejilla de píxeles (`drawGrid`, igual que en el laboratorio) |
| `M` / `H` | Mostrar u ocultar la cuadrícula transformada / figura original |
| `V` | Rotar el vértice seleccionado de referencia (`V0` a `V5`, resaltado en la ventana y consola) |
| `R` / `ESC` | Reiniciar parámetros / salir |
