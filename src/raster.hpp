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
// Rejilla discreta 40 x 30 sobre ventana 1000 x 750 (25 x 25 px por celda).
// Cada celda logica (gx, gy) se dibuja con margen m = 0.08 para distinguir
// visualmente los pixeles sobre la rejilla drawGrid().
//
// Sistema de coordenadas:
//   - Origen cartesiano (0, 0) ubicado en el centro de la rejilla: (20, 15).
//   - Rango cartesiano visible: x in [-20, 19], y in [-15, 14].
//   - Eje Y crece hacia arriba en glOrtho(0, 40, 0, 30, -1, 1).
//   - Al exportar imagenes PPM (fila 0 arriba), se invierte verticalmente.
// ============================================================================

namespace cg {

struct Color {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
};

inline constexpr int WINDOW_WIDTH  = 1000;
inline constexpr int WINDOW_HEIGHT = 750;
inline constexpr int GRID_WIDTH    = 40;
inline constexpr int GRID_HEIGHT   = 30;
inline constexpr int ORIGIN_X      = 20;
inline constexpr int ORIGIN_Y      = 15;
inline constexpr float CELL_MARGIN = 0.08f;

// Paleta fiel al examen de laboratorio y al codigo base del equipo
inline constexpr Color COL_BG           = {1.00f, 1.00f, 1.00f};
inline constexpr Color COL_GRID_LINE    = {0.87f, 0.88f, 0.90f};
inline constexpr Color COL_AXES         = {0.58f, 0.62f, 0.68f};
inline constexpr Color COL_TGRID_ORIG   = {0.82f, 0.85f, 0.90f};
inline constexpr Color COL_TGRID_TRANS  = {0.62f, 0.84f, 0.80f};
inline constexpr Color COL_NEGRO        = {0.14f, 0.20f, 0.28f};
inline constexpr Color COL_AZUL         = {0.10f, 0.34f, 0.82f};
inline constexpr Color COL_NARANJA      = {0.93f, 0.42f, 0.07f};
inline constexpr Color COL_VERDE        = {0.05f, 0.58f, 0.38f};
inline constexpr Color COL_GRIS         = {0.48f, 0.52f, 0.58f};
inline constexpr Color COL_ROJO         = {0.84f, 0.14f, 0.18f};
inline constexpr Color COL_MORADO       = {0.58f, 0.22f, 0.72f};

// Lienzo discreto 40 x 30: toda geometria transformada se rasteriza
// exclusivamente con putPoint() y Bresenham (sin GL_LINES para figuras).
// Garantiza exactamente la misma salida en OpenGL y en capturas PPM.
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

    // Conversion coordenada cartesiana (centrada en 0,0) -> indice de celda de rejilla
    int worldToGridX(double wx) const {
        return ORIGIN_X + static_cast<int>(std::lround(wx));
    }

    int worldToGridY(double wy) const {
        return ORIGIN_Y + static_cast<int>(std::lround(wy));
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

    // Algoritmo de Bresenham para todos los octantes (stepStride=1 solido, >1 punteado)
    void drawLine(int x0, int y0, int x1, int y1, Color c = COL_NEGRO, int stepStride = 1) {
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        int step = 0;
        while (true) {
            if (stepStride <= 1 || (step % stepStride == 0)) {
                putPoint(x0, y0, c);
            }
            if (x0 == x1 && y0 == y1) break;
            const int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 <  dx) { err += dx; y0 += sy; }
            ++step;
        }
    }

    void drawWorldSegment(const Point& a, const Point& b, Color c = COL_NEGRO, int stepStride = 1) {
        drawLine(worldToGridX(a.x), worldToGridY(a.y),
                 worldToGridX(b.x), worldToGridY(b.y), c, stepStride);
    }

    void drawPolygon(const std::vector<Point>& poly, Color c = COL_NEGRO) {
        if (poly.size() < 2) return;
        for (std::size_t i = 0; i < poly.size(); ++i) {
            const Point& u = poly[i];
            const Point& v = poly[(i + 1) % poly.size()];
            drawWorldSegment(u, v, c, 1);
        }
    }

    void drawAxes() {
        drawLine(0, ORIGIN_Y, width_ - 1, ORIGIN_Y, COL_AXES, 1);
        drawLine(ORIGIN_X, 0, ORIGIN_X, height_ - 1, COL_AXES, 1);
        putPoint(ORIGIN_X, ORIGIN_Y, COL_ROJO);
    }

    // Cuadricula local transformada usando exclusivamente transformPoint() + Bresenham punteado
    void drawTransformedLattice(const Matrix3& M, int xMin, int xMax, int yMin, int yMax,
                                int step, Color c, int dotStride = 2) {
        for (int x = xMin; x <= xMax; x += step) {
            const Point a = transformPoint(M, {static_cast<double>(x), static_cast<double>(yMin)});
            const Point b = transformPoint(M, {static_cast<double>(x), static_cast<double>(yMax)});
            drawWorldSegment(a, b, c, dotStride);
        }
        for (int y = yMin; y <= yMax; y += step) {
            const Point a = transformPoint(M, {static_cast<double>(xMin), static_cast<double>(y)});
            const Point b = transformPoint(M, {static_cast<double>(xMax), static_cast<double>(y)});
            drawWorldSegment(a, b, c, dotStride);
        }
    }

    // Exporta imagen PPM 1000 x 750 fiel a la ventana OpenGL:
    // dibuja la rejilla de pixeles (si showGrid = true) y cada celda ocupada con margen m = 0.08.
    bool savePPM(const std::string& path, bool showGrid = true, int cellPx = 25) const {
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

        // 1. Rejilla de pixeles drawGrid()
        if (showGrid) {
            for (int gx = 0; gx <= width_; ++gx) {
                const int px = std::min(imgW - 1, gx * cellPx);
                for (int py = 0; py < imgH; ++py) setImgPx(px, py, COL_GRID_LINE);
            }
            for (int gy = 0; gy <= height_; ++gy) {
                const int py = std::min(imgH - 1, gy * cellPx);
                for (int px = 0; px < imgW; ++px) setImgPx(px, py, COL_GRID_LINE);
            }
        }

        // 2. Celdas logicas ocupadas con margen m = 0.08 (inversion vertical explicita)
        const int pad = std::max(1, static_cast<int>(std::lround(CELL_MARGIN * cellPx)));
        for (int gy = 0; gy < height_; ++gy) {
            for (int gx = 0; gx < width_; ++gx) {
                const std::size_t idx = static_cast<std::size_t>(gy * width_ + gx);
                if (!occupied_[idx]) continue;
                const Color c = pixels_[idx];
                const int xStart = gx * cellPx + pad;
                const int xEnd   = (gx + 1) * cellPx - pad;
                const int yStart = (height_ - 1 - gy) * cellPx + pad;
                const int yEnd   = (height_ - gy) * cellPx - pad;
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
