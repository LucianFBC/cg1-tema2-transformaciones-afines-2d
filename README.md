# Computación Gráfica I (IF455AIN) — Segunda Parcial · Tema 2

**Transformaciones afines, deformación y propiedades geométricas en 2D**  
Universidad Nacional de San Antonio Abad del Cusco (UNSAAC) · FIEEIM · EPIIS · Semestre `2026-02`  
**Docente**: PhD. Carlos Fernando Montoya Cubas

## Integrantes del equipo

| Apellidos y Nombres | Código | Contribuciones principales |
| :--- | :--- | :--- |
| **CHOQUENAIRA GARCIA, Ronal Franklin** | `174941` | Formalización matemática (derivación de cizallamiento, reflexiones $\mathrm{Ref}_\phi$ e invariantes), validación de E1–E5, presentación y demostración en vivo. |
| **FERNANDEZ BACA CASTRO, Lucian Neptalí** | `184197` | Arquitectura e implementación C++17/OpenGL (`math_affine.hpp`, `raster.hpp`, `main.cpp`, `test_experiments.cpp`), automatización de evidencias, informe LaTeX y demostración en vivo. |

---

## Estructura del repositorio

```text
.
├── src/                        # Código fuente C++17 + OpenGL 2.1 + GLFW3 + CMake
│   ├── CMakeLists.txt
│   ├── README.md               # Instrucciones detalladas de compilación y controles
│   ├── math_affine.hpp         # Matrices homogéneas 3x3 propias, det(A), inversa segura, zapatero y paralelismo
│   ├── raster.hpp              # putPixel(), putPoint(), Bresenham (8 octantes), drawPolygon() y exportador PPM
│   ├── experiments.hpp         # Escena base (polígono asimétrico de 6 vértices, grilla, L1/L2) y presets E0–E5
│   ├── main.cpp                # Aplicación gráfica interactiva
│   └── test_experiments.cpp    # Suite de verificación numérica headless (error < 1e-9)
├── experimentos/               # Datos reproducibles exportados desde C++ y generador de figuras
│   ├── resultados_e1_e5.json
│   ├── resumen_experimentos.txt
│   ├── capturas_ppm/           # Salidas raster reales del programa C++
│   └── generar_figuras.py      # Genera las figuras anotadas para informe y presentación
├── informe/                    # Informe técnico breve en LaTeX (MiKTeX / TeXworks)
│   ├── main.tex
│   ├── main.pdf                # PDF compilado (portada + 6 páginas de cuerpo + 1 anexo)
│   ├── partes/01caratula.tex   # Carátula institucional UNSAAC
│   └── figuras/                # Logo institucional, capturas reales PNG y figuras compiladas
├── presentacion/               # Activos de la presentación Canva y guion de exposición
│   ├── figuras/                # Gráficos y diagramas de flujo de alta precisión (tema #0b1e35)
│   └── guion_demo_en_vivo.md   # Guion cronometrado de exposición y demostración en vivo
├── Materiales/                 # Sílabo, plan de sesiones y enunciado oficial del curso
├── AGENTS.md                   # Reglas y convenciones del proyecto para Antigravity
└── .agents/skills/             # Skills modulares del workspace
```

---

## Compilación y verificación rápida (Linux / WSL2 Ubuntu)

```bash
cd src
cmake -S . -B build
cmake --build build -j

# 1. Validar numéricamente E1–E5 (sin ventana gráfica)
./build/test_experiments ../experimentos

# 2. Abrir laboratorio interactivo OpenGL 2.1 + GLFW 3
./build/lab_afines
```

## Compilar el informe técnico (Windows MiKTeX / TeXworks)

```powershell
cd informe
pdflatex -interaction=nonstopmode main.tex
pdflatex -interaction=nonstopmode main.tex
```

## Presentación para exposición

- **Canva (editable por el equipo)**: [Presentacion_Tema2_Transformaciones_Afines_UNSAAC_2026_02](https://www.canva.com/d/ZWZ0Jh78q8gNv94) (basada en la carátula institucional del equipo, 11 diapositivas + notas de presentador).
- **Guion de demostración en vivo**: ver `presentacion/guion_demo_en_vivo.md`.
