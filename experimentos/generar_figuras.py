"""
Genera todas las figuras de alta precision para el Informe LaTeX (tema claro)
y la Presentacion en Canva (tema azul marino oscuro #0D2137), utilizando
exclusivamente los resultados numericos y capturas raster reales producidos
por el laboratorio C++ (src/lab_afines y src/test_experiments).
"""

import json
import math
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
EXP_DIR = ROOT / "experimentos"
PPM_DIR = EXP_DIR / "capturas_ppm"
INF_FIG_DIR = ROOT / "informe" / "figuras"
PRES_FIG_DIR = ROOT / "presentacion" / "figuras"

INF_FIG_DIR.mkdir(parents=True, exist_ok=True)
(INF_FIG_DIR / "capturas").mkdir(parents=True, exist_ok=True)
PRES_FIG_DIR.mkdir(parents=True, exist_ok=True)

with open(EXP_DIR / "resultados_e1_e5.json", "r", encoding="utf-8") as f:
    DATA = json.load(f)

EXPS = {e["id"]: e for e in DATA["experiments"]}
BASE_POLY = np.array(DATA["base_polygon"])

# Paleta del diseño Canva DAHXvv2xQDU (azul marino institucional exacto)
NAVY_BG = "#0b1e35"
CARD_BG = "#142E4C"
CARD_BORDER = "#2B527A"
TEXT_LIGHT = "#F1F5F9"
TEXT_MUTED = "#94A3B8"
CYAN = "#38BDF8"
EMERALD = "#34D399"
AMBER = "#FBBF24"
CORAL = "#FB7185"
PURPLE = "#C084FC"


def convert_ppms_to_pngs():
    """Convierte las capturas PPM reales del rasterizador C++ a PNG."""
    for ppm in sorted(PPM_DIR.glob("*.ppm")):
        img = Image.open(ppm)
        img.save(INF_FIG_DIR / "capturas" / f"{ppm.stem}.png")


def load_capture(name: str) -> np.ndarray:
    return np.array(Image.open(INF_FIG_DIR / "capturas" / f"{name}.png"))


def style_capture_ax(ax, img, title, subtitle="", dark=False):
    ax.imshow(img, extent=[-14, 14, -10.5, 10.5], interpolation="nearest")
    ax.set_xlim(-14, 14)
    ax.set_ylim(-10.5, 10.5)
    ax.set_xticks([-12, -8, -4, 0, 4, 8, 12])
    ax.set_yticks([-8, -4, 0, 4, 8])
    fg = TEXT_LIGHT if dark else "#1E293B"
    sub_fg = TEXT_MUTED if dark else "#475569"
    ax.tick_params(colors=fg, labelsize=8.5)
    for spine in ax.spines.values():
        spine.set_color(CARD_BORDER if dark else "#94A3B8")
        spine.set_linewidth(1.0)
    ax.set_xlabel("$x$", color=fg, fontsize=9, labelpad=2)
    ax.set_ylabel("$y$", color=fg, fontsize=9, labelpad=2)
    ax.set_title(title, color=fg, fontsize=10.5, fontweight="bold", pad=6)
    if subtitle:
        ax.text(
            0.5, -0.17, subtitle, transform=ax.transAxes,
            ha="center", va="top", fontsize=8.3, color=sub_fg
        )


def make_informe_figures():
    """Figuras limpias de fondo claro para el informe LaTeX."""
    # 1. E1 y E2 lado a lado
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.9), dpi=300)
    fig.patch.set_facecolor("white")
    style_capture_ax(
        axes[0], load_capture("E1"),
        "E1: Cizallamiento horizontal $H_x(0.7)$",
        "$\\det(A)=1.00$, Área: $10.00 \\to 10.00\\,u^2$, $\\sigma(u',v')=0$ (paralelismo conservado)"
    )
    style_capture_ax(
        axes[1], load_capture("E2"),
        "E2: Reflexión respecto al eje vertical $\\mathrm{Ref}_y$",
        "$\\det(A)=-1.00$, Área signo: $+10.00 \\to -10.00$ (CCW $\\to$ CW), $|\\mathrm{Área}|=10.00\\,u^2$"
    )
    # Leyenda comun arriba
    handles = [
        patches.Patch(color="#29384D", label="Polígono original (discontinuo)"),
        patches.Patch(color="#0F5CD1", label="Polígono transformado (sólido)"),
        patches.Patch(color="#0D9E73", label="Segmentos paralelos $L'_1, L'_2$"),
        patches.Patch(color="#A6D9D1", label="Cuadrícula transformada"),
    ]
    fig.legend(handles=handles, loc="upper center", ncol=4, frameon=False, fontsize=8.5, bbox_to_anchor=(0.5, 0.99))
    plt.tight_layout(rect=[0, 0.07, 1, 0.92])
    fig.savefig(INF_FIG_DIR / "fig_e1_e2.png", dpi=300, bbox_inches="tight", pad_inches=0.18)
    plt.close(fig)

    # 2. E3 comparacion de orden
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.9), dpi=300)
    fig.patch.set_facecolor("white")
    style_capture_ax(
        axes[0], load_capture("E3"),
        "E3: Superposición $R(45^\\circ)H_x(0.7)$ (azul) vs $H_x(0.7)R(45^\\circ)$ (naranja)",
        "Raster real C++: ambos órdenes preservan área ($10\\,u^2$) pero difieren geométricamente"
    )
    # Panel derecho: zoom cuantitativo en V4
    ax = axes[1]
    ax.set_facecolor("#F8FAFC")
    p_orig = np.array(DATA["e3_v4_comparison"]["v4_orig"])
    p_a = np.array(DATA["e3_v4_comparison"]["v4_R_Hx"])
    p_b = np.array(DATA["e3_v4_comparison"]["v4_Hx_R"])
    poly_a = np.array(EXPS["E3a"]["trans_polygon"])
    poly_b = np.array(EXPS["E3b"]["trans_polygon"])

    def close(P): return np.vstack([P, P[0]])
    ax.axhline(0, color="#94A3B8", lw=0.8)
    ax.axvline(0, color="#94A3B8", lw=0.8)
    ax.grid(True, linestyle=":", color="#CBD5E1", lw=0.7)
    ax.plot(*close(BASE_POLY).T, "--", color="#475569", lw=1.4, label="Original")
    ax.fill(*close(poly_a).T, color="#0F5CD1", alpha=0.14)
    ax.plot(*close(poly_a).T, "-", color="#0F5CD1", lw=2.0, label="$R(45^\\circ)H_x(0.7)$")
    ax.fill(*close(poly_b).T, color="#E65C0D", alpha=0.14)
    ax.plot(*close(poly_b).T, "-", color="#E65C0D", lw=2.0, label="$H_x(0.7)R(45^\\circ)$")

    ax.scatter([p_orig[0], p_a[0], p_b[0]], [p_orig[1], p_a[1], p_b[1]],
               c=["#334155", "#0F5CD1", "#E65C0D"], s=55, zorder=5)
    ax.annotate("$V_4=(4,5)$", p_orig, xytext=(4.35, 4.75), fontsize=8.5, fontweight="bold", color="#334155")
    ax.annotate("$V'_4=(1.77, 8.84)$", p_a, xytext=(-1.1, 9.15), fontsize=8.5, fontweight="bold", color="#0F5CD1")
    ax.annotate("$V''_4=(3.75, 6.36)$", p_b, xytext=(4.15, 6.55), fontsize=8.5, fontweight="bold", color="#C2410C")
    ax.annotate("", xy=p_b, xytext=p_a, arrowprops=dict(arrowstyle="<->", color="#DC2626", lw=1.8))
    mid = 0.5 * (p_a + p_b)
    ax.text(mid[0] + 0.15, mid[1] + 0.2, "$\\|\\Delta V_4\\|=3.1694$", color="#DC2626", fontsize=9, fontweight="bold")
    ax.set_xlim(-1.5, 7.2)
    ax.set_ylim(-0.5, 10.2)
    ax.set_aspect("equal", adjustable="box")
    ax.set_xlabel("$x$", fontsize=9)
    ax.set_ylabel("$y$", fontsize=9)
    ax.set_title("Cuantificación de la no conmutatividad en $V_4=(4,5)$", fontsize=10.5, fontweight="bold", color="#1E293B", pad=6)
    ax.legend(loc="lower right", fontsize=8.2, framealpha=0.95)

    plt.tight_layout(rect=[0, 0.07, 1, 0.98])
    fig.savefig(INF_FIG_DIR / "fig_e3_orden.png", dpi=300, bbox_inches="tight", pad_inches=0.18)
    plt.close(fig)

    # 3. E4 y E5 lado a lado
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.9), dpi=300)
    fig.patch.set_facecolor("white")
    style_capture_ax(
        axes[0], load_capture("E4"),
        "E4: Composición 4 pasos ($|\\det(A)|=1.20$)",
        "$T(1.5,-1)\\,R(25^\\circ)\\,S(1.5,0.8)\\,H_x(0.5)$ | Área: $10.00 \\to 12.00\\,u^2$ (err $=3.55\\times 10^{-15}$)"
    )
    style_capture_ax(
        axes[1], load_capture("E5"),
        "E5: Caso degenerado singular $S(1.0, 0.0)$",
        "$\\det(A)=0.00$, Área $=0$, colapso sobre $y=0$, $L'_1$ y $L'_2$ coincidentes, sin inversa"
    )
    plt.tight_layout(rect=[0, 0.07, 1, 0.98])
    fig.savefig(INF_FIG_DIR / "fig_e4_e5.png", dpi=300, bbox_inches="tight", pad_inches=0.18)
    plt.close(fig)

    # 4. Reflexion sobre recta oblicua phi=30 + composicion
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.7), dpi=300)
    fig.patch.set_facecolor("white")
    style_capture_ax(
        axes[0], load_capture("REF30"),
        "Verificación $\\mathrm{Ref}_{30^\\circ} = R(30^\\circ)\\,\\mathrm{Ref}_x\\,R(-30^\\circ)$",
        "Eje morado $y = \\tan(30^\\circ)x$ | $\\det(A)=-1.00$, Área signo: $+10 \\to -10$"
    )
    # Diagrama de etapas en panel derecho
    ax = axes[1]
    ax.set_facecolor("#F8FAFC")
    ax.set_xlim(-1.5, 7.5)
    ax.set_ylim(-5.5, 6.0)
    ax.axhline(0, color="#64748B", lw=0.9)
    ax.axvline(0, color="#64748B", lw=0.9)
    ax.grid(True, linestyle=":", color="#CBD5E1", lw=0.7)
    t = np.linspace(-1.5, 7.5, 50)
    ax.plot(t, t * math.tan(math.radians(30)), "--", color="#9333EA", lw=1.6, label="Recta $\\phi=30^\\circ$")

    def rot(deg):
        r = math.radians(deg)
        return np.array([[math.cos(r), -math.sin(r)], [math.sin(r), math.cos(r)]])
    refx = np.array([[1, 0], [0, -1]])
    p0 = BASE_POLY
    p1 = (rot(-30) @ p0.T).T
    p2 = (refx @ p1.T).T
    p3 = (rot(30) @ p2.T).T

    def close(P): return np.vstack([P, P[0]])
    ax.plot(*close(p0).T, "--", color="#334155", lw=1.5, label="0. Original $P$")
    ax.plot(*close(p1).T, "-.", color="#0284C7", lw=1.5, label="1. $P_1 = R(-30^\\circ)P$")
    ax.plot(*close(p2).T, ":", color="#D97706", lw=1.8, label="2. $P_2 = \\mathrm{Ref}_x P_1$")
    ax.fill(*close(p3).T, color="#0F5CD1", alpha=0.16)
    ax.plot(*close(p3).T, "-", color="#0F5CD1", lw=2.1, label="3. $P' = R(30^\\circ)P_2$")
    ax.set_aspect("equal", adjustable="box")
    ax.set_xlabel("$x$", fontsize=9)
    ax.set_ylabel("$y$", fontsize=9)
    ax.set_title("Descomposición geométrica paso a paso", fontsize=10.5, fontweight="bold", color="#1E293B", pad=6)
    ax.legend(loc="lower right", fontsize=7.8, framealpha=0.95)

    plt.tight_layout(rect=[0, 0.07, 1, 0.98])
    fig.savefig(INF_FIG_DIR / "fig_ref30_steps.png", dpi=300, bbox_inches="tight", pad_inches=0.18)
    plt.close(fig)


def draw_box(ax, x, y, w, h, title, body, edge=CYAN, fill=CARD_BG, title_color=TEXT_LIGHT):
    box = FancyBboxPatch(
        (x, y), w, h, boxstyle="round,pad=0.02,rounding_size=0.03",
        facecolor=fill, edgecolor=edge, linewidth=2.0
    )
    ax.add_patch(box)
    ax.text(x + w / 2, y + h * 0.68, title, ha="center", va="center",
            fontsize=11.5, fontweight="bold", color=title_color)
    if body:
        ax.text(x + w / 2, y + h * 0.30, body, ha="center", va="center",
                fontsize=9.3, color=TEXT_MUTED)


def make_presentation_figures():
    """Figuras explicativas de alto impacto en tema azul marino (#0D2137) para Canva."""

    # ---- Slide 2: Paradoja visual (apariencia vs cuantificacion) ----
    fig, axes = plt.subplots(1, 2, figsize=(11.0, 4.6), dpi=260)
    fig.patch.set_facecolor(NAVY_BG)
    sq = np.array([[0, 0], [4, 0], [4, 4], [0, 4], [0, 0]])
    Hx = np.array([[1, 0.7], [0, 1]])
    sh = (Hx @ sq.T).T

    for idx, (ax, pts, col, ttl, notes) in enumerate([
        (axes[0], sq, CYAN, "Cuadrado original ($4 \\times 4$)",
         "Base $= 4.0$, Altura $= 4.0$\nÁngulos $= 90^\\circ$, Área $= 16.0\\,u^2$"),
        (axes[1], sh, AMBER, "Tras cizallamiento $H_x(0.7)$",
         "Ángulo baja a $55.01^\\circ$, diagonal crece $+22\\%$\n¡Pero $\\det(H_x)=1 \\Rightarrow$ Área $= 16.0\\,u^2$ exacta!")
    ]):
        ax.set_facecolor(CARD_BG)
        # Grilla deformada o base
        for g in range(5):
            a0, b0 = np.array([g, 0]), np.array([g, 4])
            c0, d0 = np.array([0, g]), np.array([4, g])
            if idx == 1:
                a0, b0, c0, d0 = Hx @ a0, Hx @ b0, Hx @ c0, Hx @ d0
            ax.plot([a0[0], b0[0]], [a0[1], b0[1]], color="#335C85", lw=0.9)
            ax.plot([c0[0], d0[0]], [c0[1], d0[1]], color="#335C85", lw=0.9)
        ax.fill(pts[:, 0], pts[:, 1], color=col, alpha=0.22)
        ax.plot(pts[:, 0], pts[:, 1], color=col, lw=2.8)
        ax.set_xlim(-0.8, 7.6)
        ax.set_ylim(-0.8, 5.2)
        ax.set_aspect("equal", adjustable="box")
        ax.tick_params(colors=TEXT_MUTED, labelsize=8.5)
        for sp in ax.spines.values():
            sp.set_color(CARD_BORDER)
        ax.set_title(ttl, color=TEXT_LIGHT, fontsize=11.5, fontweight="bold", pad=8)
        ax.text(0.5, 0.06, notes, transform=ax.transAxes, ha="center", va="bottom",
                fontsize=9.2, color=TEXT_LIGHT,
                bbox=dict(boxstyle="round,pad=0.35", facecolor=NAVY_BG, edgecolor=col, lw=1.2))
    plt.tight_layout(pad=1.6)
    fig.savefig(PRES_FIG_DIR / "slide_02_paradoja_visual.png", dpi=260, facecolor=NAVY_BG)
    plt.close(fig)

    # ---- Slide 3: Derivacion paso a paso Ref_phi = R(phi) Ref_x R(-phi) ----
    fig, axes = plt.subplots(1, 4, figsize=(12.8, 3.6), dpi=260)
    fig.patch.set_facecolor(NAVY_BG)
    phi = 30.0
    def rot(d):
        r = math.radians(d)
        return np.array([[math.cos(r), -math.sin(r)], [math.sin(r), math.cos(r)]])
    refx = np.array([[1, 0], [0, -1]])
    P0 = BASE_POLY
    P1 = (rot(-phi) @ P0.T).T
    P2 = (refx @ P1.T).T
    P3 = (rot(phi) @ P2.T).T

    stages = [
        (P0, CYAN,   "0. Original $P$", "Eje oblicuo $\\phi=30^\\circ$", True),
        (P1, EMERALD,"1. $R(-30^\\circ)\\,P$", "Alinea recta con eje $X$", False),
        (P2, AMBER,  "2. $\\mathrm{Ref}_x\\,P_1$", "Refleja sobre eje $X$", False),
        (P3, CORAL,  "3. $R(30^\\circ)\\,P_2$", "Devuelve el eje a $\\phi$", True),
    ]
    def close(P): return np.vstack([P, P[0]])
    t = np.linspace(-2, 8, 40)
    for ax, (pts, col, ttl, sub, tilted) in zip(axes, stages):
        ax.set_facecolor(CARD_BG)
        ax.axhline(0, color="#475569", lw=0.9)
        ax.axvline(0, color="#475569", lw=0.9)
        if tilted:
            ax.plot(t, t * math.tan(math.radians(phi)), "--", color=PURPLE, lw=1.8)
        else:
            ax.axhline(0, color=PURPLE, ls="--", lw=2.0)
        if pts is not P0:
            ax.plot(*close(P0).T, ":", color=TEXT_MUTED, lw=1.1, alpha=0.6)
        ax.fill(*close(pts).T, color=col, alpha=0.22)
        ax.plot(*close(pts).T, color=col, lw=2.3)
        ax.set_xlim(-1.5, 7.2)
        ax.set_ylim(-5.2, 5.8)
        ax.set_aspect("equal", adjustable="box")
        ax.set_xticks([])
        ax.set_yticks([])
        for sp in ax.spines.values():
            sp.set_color(col)
            sp.set_linewidth(1.5)
        ax.set_title(ttl, color=TEXT_LIGHT, fontsize=10.5, fontweight="bold", pad=6)
        ax.set_xlabel(sub, color=TEXT_MUTED, fontsize=8.8, labelpad=5)
    plt.tight_layout(pad=1.0)
    fig.savefig(PRES_FIG_DIR / "slide_03_reflexion_pasos.png", dpi=260, facecolor=NAVY_BG)
    plt.close(fig)

    # ---- Slide 4: Papel de det(A) e invariantes afines ----
    fig, axes = plt.subplots(1, 3, figsize=(12.0, 3.8), dpi=260)
    fig.patch.set_facecolor(NAVY_BG)
    unit = np.array([[0, 0], [2, 0], [2, 2], [0, 2], [0, 0]])
    cases = [
        (np.array([[1.2, 0.5], [0.0, 1.0]]), EMERALD,
         "$\\det(A) > 0$  ($+1.2$)", "Conserva orientación (CCW)\nÁrea escala $\\times 1.2$ · Invertible"),
        (np.array([[-1.0, 0.0], [0.0, 1.0]]), AMBER,
         "$\\det(A) < 0$  ($-1.0$)", "Invierte orientación (CCW $\\to$ CW)\nÁrea física $= |-1|\\cdot A = A$ · Invertible"),
        (np.array([[1.0, 0.0], [0.0, 0.0]]), CORAL,
         "$\\det(A) = 0$  (Singular)", "Colapso 2D $\\to$ 1D (Área $= 0$)\nPérdida de inyectividad · SIN inversa"),
    ]
    for ax, (A, col, ttl, desc) in zip(axes, cases):
        ax.set_facecolor(CARD_BG)
        ax.axhline(0, color="#475569", lw=0.8)
        ax.axvline(0, color="#475569", lw=0.8)
        tr = (A @ unit.T).T
        ax.plot(unit[:, 0], unit[:, 1], "--", color=TEXT_MUTED, lw=1.3)
        if np.linalg.det(A) != 0:
            ax.fill(tr[:, 0], tr[:, 1], color=col, alpha=0.22)
        ax.plot(tr[:, 0], tr[:, 1], color=col, lw=3.0)
        # Flecha indicando orientacion en primer lado
        ax.annotate("", xy=tr[1], xytext=tr[0], arrowprops=dict(arrowstyle="->", color=col, lw=2.2))
        ax.set_xlim(-2.6, 3.8)
        ax.set_ylim(-1.2, 2.8)
        ax.set_aspect("equal", adjustable="box")
        ax.set_xticks([])
        ax.set_yticks([])
        for sp in ax.spines.values():
            sp.set_color(col)
            sp.set_linewidth(1.5)
        ax.set_title(ttl, color=col, fontsize=11, fontweight="bold", pad=7)
        ax.set_xlabel(desc, color=TEXT_LIGHT, fontsize=8.8, labelpad=6)
    plt.tight_layout(pad=1.2)
    fig.savefig(PRES_FIG_DIR / "slide_04_det_regimenes.png", dpi=260, facecolor=NAVY_BG)
    plt.close(fig)

    # ---- Slide 5: Diagrama de flujo del algoritmo y arquitectura C++ ----
    fig, ax = plt.subplots(figsize=(12.6, 5.2), dpi=260)
    fig.patch.set_facecolor(NAVY_BG)
    ax.set_facecolor(NAVY_BG)
    ax.set_xlim(0, 12.6)
    ax.set_ylim(0, 5.2)
    ax.axis("off")

    # Fila superior: Pipeline matematico
    draw_box(ax, 0.25, 3.15, 2.55, 1.55, "1. Entrada / Presets",
             "Teclas 0–8, modo vivo,\nconstructor consola (N >= 3)\nT, R, S, Hx, Hy, Ref", edge=CYAN)
    draw_box(ax, 3.35, 3.15, 2.75, 1.55, "2. math_affine.hpp",
             "Matrices propias Matrix3\nM = Tn * ... * T2 * T1\np' = M * p (columna)", edge=EMERALD)
    draw_box(ax, 6.65, 3.15, 2.75, 1.55, "3. Control det(A)",
             "det(A) = a00*a11 - a01*a10\nSi |det(A)| < 1e-9:\nBLOQUEA INVERSA + ALERTA", edge=CORAL)
    draw_box(ax, 9.95, 3.15, 2.40, 1.55, "4. Métricas Reales",
             "Área Zapatero (+ / |·|)\nOrientación CCW / CW\nParalelismo |u x v|/(||u||||v||)", edge=AMBER)

    # Fila inferior: Rasterizacion propia + Salida dual
    draw_box(ax, 1.30, 0.55, 3.25, 1.60, "5. raster.hpp (Sin glTransform)",
             "Mapeo mundo -> rejilla 280x210\nputPixel() / putPoint()\nBresenham 8 octantes + drawPolygon()", edge=PURPLE)
    draw_box(ax, 5.25, 0.55, 3.10, 1.60, "6. Ventana OpenGL 2.1 / GLFW",
             "glOrtho(0, 280, 0, 210)\nEje Y hacia arriba\nOriginal vs Transformada + Grilla", edge=CYAN)
    draw_box(ax, 9.05, 0.55, 3.25, 1.60, "7. Validación & Evidencia",
             "test_experiments (err < 1e-9)\nExporta JSON + TXT + PPM\n(Inversión Y_ppm = H-1-y)", edge=EMERALD)

    def arrow(x1, y1, x2, y2, col=TEXT_LIGHT):
        ax.add_patch(FancyArrowPatch((x1, y1), (x2, y2),
                                     arrowstyle="-|>,head_length=8,head_width=5",
                                     color=col, lw=2.0, mutation_scale=1.0))

    arrow(2.82, 3.92, 3.32, 3.92, CYAN)
    arrow(6.12, 3.92, 6.62, 3.92, EMERALD)
    arrow(9.42, 3.92, 9.92, 3.92, CORAL)
    # Bajada desde math_affine a raster
    arrow(4.72, 3.12, 3.10, 2.18, PURPLE)
    arrow(4.57, 1.35, 5.22, 1.35, CYAN)
    arrow(11.15, 3.12, 10.75, 2.18, AMBER)
    arrow(8.38, 1.35, 9.02, 1.35, EMERALD)

    ax.text(6.3, 4.95, "Arquitectura Modular y Flujo Algorítmico del Laboratorio C++17 (Sin funciones automáticas de OpenGL)",
            ha="center", va="center", fontsize=12.5, fontweight="bold", color=TEXT_LIGHT)
    plt.tight_layout(pad=0.3)
    fig.savefig(PRES_FIG_DIR / "slide_05_arquitectura_flujo.png", dpi=260, facecolor=NAVY_BG)
    plt.close(fig)

    # ---- Slide 6: E1 y E2 (capturas reales C++ sobre tema oscuro) ----
    fig, axes = plt.subplots(1, 2, figsize=(12.2, 4.9), dpi=260)
    fig.patch.set_facecolor(NAVY_BG)
    style_capture_ax(
        axes[0], load_capture("E1"),
        "E1 · Cizallamiento $H_x(0.7)$",
        "u'=v'=(5.4, 2.0) · seno=0.000 · Área=10.00 · ∠V0: 90°→55.01°",
        dark=True
    )
    style_capture_ax(
        axes[1], load_capture("E2"),
        "E2 · Reflexión vertical $\\mathrm{Ref}_y$",
        "det(A)=-1 · Área signo: +10 → -10 (CW) · |Área|=10.00",
        dark=True
    )
    plt.tight_layout(rect=[0, 0.08, 1, 0.94], w_pad=2.2)
    fig.savefig(PRES_FIG_DIR / "slide_06_e1_e2.png", dpi=260, facecolor=NAVY_BG, bbox_inches="tight", pad_inches=0.22)
    plt.close(fig)

    # ---- Slide 7: E3 Comparacion de orden ----
    fig, axes = plt.subplots(1, 2, figsize=(12.2, 4.9), dpi=260)
    fig.patch.set_facecolor(NAVY_BG)
    style_capture_ax(
        axes[0], load_capture("E3"),
        "E3 · Raster real: $R(45^\\circ)H_x(0.7)$ vs $H_x(0.7)R(45^\\circ)$",
        "Azul: primero Hx luego R | Naranja: primero R luego Hx",
        dark=True
    )
    ax = axes[1]
    ax.set_facecolor(CARD_BG)
    p_orig = np.array(DATA["e3_v4_comparison"]["v4_orig"])
    p_a = np.array(DATA["e3_v4_comparison"]["v4_R_Hx"])
    p_b = np.array(DATA["e3_v4_comparison"]["v4_Hx_R"])
    poly_a = np.array(EXPS["E3a"]["trans_polygon"])
    poly_b = np.array(EXPS["E3b"]["trans_polygon"])
    ax.axhline(0, color="#475569", lw=0.8)
    ax.axvline(0, color="#475569", lw=0.8)
    ax.grid(True, ls=":", color="#2B527A", lw=0.7)
    ax.plot(*close(BASE_POLY).T, "--", color=TEXT_MUTED, lw=1.5, label="Original")
    ax.fill(*close(poly_a).T, color=CYAN, alpha=0.18)
    ax.plot(*close(poly_a).T, "-", color=CYAN, lw=2.3, label="$R(45^\\circ)H_x(0.7)$")
    ax.fill(*close(poly_b).T, color=AMBER, alpha=0.18)
    ax.plot(*close(poly_b).T, "-", color=AMBER, lw=2.3, label="$H_x(0.7)R(45^\\circ)$")
    ax.scatter([p_orig[0], p_a[0], p_b[0]], [p_orig[1], p_a[1], p_b[1]],
               c=[TEXT_LIGHT, CYAN, AMBER], s=65, zorder=5)
    ax.annotate("$V_4=(4,5)$", p_orig, xytext=(4.35, 4.75), fontsize=9, fontweight="bold", color=TEXT_LIGHT)
    ax.annotate("$V'_4=(1.77, 8.84)$", p_a, xytext=(-1.1, 9.15), fontsize=9, fontweight="bold", color=CYAN)
    ax.annotate("$V''_4=(3.75, 6.36)$", p_b, xytext=(4.10, 6.55), fontsize=9, fontweight="bold", color=AMBER)
    ax.annotate("", xy=p_b, xytext=p_a, arrowprops=dict(arrowstyle="<->", color=CORAL, lw=2.2))
    mid = 0.5 * (p_a + p_b)
    ax.text(mid[0] + 0.15, mid[1] + 0.25, "$\\|\\Delta V_4\\| = 3.1694$", color=CORAL, fontsize=10, fontweight="bold")
    ax.set_xlim(-1.5, 7.2)
    ax.set_ylim(-0.5, 10.2)
    ax.set_aspect("equal", adjustable="box")
    ax.tick_params(colors=TEXT_MUTED, labelsize=8.5)
    for sp in ax.spines.values():
        sp.set_color(CARD_BORDER)
    ax.set_title("Brecha cuantificada en el vértice $V_4=(4,5)$", color=TEXT_LIGHT, fontsize=10.5, fontweight="bold", pad=6)
    ax.legend(loc="lower right", fontsize=8.5, facecolor=NAVY_BG, edgecolor=CARD_BORDER, labelcolor=TEXT_LIGHT)
    plt.tight_layout(rect=[0, 0.08, 1, 0.94], w_pad=2.0)
    fig.savefig(PRES_FIG_DIR / "slide_07_e3_orden.png", dpi=260, facecolor=NAVY_BG, bbox_inches="tight", pad_inches=0.22)
    plt.close(fig)

    # ---- Slide 8: E4 y E5 ----
    fig, axes = plt.subplots(1, 2, figsize=(12.2, 4.9), dpi=260)
    fig.patch.set_facecolor(NAVY_BG)
    style_capture_ax(
        axes[0], load_capture("E4"),
        "E4 · Composición 4 pasos ($|\\det(A)|=1.20$)",
        "Área calculada: 12.00 u² = 1.2 × 10.00 u² (error 3.55e-15)",
        dark=True
    )
    style_capture_ax(
        axes[1], load_capture("E5"),
        "E5 · Caso degenerado $S(1.0, 0.0)$ ($\\det=0$)",
        "Polígono y grilla colapsan a y=0 · L1'=L2' coincidentes · Sin inversa",
        dark=True
    )
    plt.tight_layout(rect=[0, 0.08, 1, 0.94], w_pad=2.2)
    fig.savefig(PRES_FIG_DIR / "slide_08_e4_e5.png", dpi=260, facecolor=NAVY_BG, bbox_inches="tight", pad_inches=0.22)
    plt.close(fig)


if __name__ == "__main__":
    convert_ppms_to_pngs()
    make_informe_figures()
    make_presentation_figures()
    print("[OK] Capturas convertidas y figuras generadas en informe/figuras/ y presentacion/figuras/.")
