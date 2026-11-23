---
name: experimentos-tema2
description: >-
  Diseña, ejecuta y valida matemáticamente los 5 experimentos obligatorios (E1–E5)
  del Tema 2 (Transformaciones afines, deformación y propiedades geométricas),
  incluyendo métricas cuantitativas reproducibles y preparación de preguntas de defensa.
---

# Skill: Formalización y Experimentos del Tema 2

## 1. Formalización matemática requerida (Sección 2.3 del enunciado)

1. **Transformación afín**:
   $$p' = A p + t, \quad A = \begin{bmatrix} a_{11} & a_{12} \\ a_{21} & a_{22} \end{bmatrix} \in \mathbb{R}^{2\times 2}, \quad t = \begin{bmatrix} t_x \\ t_y \end{bmatrix}$$
   En coordenadas homogéneas ($p = (x,y,1)^T$):
   $$M = \begin{bmatrix} a_{11} & a_{12} & t_x \\ a_{21} & a_{22} & t_y \\ 0 & 0 & 1 \end{bmatrix}$$
2. **Cizallamiento (Shear)**:
   $$H_x(k) = \begin{bmatrix} 1 & k & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1 \end{bmatrix}, \qquad H_y(k) = \begin{bmatrix} 1 & 0 & 0 \\ k & 1 & 0 \\ 0 & 0 & 1 \end{bmatrix}$$
   Nótese $\det(H_x(k)) = \det(H_y(k)) = 1$: conserva área y orientación, pero altera longitudes y ángulos.
3. **Reflexiones**:
   - Eje X: $\mathrm{Ref}_x = \mathrm{diag}(1, -1, 1)$; Eje Y: $\mathrm{Ref}_y = \mathrm{diag}(-1, 1, 1)$.
   - Recta por el origen con ángulo $\phi$:
     $$\mathrm{Ref}_\phi = R(\phi)\,\mathrm{Ref}_x\,R(-\phi) = \begin{bmatrix} \cos 2\phi & \sin 2\phi & 0 \\ \sin 2\phi & -\cos 2\phi & 0 \\ 0 & 0 & 1 \end{bmatrix}$$
   Nótese $\det(\mathrm{Ref}_\phi) = -1$: conserva área absoluta y longitudes, invierte orientación.
4. **Papel de $\det(A)$**:
   - Factor de cambio de área: $\text{Área}' = |\det(A)| \cdot \text{Área}$ (con signo: $\text{Área}_{\text{signo}}' = \det(A) \cdot \text{Área}_{\text{signo}}$).
   - $\det(A) > 0$: conserva orientación; $\det(A) < 0$: invierte orientación; $\det(A) = 0$: matriz singular, pérdida de dimensión (colapso a recta o punto), no existe $A^{-1}$.
5. **Invariantes afines (con $\det(A) \neq 0$)**:
   - **Se conservan**: rectas, colinealidad, paralelismo y razones de segmentos colineales (puntos medios, divisiones proporcionales).
   - **NO se conservan en general**: longitudes euclidianas, ángulos, perpendicularidad y áreas (solo en casos particulares: isometrías / equiafines).

## 2. Métricas cuantitativas obligatorias

- **Área con signo (fórmula del zapatero / shoelace)**:
  $$A_{\text{signo}} = \frac{1}{2} \sum_{i=0}^{n-1} (x_i y_{i+1} - x_{i+1} y_i), \quad (x_n, y_n) = (x_0, y_0)$$
  - $A_{\text{signo}} > 0 \implies$ recorrido antihorario (CCW).
  - $A_{\text{signo}} < 0 \implies$ recorrido horario (CW).
  - $|A_{\text{signo}}| = 0 \implies$ polígono degenerado/colineal.
- **Medida de paralelismo** para dos segmentos $s_1 = (P_1, Q_1)$ y $s_2 = (P_2, Q_2)$:
  - Vectores directores $u = Q_1 - P_1$, $v = Q_2 - P_2$.
  - Si $\|u\| > \varepsilon$ y $\|v\| > \varepsilon$, calcular el seno del ángulo entre directores:
    $$\sigma(u, v) = \frac{|u_x v_y - u_y v_x|}{\|u\|\,\|v\|} \in [0, 1]$$
    donde $\sigma = 0$ indica paralelismo exacto (reportar también el ángulo $\alpha = \arcsin(\min(1, \sigma))$ en grados).
  - Si $\|u\| \le \varepsilon$ o $\|v\| \le \varepsilon$ (p. ej. en E5 caso degenerado), reportar explícitamente que al menos un segmento colapsó a un punto y el paralelismo no está definido.

## 3. Protocolo de los 5 experimentos obligatorios

| ID | Experimento | Matriz / Secuencia | Qué verificar numéricamente y visualmente |
| :--- | :--- | :--- | :--- |
| **E1** | Cizallamiento | $H_x(0.7)$ | Coordenadas de los 2 segmentos paralelos antes y después; $\sigma(u', v') \approx 0$; $\det(A)=1$ y área conservada, mientras ángulos y longitudes cambian. |
| **E2** | Reflexión | $\mathrm{Ref}_y = \mathrm{diag}(-1, 1, 1)$ | Comparar $A_{\text{signo}}$ (cambia de signo), $|A_{\text{signo}}|$ (igual), orientación CCW $\to$ CW, $\det(A) = -1$. |
| **E3** | Orden (no conmutatividad) | $M_1 = R(45^\circ)H_x(0.7)$ vs $M_2 = H_x(0.7)R(45^\circ)$ | Misma figura inicial; comparar coordenadas exactas de un vértice elegido $v_k$ bajo $M_1$ y $M_2$, distancia $\|M_1 v_k - M_2 v_k\|$ y matrices compuestas distintas. |
| **E4** | Cambio de área | Composición de $\ge 3$ transformaciones con $|\det(A)| \neq 1$ (p. ej. $T \cdot R \cdot S(1.5, 0.8) \cdot H_x$) | Contrastar $|A'_{\text{signo}}|$ calculado por zapatero contra $|\det(A)| \cdot |A_{\text{signo}}|$; verificar error relativo $< 10^{-9}$. |
| **E5** | Caso degenerado | Escalamiento singular, p. ej. $S(1.5, 0)$ (o $S(0, 1)$) | $\det(A) = 0$, $A'_{\text{signo}} = 0$, todos los vértices colapsan sobre una recta, la orientación y la inversa dejan de existir, advertencia explícita en consola/UI. |

## 4. Validación cruzada reproducible

- Exportar desde C++ (o validar en paralelo con un script Python en `experimentos/verificar_experimentos.py`) las tablas exactas de vértices, matrices $3\times 3$, determinantes, áreas y métricas de paralelismo para E1–E5.
- Guardar los resultados en `experimentos/resultados_e1_e5.json` o `.csv` para que el informe LaTeX y las diapositivas usen **exactamente los mismos números** que imprime el programa C++.

## 5. Preguntas de defensa que el equipo debe dominar

1. **¿Puede una transformación conservar el área y alterar los ángulos?** Sí: el cizallamiento $H_x(k)$ con $k \neq 0$ (y en general cualquier transformación con $|\det(A)| = 1$ no ortogonal, como $S(2, 0.5)$) conserva área pero deforma ángulos y longitudes.
2. **¿Por qué el determinante negativo no implica área negativa en sentido físico?** El signo de $\det(A)$ codifica la inversión de orientación (chirality/recorrido horario vs antihorario); el área geométrica/física siempre es no negativa y escala por $|\det(A)|$.
3. **¿Por qué una matriz singular requiere un tratamiento especial?** Porque $\det(A)=0$ implica rango $< 2$ (núcleo no trivial): múltiples puntos distintos se proyectan al mismo punto, la transformación no es inyectiva ni invertible, el área colapsa a $0$, el paralelismo/orientación pueden quedar indefinidos y dividir entre $\det(A)$ provocaría división por cero o `NaN`/`Inf`.
