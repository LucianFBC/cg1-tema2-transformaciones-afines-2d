#pragma once

#include "math_affine.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// ============================================================================
// Modulo de rasterizacion propia (Tema 2 - Computacion Grafica I)
//
// Reutiliza el enfoque de los laboratorios del curso:
//   - putPixel() / putPoint() sobre una rejilla discreta de celdas logicas.
//   - drawLine() mediante algoritmo de Bresenham en todos los octantes.
//   - drawPolygon() trazando aristas con drawLine().
//
// Sistema de coordenadas logico:
//   - Espacio continuo de mundo: x in [-14, 14], y in [-10.5, 10.5]
//   - Rejilla discreta de rasterizacion: 280 x 210 celdas (10 celdas / unidad)
//   - Orientacion del eje vertical: Y CRECE HACIA ARRIBA (tanto en mundo como
//     en el lienzo glOrtho(0, GRID_W, 0, GRID_H, -1, 1)).
//   - Al exportar a imagen PPM (donde la fila 0 es el borde superior), se aplica
//     la correccion explicita: row_ppm = (GRID_H - 1) - py.
// ============================================================================

namespace cg {

struct Color {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
};

// Paleta de alto contraste y lectura clara en pantalla e informe
inline constexpr Color COL_BG           = {0.98f, 0.99f, 1.00f};
inline constexpr Color COL_AXES         = {0.35f, 0.40f, 0.48f};
inline constexpr Color COL_GRID_ORIG    = {0.84f, 0.88f, 0.93f};
inline constexpr Color COL_GRID_TRANS   = {0.65f, 0.85f, 0.82f};
inline constexpr Color COL_POLY_ORIG    = {0.16f, 0.22f, 0.30f};
inline constexpr Color COL_POLY_TRANS   = {0.06f, 0.36f, 0.82f};
inline constexpr Color COL_POLY_ALT     = {0.90f, 0.36f, 0.05f};
inline constexpr Color COL_SEG_ORIG     = {0.42f, 0.48f, 0.56f};
inline constexpr Color COL_SEG_TRANS    = {0.05f, 0.62f, 0.45f};
inline constexpr Color COL_SEG_ALT      = {0.75f, 0.16f, 0.48f};
inline constexpr Color COL_VERTEX_ORIG  = {0.20f, 0.24f, 0.32f};
inline constexpr Color COL_VERTEX_TRANS = {0.86f, 0.14f, 0.18f};
inline constexpr Color COL_REF_LINE     = {0.60f, 0.20f, 0.72f};

inline constexpr int GRID_WIDTH  = 280;
inline constexpr int GRID_HEIGHT = 210;
inline constexpr double CELLS_PER_UNIT = 10.0;
inline constexpr double WORLD_X_MIN = -14.0;
inline constexpr double WORLD_X_MAX =  14.0;
inline constexpr double WORLD_Y_MIN = -10.5;
inline constexpr double WORLD_Y_MAX =  10.5;

enum class LineStyle {
    Solid,
    Dashed,
    Dotted
};

// Buffer de rasterizacion propio: cada celda logica (px, py) almacena un color.
// Permite exactamente la misma salida tanto en ventana OpenGL (via putPoint)
// como en exportacion reproducible PPM sin depender de capturas externas.
class RasterCanvas {
public:
    RasterCanvas(int width = GRID_WIDTH, int height = GRID_HEIGHT)
        : width_(width), height_(height), pixels_(static_cast<std::size_t>(width * height), COL_BG) {}

    int width() const { return width_; }
    int height() const { return height_; }
    const std::vector<Color>& pixels() const { return pixels_; }

    void clear(Color bg = COL_BG) {
        std::fill(pixels_.begin(), pixels_.end(), bg);
    }

    // Conversion de coordenada continua de mundo -> celda discreta de raster
    int worldToRasterX(double wx) const {
        return static_cast<int>(std::lround((wx - WORLD_X_MIN) * CELLS_PER_UNIT));
    }

    int worldToRasterY(double wy) const {
        return static_cast<int>(std::lround((wy - WORLD_Y_MIN) * CELLS_PER_UNIT));
    }

    // Rutina fundamental del curso: dibuja una celda logica individual
    void putPixel(int x, int y, float r, float g, float b) {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
        pixels_[static_cast<std::size_t>(y * width_ + x)] = {r, g, b};
    }

    // Alias de laboratorio: putPoint()
    void putPoint(int x, int y, Color c) {
        putPixel(x, y, c.r, c.g, c.b);
    }

    // Dibuja un punto reforzado (radio en celdas) para resaltar vertices o grosor
    void putThickPoint(int cx, int cy, Color c, int radius = 1) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                putPoint(cx + dx, cy + dy, c);
            }
        }
    }

    // Algoritmo de Bresenham para todos los octantes (con soporte de estilo y grosor)
    void drawLine(int x0, int y0, int x1, int y1, Color c,
                  LineStyle style = LineStyle::Solid, int thickness = 0) {
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        int step = 0;

        while (true) {
            bool draw = true;
            if (style == LineStyle::Dashed) {
                draw = ((step % 8) < 5);
            } else if (style == LineStyle::Dotted) {
                draw = ((step % 4) == 0);
            }
            if (draw) {
                if (thickness <= 0) {
                    putPoint(x0, y0, c);
                } else {
                    putThickPoint(x0, y0, c, thickness);
                }
            }
            if (x0 == x1 && y0 == y1) break;
            const int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
            ++step;
        }
    }

    void drawWorldLine(const Point& a, const Point& b, Color c,
                       LineStyle style = LineStyle::Solid, int thickness = 0) {
        drawLine(worldToRasterX(a.x), worldToRasterY(a.y),
                 worldToRasterX(b.x), worldToRasterY(b.y), c, style, thickness);
    }

    void drawPolygon(const std::vector<Point>& poly, Color c,
                     LineStyle style = LineStyle::Solid, int thickness = 0) {
        if (poly.size() < 2) return;
        for (std::size_t i = 0; i < poly.size(); ++i) {
            const Point& u = poly[i];
            const Point& v = poly[(i + 1) % poly.size()];
            drawWorldLine(u, v, c, style, thickness);
        }
    }

    void drawVertexMarkers(const std::vector<Point>& poly, Color c, int radius = 2) {
        for (const auto& v : poly) {
            putThickPoint(worldToRasterX(v.x), worldToRasterY(v.y), c, radius);
        }
    }

    // Dibuja los ejes cartesianos X=0, Y=0 y marcas de unidad mediante Bresenham
    void drawCartesianAxes() {
        drawWorldLine({WORLD_X_MIN, 0.0}, {WORLD_X_MAX, 0.0}, COL_AXES, LineStyle::Solid, 0);
        drawWorldLine({0.0, WORLD_Y_MIN}, {0.0, WORLD_Y_MAX}, COL_AXES, LineStyle::Solid, 0);

        const int ox = worldToRasterX(0.0);
        const int oy = worldToRasterY(0.0);
        for (int x = static_cast<int>(WORLD_X_MIN); x <= static_cast<int>(WORLD_X_MAX); ++x) {
            const int px = worldToRasterX(static_cast<double>(x));
            drawLine(px, oy - 2, px, oy + 2, COL_AXES);
        }
        for (int y = static_cast<int>(WORLD_Y_MIN); y <= static_cast<int>(WORLD_Y_MAX); ++y) {
            const int py = worldToRasterY(static_cast<double>(y));
            drawLine(ox - 2, py, ox + 2, py, COL_AXES);
        }
        // Flechas de orientacion positiva (+X derecha, +Y arriba)
        drawLine(width_ - 2, oy, width_ - 7, oy + 3, COL_AXES);
        drawLine(width_ - 2, oy, width_ - 7, oy - 3, COL_AXES);
        drawLine(ox, height_ - 2, ox - 3, height_ - 7, COL_AXES);
        drawLine(ox, height_ - 2, ox + 3, height_ - 7, COL_AXES);
    }

    // Dibuja una cuadricula local transformada por M (usando segmentos de recta)
    void drawTransformedGrid(const Matrix3& M, int xMin, int xMax, int yMin, int yMax,
                             Color c, LineStyle style = LineStyle::Solid) {
        for (int x = xMin; x <= xMax; ++x) {
            const Point a = transformPoint(M, {static_cast<double>(x), static_cast<double>(yMin)});
            const Point b = transformPoint(M, {static_cast<double>(x), static_cast<double>(yMax)});
            drawWorldLine(a, b, c, style, 0);
        }
        for (int y = yMin; y <= yMax; ++y) {
            const Point a = transformPoint(M, {static_cast<double>(xMin), static_cast<double>(y)});
            const Point b = transformPoint(M, {static_cast<double>(xMax), static_cast<double>(y)});
            drawWorldLine(a, b, c, style, 0);
        }
    }

    // Exporta el raster actual a formato PPM (escala entera para visualizacion nitida).
    // Aplica inversion vertical explicita: y_ppm = (height - 1) - y_mundo.
    bool savePPM(const std::string& path, int pixelScale = 3) const {
        std::ofstream out(path, std::ios::binary);
        if (!out) return false;
        const int outW = width_ * pixelScale;
        const int outH = height_ * pixelScale;
        out << "P6\n" << outW << " " << outH << "\n255\n";

        auto toByte = [](float v) -> unsigned char {
            const int iv = static_cast<int>(std::lround(std::clamp(v, 0.0f, 1.0f) * 255.0f));
            return static_cast<unsigned char>(iv);
        };

        for (int y = height_ - 1; y >= 0; --y) {
            for (int sy = 0; sy < pixelScale; ++sy) {
                for (int x = 0; x < width_; ++x) {
                    const Color& c = pixels_[static_cast<std::size_t>(y * width_ + x)];
                    const unsigned char rgb[3] = {toByte(c.r), toByte(c.g), toByte(c.b)};
                    for (int sx = 0; sx < pixelScale; ++sx) {
                        out.write(reinterpret_cast<const char*>(rgb), 3);
                    }
                }
            }
        }
        return true;
    }

private:
    int width_;
    int height_;
    std::vector<Color> pixels_;
};

}  // namespace cg
