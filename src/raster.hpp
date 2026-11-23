#pragma once

#include "math_affine.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// ============================================================================
// Modulo de rasterizacion propia (estilo Examen de Laboratorio UNSAAC)
//
// Escala de alta claridad visual:
//   - PX_PER_UNIT = 4 celdas discretas por cada unidad matematica.
//   - Rejilla discreta 160 x 120 sobre ventana 1280 x 960 (8 x 8 px por celda).
//   - Rango cartesiano identico: x in [-20, 20], y in [-15, 15].
//   - Origen cartesiano (0, 0) en el centro de la rejilla: (80, 60).
//   - Eje Y crece hacia arriba en glOrtho(0, 160, 0, 120, -1, 1).
//   - Al exportar imagenes PPM (fila 0 arriba), se invierte verticalmente.
// ============================================================================

namespace cg {

struct Color {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
};

inline constexpr int PX_PER_UNIT   = 4;
inline constexpr int CELL_PX       = 8;
inline constexpr int GRID_WIDTH    = 40 * PX_PER_UNIT;   // 160 celdas
inline constexpr int GRID_HEIGHT   = 30 * PX_PER_UNIT;   // 120 celdas
inline constexpr int ORIGIN_X      = 20 * PX_PER_UNIT;   // 80
inline constexpr int ORIGIN_Y      = 15 * PX_PER_UNIT;   // 60
inline constexpr int WINDOW_WIDTH  = GRID_WIDTH * CELL_PX;  // 1280 px
inline constexpr int WINDOW_HEIGHT = GRID_HEIGHT * CELL_PX; // 960 px
inline constexpr float CELL_MARGIN = 0.06f;

// Paleta con jerarquia visual de alto contraste sobre fondo blanco puro
inline constexpr Color COL_BG           = {1.00f, 1.00f, 1.00f}; // #FFFFFF
inline constexpr Color COL_GRID_SUB     = {0.95f, 0.96f, 0.98f}; // #F2F5FA (sub-rejilla de pixeles)
inline constexpr Color COL_GRID_LINE    = {0.86f, 0.89f, 0.93f}; // #DBE3ED (rejilla cada 1 unidad)
inline constexpr Color COL_AXES         = {0.75f, 0.79f, 0.85f}; // #BFC9D9 (ejes cartesianos 1px)
inline constexpr Color COL_AXES_TICK    = {0.55f, 0.61f, 0.70f}; // #8C9CB3 (marcas cada 5 unidades)
inline constexpr Color COL_TGRID_ORIG   = {0.88f, 0.91f, 0.95f}; // #E0E8F2 (cuadricula base tenue)
inline constexpr Color COL_TGRID_TRANS  = {0.72f, 0.88f, 0.88f}; // #B8E0E0 (cuadricula transformada)
inline constexpr Color COL_TGRID_FAINT  = {0.85f, 0.93f, 0.93f}; // #D9EDED (cuadricula en superposicion)
inline constexpr Color COL_NEGRO        = {0.23f, 0.27f, 0.34f}; // #3A4556 (poligono original)
inline constexpr Color COL_AZUL         = {0.07f, 0.31f, 0.82f}; // #1250D0 (poligono transformado 2px)
inline constexpr Color COL_NARANJA      = {0.90f, 0.36f, 0.00f}; // #E65C00 (segundo orden Hx*R 2px)
inline constexpr Color COL_VERDE        = {0.05f, 0.58f, 0.35f}; // #0D9459 (segmentos paralelos L1', L2')
inline constexpr Color COL_GRIS         = {0.48f, 0.53f, 0.60f}; // #7A8699 (segmentos originales L1, L2)
inline constexpr Color COL_ROJO         = {0.84f, 0.16f, 0.16f}; // #D62828 (marcadores de vertices)
inline constexpr Color COL_MORADO       = {0.49f, 0.23f, 0.93f}; // #7C3AED (recta oblicua / L'' en orden)

// Lienzo discreto 160 x 120: toda geometria se rasteriza exclusivamente
// con putPixel()/putPoint(), Bresenham (drawLine) y drawPolygon().
class RasterCanvas {
public:
    RasterCanvas(int width = GRID_WIDTH, int height = GRID_HEIGHT)
        : width_(width),
          height_(height),
          pixels_(static_cast<std::size_t>(width * height), COL_BG),
          occupied_(static_cast<std::size_t>(width * height), false) {}

    int width() const { return width_; }
    int height() const { return height_; }
    const std::vector<Color>& pixels() const { return pixels_; }
    const std::vector<bool>& occupied() const { return occupied_; }

    void clear(Color bg = COL_BG) {
        std::fill(pixels_.begin(), pixels_.end(), bg);
        std::fill(occupied_.begin(), occupied_.end(), false);
    }

    // Conversion coordenada cartesiana (en unidades matematicas) -> celda discreta
    int worldToGridX(double wx) const {
        return ORIGIN_X + static_cast<int>(std::lround(wx * static_cast<double>(PX_PER_UNIT)));
    }

    int worldToGridY(double wy) const {
        return ORIGIN_Y + static_cast<int>(std::lround(wy * static_cast<double>(PX_PER_UNIT)));
    }

    // Rutina fundamental del curso: pinta una celda logica individual
    void putPixel(int x, int y, float r, float g, float b) {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
        const std::size_t idx = static_cast<std::size_t>(y * width_ + x);
        pixels_[idx] = {r, g, b};
        occupied_[idx] = true;
    }

    // Alias de laboratorio: putPoint()
    void putPoint(int x, int y, Color c = COL_AZUL) {
        putPixel(x, y, c.r, c.g, c.b);
    }

    // Marcador en cruz 3x3 (o 5x5) centrado en (cx, cy) usando putPoint()
    void drawCrossMarker(int cx, int cy, Color c, int arm = 1) {
        putPoint(cx, cy, c);
        for (int k = 1; k <= arm; ++k) {
            putPoint(cx - k, cy, c);
            putPoint(cx + k, cy, c);
            putPoint(cx, cy - k, c);
            putPoint(cx, cy + k, c);
        }
    }

    // Marcador cuadrado (2*halfSize+1)x(2*halfSize+1) centrado en (cx, cy) usando putPoint()
    void drawSquareMarker(int cx, int cy, Color c, int halfSize = 1, bool hollowCenter = false) {
        for (int dy = -halfSize; dy <= halfSize; ++dy) {
            for (int dx = -halfSize; dx <= halfSize; ++dx) {
                if (hollowCenter && dx == 0 && dy == 0) {
                    putPoint(cx, cy, COL_BG);
                } else {
                    putPoint(cx + dx, cy + dy, c);
                }
            }
        }
    }

    // Algoritmo de Bresenham en los 8 octantes con soporte de patron (guiones) y grosor (1 o 2 px)
    // mediante vecino perpendicular, 100% usando putPoint().
    void drawPatternLine(int x0, int y0, int x1, int y1, Color c,
                         int onSteps, int offSteps, int thickness = 1) {
        const int dx = std::abs(x1 - x0);
        const int dy = std::abs(y1 - y0);
        const int sx = (x0 < x1) ? 1 : -1;
        const int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        int step = 0;
        const int cycle = (onSteps > 0 && offSteps > 0) ? (onSteps + offSteps) : 0;

        while (true) {
            const bool paint = (cycle == 0) || ((step % cycle) < onSteps);
            if (paint) {
                putPoint(x0, y0, c);
                if (thickness >= 2) {
                    if (dx >= dy) {
                        putPoint(x0, y0 + 1, c);
                    } else {
                        putPoint(x0 + 1, y0, c);
                    }
                }
            }
            if (x0 == x1 && y0 == y1) break;
            const int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 <  dx) { err += dx; y0 += sy; }
            ++step;
        }
    }

    // Interfaz estandar de Bresenham:
    //   stepStride <= 1: trazo continuo
    //   stepStride == 2: trazo guionado corto (4px on / 2px off)
    //   stepStride >= 3: trazo discontinuo largo (6px on / 4px off)
    void drawLine(int x0, int y0, int x1, int y1, Color c = COL_NEGRO,
                  int stepStride = 1, int thickness = 1) {
        if (stepStride <= 1) {
            drawPatternLine(x0, y0, x1, y1, c, 1, 0, thickness);
        } else if (stepStride == 2) {
            drawPatternLine(x0, y0, x1, y1, c, 4, 2, thickness);
        } else {
            drawPatternLine(x0, y0, x1, y1, c, 6, 4, thickness);
        }
    }

    void drawWorldSegment(const Point& a, const Point& b, Color c = COL_NEGRO,
                          int stepStride = 1, int thickness = 1) {
        drawLine(worldToGridX(a.x), worldToGridY(a.y),
                 worldToGridX(b.x), worldToGridY(b.y), c, stepStride, thickness);
    }

    void drawPolygon(const std::vector<Point>& poly, Color c = COL_NEGRO,
                     int stepStride = 1, int thickness = 1) {
        if (poly.size() < 2) return;
        for (std::size_t i = 0; i < poly.size(); ++i) {
            const Point& u = poly[i];
            const Point& v = poly[(i + 1) % poly.size()];
            drawWorldSegment(u, v, c, stepStride, thickness);
        }
    }

    // Ejes cartesianos delgados (1 px) en gris neutro con marcas cada 5 unidades
    void drawAxes() {
        drawLine(0, ORIGIN_Y, width_ - 1, ORIGIN_Y, COL_AXES, 1, 1);
        drawLine(ORIGIN_X, 0, ORIGIN_X, height_ - 1, COL_AXES, 1, 1);

        for (int u = -15; u <= 15; u += 5) {
            if (u == 0) continue;
            const int gx = worldToGridX(static_cast<double>(u));
            for (int d = -2; d <= 2; ++d) {
                putPoint(gx, ORIGIN_Y + d, COL_AXES_TICK);
            }
        }
        for (int u = -10; u <= 10; u += 5) {
            if (u == 0) continue;
            const int gy = worldToGridY(static_cast<double>(u));
            for (int d = -2; d <= 2; ++d) {
                putPoint(ORIGIN_X + d, gy, COL_AXES_TICK);
            }
        }
        drawCrossMarker(ORIGIN_X, ORIGIN_Y, COL_AXES_TICK, 1);
    }

    // Cuadricula local transformable usando exclusivamente transformPoint() + Bresenham 1 px
    void drawTransformedLattice(const Matrix3& M, int xMin, int xMax, int yMin, int yMax,
                                int step, Color c, int stepStride = 1) {
        for (int x = xMin; x <= xMax; x += step) {
            const Point a = transformPoint(M, {static_cast<double>(x), static_cast<double>(yMin)});
            const Point b = transformPoint(M, {static_cast<double>(x), static_cast<double>(yMax)});
            drawWorldSegment(a, b, c, stepStride, 1);
        }
        for (int y = yMin; y <= yMax; y += step) {
            const Point a = transformPoint(M, {static_cast<double>(xMin), static_cast<double>(y)});
            const Point b = transformPoint(M, {static_cast<double>(xMax), static_cast<double>(y)});
            drawWorldSegment(a, b, c, stepStride, 1);
        }
    }

    // Exporta imagen PPM 1280 x 960 fiel a la ventana OpenGL:
    // dibuja sub-rejilla de pixeles, rejilla por unidad matematica y celdas ocupadas.
    bool savePPM(const std::string& path, bool showGrid = true, int cellPx = CELL_PX) const {
        const int imgW = width_ * cellPx;
        const int imgH = height_ * cellPx;
        std::vector<unsigned char> img(static_cast<std::size_t>(imgW * imgH * 3), 255);

        auto toByte = [](float v) -> unsigned char {
            return static_cast<unsigned char>(std::lround(std::clamp(v, 0.0f, 1.0f) * 255.0f));
        };

        auto setImgPx = [&](int ix, int iy, Color c) {
            if (ix < 0 || ix >= imgW || iy < 0 || iy >= imgH) return;
            const std::size_t off = static_cast<std::size_t>((iy * imgW + ix) * 3);
            img[off]     = toByte(c.r);
            img[off + 1] = toByte(c.g);
            img[off + 2] = toByte(c.b);
        };

        // 1. Rejilla de fondo: sub-rejilla discreta y rejilla por unidad matematica
        if (showGrid) {
            for (int gx = 0; gx <= width_; ++gx) {
                const int px = std::min(imgW - 1, gx * cellPx);
                const Color c = (gx % PX_PER_UNIT == 0) ? COL_GRID_LINE : COL_GRID_SUB;
                for (int py = 0; py < imgH; ++py) setImgPx(px, py, c);
            }
            for (int gy = 0; gy <= height_; ++gy) {
                const int py = std::min(imgH - 1, gy * cellPx);
                const Color c = (gy % PX_PER_UNIT == 0) ? COL_GRID_LINE : COL_GRID_SUB;
                for (int px = 0; px < imgW; ++px) setImgPx(px, py, c);
            }
        }

        // 2. Celdas logicas ocupadas (conservando 1 px de borde de rejilla para ver cada pixel)
        const int inset = (showGrid && cellPx >= 4) ? 1 : 0;
        for (int gy = 0; gy < height_; ++gy) {
            for (int gx = 0; gx < width_; ++gx) {
                const std::size_t idx = static_cast<std::size_t>(gy * width_ + gx);
                if (!occupied_[idx]) continue;
                const Color c = pixels_[idx];
                const int xStart = gx * cellPx + inset;
                const int xEnd   = (gx + 1) * cellPx;
                const int yStart = (height_ - 1 - gy) * cellPx + inset;
                const int yEnd   = (height_ - gy) * cellPx;
                for (int py = yStart; py < yEnd; ++py) {
                    for (int px = xStart; px < xEnd; ++px) {
                        setImgPx(px, py, c);
                    }
                }
            }
        }

        std::ofstream out(path, std::ios::binary);
        if (!out) return false;
        out << "P6\n" << imgW << " " << imgH << "\n255\n";
        out.write(reinterpret_cast<const char*>(img.data()),
                  static_cast<std::streamsize>(img.size()));
        return true;
    }

private:
    int width_;
    int height_;
    std::vector<Color> pixels_;
    std::vector<bool> occupied_;
};

}  // namespace cg
