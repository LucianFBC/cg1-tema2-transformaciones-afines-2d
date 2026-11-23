---
name: wsl-cpp-opengl
description: >-
  Compila, ejecuta y valida el laboratorio C++17 + OpenGL 2.1 + GLFW3 del Tema 2
  dentro de WSL2 (Ubuntu), reutilizando las rutinas de rasterización del curso
  (putPoint, Bresenham, drawPolygon) y verificando compilación limpia y pruebas numéricas.
---

# Skill: Desarrollo y Compilación C++ / OpenGL en WSL2

## Entorno verificado de la máquina

- **Distro WSL**: `Ubuntu` (WSL2, Ubuntu 24.04) con `g++ 13.3.0`, `cmake 3.28.3`, `OpenGL` y `GLFW 3` (`find_package(glfw3)`).
- **Display WSLg**: `DISPLAY=:0` y `WAYLAND_DISPLAY=wayland-0` activos (las ventanas GLFW se abren directamente en Windows).
- **Ruta WSL del workspace**: `/mnt/e/Lucian/CG1/Unidad II/Expo` (citar siempre con comillas por el espacio en `Unidad II`).
- **Nota**: `pkg-config` no está instalado en Ubuntu, pero `find_package(glfw3)` funciona directamente en CMake (igual que en `E:\Lucian\CG1\Examen_02\CMakeLists.txt`).

## Estructura recomendada de `src/`

Separar la lógica matemática del renderizado facilita la claridad, la exposición y las pruebas automatizadas sin ventana gráfica:

```text
src/
├── CMakeLists.txt
├── README.md
├── math_affine.hpp        # Point, Matrix3, T, R, S, Hx, Hy, Ref, multiply, det, area, paralelismo
├── raster.hpp             # putPixel, putPoint, drawLine (Bresenham), drawPolygon, cuadrícula/ejes
├── experiments.hpp        # Escena base (polígono >=5 vértices, segmentos paralelos) y presets E1–E5
├── main.cpp               # Ventana GLFW, render comparativo, controles interactivos y salida en consola
└── test_experiments.cpp   # Ejecutable headless (sin OpenGL) para verificar numéricamente E1–E5 y exportar tablas
```

## Convenciones obligatorias del código

1. **Sin funciones automáticas de OpenGL**:
   - Solo usar `glOrtho` una vez en `framebufferSizeCallback` para definir el lienzo lógico de coordenadas (con `y` creciendo hacia arriba).
   - Todo dibujo de figuras, cuadrícula transformada, ejes y marcadores debe pasar por `putPoint()` / `drawLine()` (Bresenham).
   - Prohibido `glTranslatef`, `glRotatef`, `glScalef`, `glMultMatrix*`, `GL_POLYGON`, `GL_TRIANGLES`.
2. **Convención matricial**:
   - `Point { double x, y; }` tratado como vector columna `(x, y, 1)^T`.
   - `multiply(A, B)` aplica primero `B` y luego `A`.
   - Cuando el usuario encadena transformaciones $T_1, T_2, T_3$ en ese orden cronológico, la matriz compuesta es $M = T_3 \cdot T_2 \cdot T_1$ y debe imprimirse explícitamente tanto el orden de aplicación como el orden matricial.
3. **Coordenadas lógicas vs píxeles**:
   - Si se usa un espacio lógico centrado (p. ej. $[-20, 20] \times [-15, 15]$) mapeado a una rejilla discreta de celdas enteras para `putPoint()`, documentar explícitamente la correspondencia `(x_mundo, y_mundo) -> (col, fil)` y que las mediciones numéricas (área, vértices, paralelismo) se calculan en doble precisión *antes* del redondeo de rasterización.

## Comandos de compilación y prueba desde PowerShell

```powershell
# Configurar y compilar en WSL Ubuntu
wsl -d Ubuntu -- bash -lc 'cd "/mnt/e/Lucian/CG1/Unidad II/Expo/src" && cmake -S . -B build && cmake --build build -j'

# Ejecutar pruebas numéricas headless (sin abrir ventana)
wsl -d Ubuntu -- bash -lc 'cd "/mnt/e/Lucian/CG1/Unidad II/Expo/src" && ./build/test_experiments'

# Ejecutar aplicación gráfica interactiva (abre ventana GLFW vía WSLg)
wsl -d Ubuntu -- bash -lc 'cd "/mnt/e/Lucian/CG1/Unidad II/Expo/src" && ./build/lab_afines'
```

## Checklist de verificación antes de dar por listo el código

- [ ] Compila sin warnings críticos (`-Wall -Wextra`) en WSL Ubuntu desde cero (`rm -rf build && cmake -S . -B build && cmake --build build -j`).
- [ ] `test_experiments` pasa todas las aserciones de E1–E5 (tolerancia `1e-9`).
- [ ] La app interactiva permite activar E1–E5 con teclas directas, modificar parámetros en vivo, concatenar $\ge 3$ transformaciones y reiniciar el estado.
- [ ] Las matrices singulares ($\det(A)=0$) nunca dividen entre cero ni reportan inversa.
- [ ] `README.md` documenta dependencias, compilación, ejecución y todos los controles.
