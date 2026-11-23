---
name: informe-latex
description: >-
  Redacta y compila en PDF el informe técnico breve (5–8 páginas sin portada ni anexos)
  del Tema 2 en LaTeX usando MiKTeX (pdflatex), aplicando el modelo de carátula institucional
  UNSAAC del usuario y coherencia exacta con las salidas numéricas y capturas del programa C++.
---

# Skill: Informe Técnico en LaTeX (MiKTeX / TeXworks)

## Entorno verificado y elección técnica

- **Herramienta elegida**: **LaTeX (`pdflatex` / MiKTeX + TeXworks)** — óptima para esta PC y flujo agéntico por compilación local determinista, soporte nativo de matrices $3\times 3$/TikZ, control de versiones limpio en Git e inyección directa de tablas generadas por C++.
- Compiladores en Windows `PATH`: `pdflatex.exe` y `xelatex.exe` (MiKTeX).

## Modelo de carátula obligatorio (adaptado de `01caratula.tex`)

- Basar `informe/partes/01caratula.tex` en `E:\Lucian\Tesis\01_PLAN_TESIS\05_documento\partes\01caratula.tex` usando el escudo ya copiado localmente en `informe/figuras/institucional/logo.png` (sin rutas absolutas externas).
- Paquetes necesarios para esa carátula: `graphicx`, `tcolorbox` (`[most]`), `textpos` (`[absolute,overlay]`).
- Adaptar los campos de tesis al curso:
  - Universidad: `UNIVERSIDAD NACIONAL DE SAN ANTONIO ABAD DEL CUSCO`
  - Subtítulo superior: `TRABAJO DE INVESTIGACIÓN Y APLICACIÓN — SEGUNDA PARCIAL` / `COMPUTACIÓN GRÁFICA I (IF455AIN)`
  - Recuadro `tcolorbox` (bordes superior e inferior negros de 2pt): título en mayúsculas negrita del Tema 2 (`TRANSFORMACIONES AFINES, DEFORMACIÓN Y PROPIEDADES GEOMÉTRICAS EN 2D`).
  - Bloque derecho (`textblock*`):
    - **Asignatura**: Computación Gráfica I (`IF455AIN`) — Semestre 2026-02
    - **Docente**: PhD. Carlos Fernando Montoya Cubas
    - **Presentado por**:
      - CHOQUENAIRA GARCIA, Ronal Franklin (174941)
      - FERNANDEZ BACA CASTRO, Lucian Neptalí (184197)
  - Pie centrado: `Cusco, Perú — Noviembre 2026`, `ESCUELA PROFESIONAL DE INGENIERÍA INFORMÁTICA Y DE SISTEMAS`, `FACULTAD DE INGENIERÍA ELÉCTRICA, ELECTRÓNICA, INFORMÁTICA Y MECÁNICA`.

## Requisitos del informe (enunciado oficial)

- **Extensión sugerida**: **5–8 páginas** sin contar portada ni anexos.
- **Secciones obligatorias**:
  1. Portada (no cuenta en las 5–8 páginas).
  2. Planteamiento del problema, objetivo e hipótesis/preguntas de investigación.
  3. Marco teórico y formalización matemática (convención $p'=Mp$, eje vertical, derivación de $H_x$, $H_y$, $\mathrm{Ref}_\phi = R(\phi)\mathrm{Ref}_x R(-\phi)$, papel de $\det(A)$, invariantes afines y contraejemplos para ángulos/longitudes).
  4. Arquitectura e implementación en C++ (separación matemática/rasterización, `Matrix3`, `putPoint()`, Bresenham, controles y manejo seguro de $\det(A)=0$).
  5. Experimentos controlados E1–E5 (tablas con coordenadas, matrices $3\times 3$, $\det(A)$, área con signo y absoluta, medida de paralelismo, más capturas reales del programa y/o diagramas contrastados).
  6. Análisis, discusión de limitaciones (errores de rasterización/redondeo a píxel, casos degenerados) y respuestas técnicas a las 3 preguntas de defensa.
  7. Conclusiones.
  8. Bibliografía real y verificada (citando textos del sílabo, formato consistente).
  9. Anexos (tabla de controles, guía de compilación rápida y matriz de contribuciones de los integrantes).

## Reglas de calidad y coherencia

- Todos los valores numéricos de las tablas deben provenir de la ejecución real del código C++ (verificados con `test_experiments`).
- Cero referencias inventadas: solo citar fuentes reales y verificables (Hearn et al. 2011, Foley et al. 2014, Shirley et al. 2021, Vince 2012, Angel & Shreiner 2015).
- Incluir explícitamente las contribuciones de ambos integrantes (requisito de la lista de comprobación, pág. 8).

## Compilación desde PowerShell

```powershell
Set-Location "E:\Lucian\CG1\Unidad II\Expo\informe"
pdflatex -interaction=nonstopmode main.tex
pdflatex -interaction=nonstopmode main.tex
```

Verificar al final:
- Código de salida `0` y ausencia de referencias indefinidas (`??`).
- Conteo de páginas del cuerpo entre 5 y 8 páginas (excluyendo portada y anexos).
- Sin `Overfull \hbox` críticos ni tablas desbordadas.
