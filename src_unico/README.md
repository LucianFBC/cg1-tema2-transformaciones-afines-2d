# Laboratorio de Transformaciones Afines 2D (Version Archivo Unico)

UNSAAC - Computacion Grafica I (IF455AIN) - Semestre 2026-02
Tema 2: Transformaciones afines, deformacion y propiedades geometricas.

Implementacion autocontenida en un unico archivo fuente (`main.cpp`) en C++17 con OpenGL 2.1, GLFW 3 y rasterizacion propia sobre rejilla 160x120 (4 px/unidad, ventana 1280x960, `putPixel()` / `putPoint()`, Bresenham y `drawPolygon()`, margen 0.06 y `drawGrid()`).

## Compilacion y autoprueba (WSL2 Ubuntu)

```bash
rm -rf build
cmake -S . -B build
cmake --build build -j
./build/lab_afines_unico --self-test
```

## Ejecucion interactiva y exportacion

```bash
./build/lab_afines_unico
./build/lab_afines_unico --export-ppm ../experimentos/ppm
```

## Controles interactivos

- `0`: Escena base (poligono asimetrico de 6 vertices con area +24.0, segmentos paralelos `L1` y `L2`, cuadricula local)
- `1`: Cizallamiento horizontal `Hx(kx)`
- `2`: Cizallamiento vertical `Hy(ky)`
- `3`: Reflexion respecto al eje vertical u horizontal (`Ref_y`/`Ref_x` con `F`)
- `4`: Reflexion respecto a recta oblicua por el origen `Ref(phi) = R(phi)*Ref_x*R(-phi)`
- `T` / `9` / `K`: Traslacion pura `T(tx,ty)`, Rotacion pura `R(ang)` y Escalamiento puro `S(sx,sy)`
- `5`: Comparacion simultanea de orden `R(ang)*Hx(kx)` (azul) vs `Hx(kx)*R(ang)` (naranja)
- `6`: Composicion de 4 pasos con cambio de area `T*R*S*Hx`
- `7`: Caso degenerado / matriz singular `S(sx, 0)`
- `8`: Modo interactivo libre (`Hx -> S -> Ref_y? -> R -> T`)
- `C`: Constructor por consola de cadena personalizada (`N >= 3` transformaciones)
- `+` / `-`: Ajusta el parametro principal del modo activo
- `A`/`D` (`kx`), `J`/`L` (`ky`), `Z`/`X` (`sx`), `W`/`S` (`sy`), `Q`/`E` (`angulo` o `phi`), `Flechas` (`tx`, `ty`)
- `F`: Activar/desactivar `Ref_y` en modo libre
- `O`: Invertir el orden de composicion en modo libre
- `G`: Mostrar u ocultar rejilla de pixeles `drawGrid()`
- `M`: Mostrar u ocultar cuadricula local transformada
- `H`: Mostrar u ocultar figura original de referencia
- `V`: Rotar el vertice de referencia seleccionado (`V0`..`V5`)
- `R`: Restablecer parametros del modo activo
- `ESC`: Salir
