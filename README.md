# Computación Gráfica I (IF455AIN) - Segunda Parcial

**Transformaciones afines, deformación y propiedades geométricas en 2D**  
Universidad Nacional de San Antonio Abad del Cusco (UNSAAC) - FIEEIM - EPIIS - Semestre `2026-02`  
**Docente**: PhD. Carlos Fernando Montoya Cubas  
**Integrantes**:
- CHOQUENAIRA GARCIA, Ronal Franklin (`174941`)
- FERNANDEZ BACA CASTRO, Lucian Neptalí (`184197`)

---

## Estructura del repositorio

```text
.
├── src/                        # Versión modular C++17 + OpenGL 2.1 + GLFW 3 + CMake
│   ├── CMakeLists.txt
│   ├── README.md               # Compilación, ejecución y tabla de controles
│   ├── math_affine.hpp         # Matrices homogéneas 3x3 propias, det(A), inversa segura, zapatero y paralelismo
│   ├── raster.hpp              # Rejilla 40x30, putPixel()/putPoint(), Bresenham, drawPolygon() y exportador PPM
│   ├── experiments.hpp         # Escena base (6 vértices, área +24.0 u^2, L1/L2, cuadrícula) y casos de prueba
│   ├── main.cpp                # Aplicación gráfica interactiva con panel limpio en consola
│   └── test_experiments.cpp    # Verificación numérica automatizada headless (error < 1e-9)
├── src_unico/                  # Versión autocontenida en UN SOLO ARCHIVO (estilo Examen_02)
│   ├── main.cpp                # Todas las matrices, rasterización 40x30, panel en vivo y --self-test en 1 archivo
│   ├── CMakeLists.txt
│   └── README.md
├── experimentos/               # Resultados verificados exportados desde C++, capturas 40x30 y script de figuras
│   ├── resultados_experimentos.json
│   ├── resumen_experimentos.txt
│   ├── capturas_ppm/
│   └── generar_figuras.py
├── informe/                    # Informe técnico en LaTeX (MiKTeX / TeXworks)
│   ├── main.tex
│   ├── main.pdf                # PDF compilado (portada UNSAAC + 8 páginas de cuerpo + 1 anexo de uso)
│   ├── referencias.bib
│   ├── partes/01caratula.tex
│   └── figuras/
├── presentacion/               # Presentación de 12 diapositivas y guion de demostración en vivo
│   ├── Presentacion_Tema2.pdf
│   ├── Presentacion_Tema2.pptx
│   ├── figuras/
│   └── guion_demo_en_vivo.md
└── Materiales/                 # Sílabo, plan de sesiones y enunciado oficial del curso
```

---

## Compilación y ejecución rápida (Linux / WSL2 Ubuntu)

### Opción A: Versión modular (`src/`)

```bash
cd src
cmake -S . -B build && cmake --build build -j
./build/test_experiments ../experimentos
./build/lab_afines
```

### Opción B: Versión en un único archivo (`src_unico/main.cpp`)

```bash
cd src_unico
cmake -S . -B build && cmake --build build -j
./build/lab_afines_unico --self-test
./build/lab_afines_unico
```

---

## Informe técnico y presentación

- **Informe PDF**: `informe/main.pdf` (compilable con `pdflatex` + `bibtex` desde `informe/main.tex`).
- **Presentación en Canva (12 diapositivas)**: https://www.canva.com/design/DAHbsW56mtM/edit (y respaldos offline `presentacion/Presentacion_Tema2.pdf` y `presentacion/Presentacion_Tema2.pptx`).
- **Guion de exposición y demo**: `presentacion/guion_demo_en_vivo.md`.
