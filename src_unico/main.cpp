#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
// Laboratorio Interactivo de Transformaciones Afines 2D (Archivo Unico)
// UNSAAC - Computacion Grafica I (IF455AIN) - Semestre 2026-02
// Segunda Parcial - Tema 2: Transformaciones afines, deformacion y propiedades
//
// Convencion matematica obligatoria:
//   - Vectores columna homogeneos: p = [x, y, 1]^T
//   - Transformacion: p' = M * p
//   - Composicion: en M = A * B * C, se aplica primero C, luego B y al final A.
//
// Entorno de rasterizacion propia:
//   - Rejilla discreta 40 x 30 sobre ventana 1000 x 750 (25 x 25 px por celda).
//   - Cada celda logica se dibuja con margen m = 0.08 sobre drawGrid() (tecla G).
//   - Trazado 100% mediante putPixel()/putPoint(), Bresenham (drawLine) y
//     drawPolygon(), sin glTranslatef/glRotatef/glScalef ni GL_POLYGON.
//   - GL_LINES se usa exclusivamente en drawGrid().
// ============================================================================

namespace cg {

constexpr double PI = 3.14159265358979323846;
constexpr double EPSILON = 1e-9;

struct Point {
    double x{0.0};
    double y{0.0};
};

struct Segment {
    Point p;
    Point q;
    std::string label;
};

struct Matrix3 {
    double a[3][3] = {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    };
};

struct NamedTransform {
    std::string name;
    Matrix3 matrix;
};

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

// ---------------- Matrices elementales 3x3 ----------------

inline Matrix3 identityMatrix() {
    return Matrix3{{
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    }};
}

inline Matrix3 translationMatrix(double tx, double ty) {
    return Matrix3{{
        {1.0, 0.0, tx},
        {0.0, 1.0, ty},
        {0.0, 0.0, 1.0}
    }};
}

inline Matrix3 rotationMatrix(double degrees) {
    const double rad = degrees * (PI / 180.0);
    const double c = std::cos(rad);
    const double s = std::sin(rad);
    return Matrix3{{
        { c,  -s,  0.0},
        { s,   c,  0.0},
        {0.0, 0.0, 1.0}
    }};
}

inline Matrix3 scalingMatrix(double sx, double sy) {
    return Matrix3{{
        {sx,  0.0, 0.0},
        {0.0, sy,  0.0},
        {0.0, 0.0, 1.0}
    }};
}

inline Matrix3 shearXMatrix(double k) {
    return Matrix3{{
        {1.0,  k,  0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    }};
}

inline Matrix3 shearYMatrix(double k) {
    return Matrix3{{
        {1.0, 0.0, 0.0},
        { k,  1.0, 0.0},
        {0.0, 0.0, 1.0}
    }};
}

inline Matrix3 reflectionXMatrix() {
    return Matrix3{{
        {1.0,  0.0, 0.0},
        {0.0, -1.0, 0.0},
        {0.0,  0.0, 1.0}
    }};
}

inline Matrix3 reflectionYMatrix() {
    return Matrix3{{
        {-1.0, 0.0, 0.0},
        { 0.0, 1.0, 0.0},
        { 0.0, 0.0, 1.0}
    }};
}

inline Matrix3 multiplyMatrix(const Matrix3& left, const Matrix3& right) {
    Matrix3 res{};
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            double sum = 0.0;
            for (int k = 0; k < 3; ++k) {
                sum += left.a[r][k] * right.a[k][c];
            }
            res.a[r][c] = sum;
        }
    }
    return res;
}

// Reflexion respecto a una recta por el origen con angulo phi (grados):
// Ref_phi = R(phi) * Ref_x * R(-phi)
inline Matrix3 reflectionLineMatrix(double phiDegrees) {
    const Matrix3 rNeg = rotationMatrix(-phiDegrees);
    const Matrix3 refX = reflectionXMatrix();
    const Matrix3 rPos = rotationMatrix(phiDegrees);
    return multiplyMatrix(rPos, multiplyMatrix(refX, rNeg));
}

// Aplica una cadena en orden cronologico:
// steps = [T1, T2, ..., Tn] => M = Tn * ... * T2 * T1
inline Matrix3 composeInApplicationOrder(const std::vector<NamedTransform>& steps) {
    Matrix3 M = identityMatrix();
    for (const auto& step : steps) {
        M = multiplyMatrix(step.matrix, M);
    }
    return M;
}

inline Point transformPoint(const Matrix3& M, const Point& p) {
    const double x = M.a[0][0] * p.x + M.a[0][1] * p.y + M.a[0][2];
    const double y = M.a[1][0] * p.x + M.a[1][1] * p.y + M.a[1][2];
    const double w = M.a[2][0] * p.x + M.a[2][1] * p.y + M.a[2][2];
    if (std::abs(w) < EPSILON) {
        throw std::domain_error("Coordenada homogenea w = 0: punto en el infinito.");
    }
    return {x / w, y / w};
}

inline std::vector<Point> transformPolygon(const Matrix3& M, const std::vector<Point>& poly) {
    std::vector<Point> out;
    out.reserve(poly.size());
    for (const auto& p : poly) {
        out.push_back(transformPoint(M, p));
    }
    return out;
}

inline Segment transformSegment(const Matrix3& M, const Segment& seg) {
    return {transformPoint(M, seg.p), transformPoint(M, seg.q), seg.label + "'"};
}

// ---------------- Propiedades lineales y metricas geometricas ----------------

inline double linearDeterminant(const Matrix3& M) {
    return M.a[0][0] * M.a[1][1] - M.a[0][1] * M.a[1][0];
}

inline bool isSingular(const Matrix3& M, double tol = EPSILON) {
    return std::abs(linearDeterminant(M)) < tol;
}

// Calcula la inversa afin solo si |det(A)| >= tol.
// Nunca divide por cero ni afirma existencia de inversa para matrices singulares.
inline bool tryInvertAffine(const Matrix3& M, Matrix3& outInv, double tol = EPSILON) {
    const double det = linearDeterminant(M);
    if (std::abs(det) < tol) {
        return false;
    }
    const double invDet = 1.0 / det;
    const double a00 =  M.a[1][1] * invDet;
    const double a01 = -M.a[0][1] * invDet;
    const double a10 = -M.a[1][0] * invDet;
    const double a11 =  M.a[0][0] * invDet;
    const double tx  = M.a[0][2];
    const double ty  = M.a[1][2];
    outInv = Matrix3{{
        {a00, a01, -(a00 * tx + a01 * ty)},
        {a10, a11, -(a10 * tx + a11 * ty)},
        {0.0, 0.0, 1.0}
    }};
    return true;
}

// Formula del zapatero (shoelace): area con signo del poligono.
// > 0: recorrido antihorario (CCW), < 0: horario (CW), = 0: colineal/degenerado.
inline double signedArea(const std::vector<Point>& poly) {
    if (poly.size() < 3) return 0.0;
    double sum = 0.0;
    const std::size_t n = poly.size();
    for (std::size_t i = 0; i < n; ++i) {
        const Point& cur = poly[i];
        const Point& nxt = poly[(i + 1) % n];
        sum += (cur.x * nxt.y - nxt.x * cur.y);
    }
    return 0.5 * sum;
}

inline double absoluteArea(const std::vector<Point>& poly) {
    return std::abs(signedArea(poly));
}

inline std::string orientationLabel(double sArea, double tol = EPSILON) {
    if (std::abs(sArea) < tol) return "DEGENERADA (area nula / colineal)";
    return (sArea > 0.0) ? "ANTIHORARIA (CCW, +)" : "HORARIA (CW, -)";
}

inline double distanceBetween(const Point& a, const Point& b) {
    return std::hypot(b.x - a.x, b.y - a.y);
}

struct ParallelismMetrics {
    Point dir1;
    Point dir2;
    double len1{0.0};
    double len2{0.0};
    double cross2D{0.0};
    double normalizedSine{0.0};
    double angleDegrees{0.0};
    double lineSeparation{0.0};
    bool defined{true};
    bool parallel{false};
    bool coincident{false};
    std::string status;
};

inline ParallelismMetrics measureParallelism(const Segment& s1, const Segment& s2, double tol = EPSILON) {
    ParallelismMetrics m{};
    m.dir1 = {s1.q.x - s1.p.x, s1.q.y - s1.p.y};
    m.dir2 = {s2.q.x - s2.p.x, s2.q.y - s2.p.y};
    m.len1 = std::hypot(m.dir1.x, m.dir1.y);
    m.len2 = std::hypot(m.dir2.x, m.dir2.y);

    if (m.len1 < tol || m.len2 < tol) {
        m.defined = false;
        m.parallel = false;
        m.coincident = false;
        m.status = "INDEFINIDO (al menos un segmento colapso a un punto)";
        return m;
    }

    m.cross2D = m.dir1.x * m.dir2.y - m.dir1.y * m.dir2.x;
    const double denom = m.len1 * m.len2;
    m.normalizedSine = std::clamp(std::abs(m.cross2D) / denom, 0.0, 1.0);
    m.angleDegrees = std::asin(m.normalizedSine) * (180.0 / PI);
    m.parallel = (m.normalizedSine < 1e-7);

    const double dx = s2.p.x - s1.p.x;
    const double dy = s2.p.y - s1.p.y;
    m.lineSeparation = std::abs(m.dir1.x * dy - m.dir1.y * dx) / m.len1;
    m.coincident = m.parallel && (m.lineSeparation < 1e-7);

    if (m.coincident) {
        m.status = "COLINEALES / COINCIDENTES (colapso sobre una misma recta)";
    } else if (m.parallel) {
        m.status = "PARALELOS (rectas distintas, seno = 0)";
    } else {
        m.status = "NO PARALELOS";
    }
    return m;
}

inline double interiorAngleDegrees(const Point& prev, const Point& curr, const Point& next) {
    const Point u = {prev.x - curr.x, prev.y - curr.y};
    const Point v = {next.x - curr.x, next.y - curr.y};
    const double nu = std::hypot(u.x, u.y);
    const double nv = std::hypot(v.x, v.y);
    if (nu < EPSILON || nv < EPSILON) return 0.0;
    const double dot = std::clamp((u.x * v.x + u.y * v.y) / (nu * nv), -1.0, 1.0);
    return std::acos(dot) * (180.0 / PI);
}

// ---------------- Utilidades de formato ----------------

inline double cleanZero(double v) {
    return (std::abs(v) < 1e-12) ? 0.0 : v;
}

inline std::string formatMatrix(const Matrix3& M, int precision = 4) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision);
    for (int r = 0; r < 3; ++r) {
        oss << "  [ ";
        for (int c = 0; c < 3; ++c) {
            oss << std::setw(9) << cleanZero(M.a[r][c]);
            if (c < 2) oss << ", ";
        }
        oss << " ]\n";
    }
    return oss.str();
}

inline std::string formatPoint(const Point& p, int precision = 4) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision)
        << "(" << cleanZero(p.x) << ", " << cleanZero(p.y) << ")";
    return oss.str();
}

inline std::string formatApplicationOrder(const std::vector<NamedTransform>& steps) {
    if (steps.empty()) return "I (Identidad)";
    std::ostringstream oss;
    for (std::size_t i = 0; i < steps.size(); ++i) {
        if (i > 0) oss << " -> ";
        oss << (i + 1) << ". " << steps[i].name;
    }
    return oss.str();
}

inline std::string formatMatrixProductOrder(const std::vector<NamedTransform>& steps) {
    if (steps.empty()) return "M = I";
    std::ostringstream oss;
    oss << "M = ";
    for (std::size_t i = steps.size(); i-- > 0;) {
        oss << steps[i].name;
        if (i > 0) oss << " * ";
    }
    return oss.str();
}

// ---------------- Lienzo de rasterizacion propia 40 x 30 ----------------

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

    int worldToGridX(double wx) const {
        return ORIGIN_X + static_cast<int>(std::lround(wx));
    }

    int worldToGridY(double wy) const {
        return ORIGIN_Y + static_cast<int>(std::lround(wy));
    }

    void putPixel(int x, int y, float r, float g, float b) {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
        const std::size_t idx = static_cast<std::size_t>(y * width_ + x);
        pixels_[idx] = {r, g, b};
        occupied_[idx] = true;
    }

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

// ---------------- Escena base y evaluacion de experimentos ----------------

struct BaseScene {
    // Poligono asimetrico concavo de 6 vertices en el primer cuadrante.
    // Area exacta con signo (zapatero): +24.0 u^2 (recorrido antihorario CCW).
    std::vector<Point> polygon = {
        {1.0, 1.0},  // V0
        {7.0, 1.0},  // V1
        {7.0, 3.0},  // V2
        {4.0, 3.0},  // V3 (vertice concavo)
        {5.0, 7.0},  // V4 (vertice de referencia para comparacion de orden)
        {1.0, 6.0}   // V5
    };
    std::vector<std::string> vertexNames = {"V0", "V1", "V2", "V3", "V4", "V5"};
    std::size_t highlightVertexIndex = 4;  // V4 = (5, 7)

    // Dos segmentos originalmente paralelos con vector director u = v = (8, 4)
    Segment seg1 = {{2.0, -2.0}, {10.0,  2.0}, "L1"};
    Segment seg2 = {{2.0, -6.0}, {10.0, -2.0}, "L2"};

    // Cuadricula local transformable (paso de 3 unidades)
    int latticeXMin = 0;
    int latticeXMax = 9;
    int latticeYMin = -6;
    int latticeYMax = 9;
    int latticeStep = 3;
};

struct ExperimentConfig {
    std::string id;
    std::string title;
    std::string description;
    std::vector<NamedTransform> steps;
    bool hasSecondBranch{false};
    std::vector<NamedTransform> secondSteps;
    bool drawReflectionAxis{false};
    double reflectionAxisDeg{0.0};
};

struct TransformReport {
    Matrix3 M;
    double detA{1.0};
    bool singular{false};
    bool invertible{true};
    double origSignedArea{0.0};
    double origAbsArea{0.0};
    double transSignedArea{0.0};
    double transAbsArea{0.0};
    double expectedAbsArea{0.0};
    double areaAbsError{0.0};
    std::string origOrientation;
    std::string transOrientation;
    std::vector<Point> transPoly;
    Segment transSeg1;
    Segment transSeg2;
    ParallelismMetrics origParallel;
    ParallelismMetrics transParallel;
    double origAngleAtV0{0.0};
    double transAngleAtV0{0.0};
    double origEdgeV0V1{0.0};
    double transEdgeV0V1{0.0};
};

inline TransformReport evaluateTransform(const BaseScene& scene, const Matrix3& M) {
    TransformReport r{};
    r.M = M;
    r.detA = linearDeterminant(M);
    r.singular = isSingular(M);
    r.invertible = !r.singular;

    r.origSignedArea = signedArea(scene.polygon);
    r.origAbsArea = std::abs(r.origSignedArea);
    r.origOrientation = orientationLabel(r.origSignedArea);

    r.transPoly = transformPolygon(M, scene.polygon);
    r.transSignedArea = signedArea(r.transPoly);
    r.transAbsArea = std::abs(r.transSignedArea);
    r.transOrientation = orientationLabel(r.transSignedArea);

    r.expectedAbsArea = std::abs(r.detA) * r.origAbsArea;
    r.areaAbsError = std::abs(r.transAbsArea - r.expectedAbsArea);

    r.transSeg1 = transformSegment(M, scene.seg1);
    r.transSeg2 = transformSegment(M, scene.seg2);

    r.origParallel = measureParallelism(scene.seg1, scene.seg2);
    r.transParallel = measureParallelism(r.transSeg1, r.transSeg2);

    r.origAngleAtV0 = interiorAngleDegrees(scene.polygon[5], scene.polygon[0], scene.polygon[1]);
    r.transAngleAtV0 = interiorAngleDegrees(r.transPoly[5], r.transPoly[0], r.transPoly[1]);
    r.origEdgeV0V1 = distanceBetween(scene.polygon[0], scene.polygon[1]);
    r.transEdgeV0V1 = distanceBetween(r.transPoly[0], r.transPoly[1]);
    return r;
}

inline std::vector<ExperimentConfig> buildPresetExperiments() {
    return {
        {
            "BASE",
            "Escena base (Identidad)",
            "Poligono asimetrico de 6 vertices (area +24 u^2), cuadricula y segmentos paralelos L1, L2.",
            {}, false, {}, false, 0.0
        },
        {
            "SHEAR_X",
            "Cizallamiento horizontal Hx(0.7)",
            "Verifica conservacion de paralelismo y area (det = 1) con deformacion de angulos y longitudes.",
            {{"Hx(0.7)", shearXMatrix(0.7)}}, false, {}, false, 0.0
        },
        {
            "REF_Y",
            "Reflexion respecto al eje vertical (Ref_y)",
            "Refleja x -> -x; invierte area con signo (+24 -> -24) y orientacion (CCW -> CW) conservando |area|.",
            {{"Ref_y", reflectionYMatrix()}}, false, {}, false, 0.0
        },
        {
            "ORDEN_RH",
            "Composicion R(45) * Hx(0.7)",
            "Aplica primero cizallamiento Hx(0.7) y luego rotacion R(45).",
            {{"Hx(0.7)", shearXMatrix(0.7)}, {"R(45)", rotationMatrix(45.0)}}, false, {}, false, 0.0
        },
        {
            "ORDEN_HR",
            "Composicion Hx(0.7) * R(45)",
            "Aplica primero rotacion R(45) y luego cizallamiento Hx(0.7).",
            {{"R(45)", rotationMatrix(45.0)}, {"Hx(0.7)", shearXMatrix(0.7)}}, false, {}, false, 0.0
        },
        {
            "ORDEN_COMP",
            "Comparacion de orden: R(45)*Hx(0.7) vs Hx(0.7)*R(45)",
            "Superpone ambos ordenes (azul: R*Hx, naranja: Hx*R) y cuantifica la brecha en V4 = (5, 7).",
            {{"Hx(0.7)", shearXMatrix(0.7)}, {"R(45)", rotationMatrix(45.0)}},
            true,
            {{"R(45)", rotationMatrix(45.0)}, {"Hx(0.7)", shearXMatrix(0.7)}},
            false, 0.0
        },
        {
            "AREA_COMP",
            "Cambio de area: T(2,-2) * R(25) * S(1.5,0.8) * Hx(0.5)",
            "Composicion de 4 transformaciones con |det(A)| = 1.20; contrasta area calculada vs |det(A)| * Area_orig.",
            {
                {"Hx(0.5)", shearXMatrix(0.5)},
                {"S(1.5, 0.8)", scalingMatrix(1.5, 0.8)},
                {"R(25)", rotationMatrix(25.0)},
                {"T(2.0, -2.0)", translationMatrix(2.0, -2.0)}
            },
            false, {}, false, 0.0
        },
        {
            "SINGULAR",
            "Caso degenerado / matriz singular S(1.0, 0.0)",
            "Escalamiento con factor vertical nulo (det = 0): colapso sobre y = 0, area 0 y sin inversa.",
            {{"S(1.0, 0.0)", scalingMatrix(1.0, 0.0)}}, false, {}, false, 0.0
        },
        {
            "REF_30",
            "Reflexion sobre recta oblicua (phi = 30 grados)",
            "Verifica Ref_30 = R(30) * Ref_x * R(-30) respecto al eje y = tan(30)*x.",
            {
                {"R(-30)", rotationMatrix(-30.0)},
                {"Ref_x", reflectionXMatrix()},
                {"R(30)", rotationMatrix(30.0)}
            },
            false, {}, true, 30.0
        }
    };
}

inline void renderExperimentToCanvas(RasterCanvas& canvas, const BaseScene& scene,
                                     const ExperimentConfig& exp,
                                     bool showOriginal, bool showTransformedLattice) {
    canvas.clear(COL_BG);

    const Matrix3 I = identityMatrix();
    const Matrix3 M1 = composeInApplicationOrder(exp.steps);

    // 1. Cuadricula local transformable punteada
    if (showTransformedLattice) {
        if (showOriginal) {
            canvas.drawTransformedLattice(I, scene.latticeXMin, scene.latticeXMax,
                                          scene.latticeYMin, scene.latticeYMax,
                                          scene.latticeStep, COL_TGRID_ORIG, 2);
        }
        if (!exp.steps.empty()) {
            canvas.drawTransformedLattice(M1, scene.latticeXMin, scene.latticeXMax,
                                          scene.latticeYMin, scene.latticeYMax,
                                          scene.latticeStep, COL_TGRID_TRANS, 2);
        }
    }

    // 2. Ejes cartesianos centrados en (20, 15) mediante Bresenham
    canvas.drawAxes();

    // 3. Recta oblicua de reflexion mediante Bresenham punteado (si aplica)
    if (exp.drawReflectionAxis) {
        const double rad = exp.reflectionAxisDeg * (PI / 180.0);
        const double L = 22.0;
        canvas.drawWorldSegment({-L * std::cos(rad), -L * std::sin(rad)},
                                { L * std::cos(rad),  L * std::sin(rad)},
                                COL_MORADO, 2);
    }

    // 4. Figura y segmentos originales (negro y gris)
    if (showOriginal) {
        canvas.drawWorldSegment(scene.seg1.p, scene.seg1.q, COL_GRIS);
        canvas.drawWorldSegment(scene.seg2.p, scene.seg2.q, COL_GRIS);
        canvas.drawPolygon(scene.polygon, COL_NEGRO);
        for (const auto& v : scene.polygon) {
            canvas.putPoint(canvas.worldToGridX(v.x), canvas.worldToGridY(v.y), COL_NEGRO);
        }
    }

    // 5. Figura y segmentos transformados (azul, verde y rojo en vertices)
    if (!exp.steps.empty()) {
        const auto poly1 = transformPolygon(M1, scene.polygon);
        const auto s1 = transformSegment(M1, scene.seg1);
        const auto s2 = transformSegment(M1, scene.seg2);

        canvas.drawWorldSegment(s1.p, s1.q, COL_VERDE);
        canvas.drawWorldSegment(s2.p, s2.q, COL_VERDE);
        canvas.drawPolygon(poly1, COL_AZUL);
        for (const auto& v : poly1) {
            canvas.putPoint(canvas.worldToGridX(v.x), canvas.worldToGridY(v.y), COL_ROJO);
        }
    }

    // 6. Segunda rama para comparacion simultanea de orden (naranja y morado)
    if (exp.hasSecondBranch) {
        const Matrix3 M2 = composeInApplicationOrder(exp.secondSteps);
        const auto poly2 = transformPolygon(M2, scene.polygon);
        const auto s1b = transformSegment(M2, scene.seg1);
        const auto s2b = transformSegment(M2, scene.seg2);

        canvas.drawWorldSegment(s1b.p, s1b.q, COL_MORADO);
        canvas.drawWorldSegment(s2b.p, s2b.q, COL_MORADO);
        canvas.drawPolygon(poly2, COL_NARANJA);
        for (const auto& v : poly2) {
            canvas.putPoint(canvas.worldToGridX(v.x), canvas.worldToGridY(v.y), COL_NARANJA);
        }
    }

    canvas.putPoint(ORIGIN_X, ORIGIN_Y, COL_ROJO);
}

}  // namespace cg

// ============================================================================
// Estado interactivo, interfaz OpenGL, consola ANSI y autoprueba (--self-test)
// ============================================================================

namespace {

cg::BaseScene gScene;
cg::RasterCanvas gCanvas(cg::GRID_WIDTH, cg::GRID_HEIGHT);

bool gShowPixelGrid = true;          // Tecla G: rejilla de pixeles drawGrid()
bool gShowTransformedLattice = true; // Tecla M: cuadricula local transformada
bool gShowOriginal = true;           // Tecla H: figura original de referencia
int gMode = 0;                       // 0..8 o 9 (cadena personalizada tecla C)

struct LiveParams {
    double shearX = 0.7;
    double shearY = 0.4;
    double angleDeg = 45.0;
    double refPhiDeg = 30.0;
    double scaleX = 1.5;
    double scaleY = 0.8;
    double tx = 2.0;
    double ty = -2.0;
    bool reflectY = false;
    bool reverseOrder = false;
} gParams;

std::vector<cg::NamedTransform> gCustomChain;

std::string fmtNum(double v, int prec = 2) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(prec) << cg::cleanZero(v);
    return oss.str();
}

cg::ExperimentConfig buildActiveConfig() {
    cg::ExperimentConfig cfg{};
    switch (gMode) {
        case 0:
            cfg.id = "BASE";
            cfg.title = "0: Escena base (Identidad)";
            cfg.steps = {};
            break;
        case 1:
            cfg.id = "SHEAR_X";
            cfg.title = "1: Cizallamiento horizontal Hx(" + fmtNum(gParams.shearX) + ")";
            cfg.steps = {{"Hx(" + fmtNum(gParams.shearX) + ")", cg::shearXMatrix(gParams.shearX)}};
            break;
        case 2:
            cfg.id = "SHEAR_Y";
            cfg.title = "2: Cizallamiento vertical Hy(" + fmtNum(gParams.shearY) + ")";
            cfg.steps = {{"Hy(" + fmtNum(gParams.shearY) + ")", cg::shearYMatrix(gParams.shearY)}};
            break;
        case 3:
            cfg.id = "REF_Y";
            cfg.title = "3: Reflexion respecto al eje vertical (Ref_y)";
            cfg.steps = {{"Ref_y", cg::reflectionYMatrix()}};
            break;
        case 4:
            cfg.id = "REF_PHI";
            cfg.title = "4: Reflexion recta oblicua phi=" + fmtNum(gParams.refPhiDeg, 1) + " deg";
            cfg.steps = {
                {"R(" + fmtNum(-gParams.refPhiDeg, 1) + ")", cg::rotationMatrix(-gParams.refPhiDeg)},
                {"Ref_x", cg::reflectionXMatrix()},
                {"R(" + fmtNum(gParams.refPhiDeg, 1) + ")", cg::rotationMatrix(gParams.refPhiDeg)}
            };
            cfg.drawReflectionAxis = true;
            cfg.reflectionAxisDeg = gParams.refPhiDeg;
            break;
        case 5:
            cfg.id = "ORDEN_COMP";
            cfg.title = "5: Orden R(" + fmtNum(gParams.angleDeg, 1) + ")*Hx(" + fmtNum(gParams.shearX)
                      + ") [azul] vs Hx*R [naranja]";
            cfg.steps = {
                {"Hx(" + fmtNum(gParams.shearX) + ")", cg::shearXMatrix(gParams.shearX)},
                {"R(" + fmtNum(gParams.angleDeg, 1) + ")", cg::rotationMatrix(gParams.angleDeg)}
            };
            cfg.hasSecondBranch = true;
            cfg.secondSteps = {
                {"R(" + fmtNum(gParams.angleDeg, 1) + ")", cg::rotationMatrix(gParams.angleDeg)},
                {"Hx(" + fmtNum(gParams.shearX) + ")", cg::shearXMatrix(gParams.shearX)}
            };
            break;
        case 6:
            cfg.id = "AREA_COMP";
            cfg.title = "6: Composicion con cambio de area T*R*S*Hx";
            cfg.steps = {
                {"Hx(" + fmtNum(gParams.shearX) + ")", cg::shearXMatrix(gParams.shearX)},
                {"S(" + fmtNum(gParams.scaleX) + "," + fmtNum(gParams.scaleY) + ")",
                 cg::scalingMatrix(gParams.scaleX, gParams.scaleY)},
                {"R(" + fmtNum(gParams.angleDeg, 1) + ")", cg::rotationMatrix(gParams.angleDeg)},
                {"T(" + fmtNum(gParams.tx, 1) + "," + fmtNum(gParams.ty, 1) + ")",
                 cg::translationMatrix(gParams.tx, gParams.ty)}
            };
            break;
        case 7:
            cfg.id = "SINGULAR";
            cfg.title = "7: Caso degenerado singular S(" + fmtNum(gParams.scaleX) + ", 0.00)";
            cfg.steps = {{"S(" + fmtNum(gParams.scaleX) + ",0.00)", cg::scalingMatrix(gParams.scaleX, 0.0)}};
            break;
        case 8: {
            cfg.id = "LIVE";
            cfg.title = gParams.reverseOrder
                            ? "8: Modo interactivo libre (ORDEN INVERTIDO)"
                            : "8: Modo interactivo libre (Hx -> S -> Ref? -> R -> T)";
            std::vector<cg::NamedTransform> st;
            st.push_back({"Hx(" + fmtNum(gParams.shearX) + ")", cg::shearXMatrix(gParams.shearX)});
            st.push_back({"S(" + fmtNum(gParams.scaleX) + "," + fmtNum(gParams.scaleY) + ")",
                          cg::scalingMatrix(gParams.scaleX, gParams.scaleY)});
            if (gParams.reflectY) st.push_back({"Ref_y", cg::reflectionYMatrix()});
            st.push_back({"R(" + fmtNum(gParams.angleDeg, 1) + ")", cg::rotationMatrix(gParams.angleDeg)});
            st.push_back({"T(" + fmtNum(gParams.tx, 1) + "," + fmtNum(gParams.ty, 1) + ")",
                          cg::translationMatrix(gParams.tx, gParams.ty)});
            if (gParams.reverseOrder) std::reverse(st.begin(), st.end());
            cfg.steps = std::move(st);
            break;
        }
        default:
            cfg.id = "CUSTOM";
            cfg.title = "C: Cadena personalizada (" + std::to_string(gCustomChain.size()) + " pasos)";
            cfg.steps = gCustomChain;
            break;
    }
    return cfg;
}

void setModeDefaults(int mode) {
    gMode = mode;
    if (mode == 1) {
        gParams.shearX = 0.7;
    } else if (mode == 2) {
        gParams.shearY = 0.4;
    } else if (mode == 4) {
        gParams.refPhiDeg = 30.0;
    } else if (mode == 5) {
        gParams.shearX = 0.7;
        gParams.angleDeg = 45.0;
    } else if (mode == 6) {
        gParams.shearX = 0.5;
        gParams.scaleX = 1.5;
        gParams.scaleY = 0.8;
        gParams.angleDeg = 25.0;
        gParams.tx = 2.0;
        gParams.ty = -2.0;
    } else if (mode == 7) {
        gParams.scaleX = 1.0;
        gParams.scaleY = 0.0;
    } else if (mode == 8) {
        gParams.shearX = 0.5;
        gParams.scaleX = 1.2;
        gParams.scaleY = 0.9;
        gParams.angleDeg = 20.0;
        gParams.tx = 1.0;
        gParams.ty = -1.0;
        gParams.reflectY = false;
        gParams.reverseOrder = false;
    }
}

void renderTerminalDashboard() {
    std::cout << "\033[2J\033[H";

    const auto cfg = buildActiveConfig();
    const cg::Matrix3 M1 = cg::composeInApplicationOrder(cfg.steps);
    const auto rep = cg::evaluateTransform(gScene, M1);

    std::cout << "============================================================================\n"
              << "  LABORATORIO DE TRANSFORMACIONES AFINES 2D  |  Rejilla 40x30 (Origen 20,15)\n"
              << "============================================================================\n"
              << "MODOS : [0] Base  [1] Hx  [2] Hy  [3] Ref_y  [4] Ref_phi  [5] Orden R*Hx/Hx*R\n"
              << "        [6] Cambio de area (4 pasos)  [7] Singular  [8] Libre  [C] Cadena N>=3\n"
              << "AJUSTE: [+/-] Parametro activo | [A/D] kx | [J/L] ky | [Z/X] sx | [W/S] sy\n"
              << "        [Q/E] angulo | [Flechas] tx,ty | [F] Ref_y | [O] Invertir orden\n"
              << "VISTA : [G] Rejilla(" << (gShowPixelGrid ? "ON" : "OFF")
              << ") | [M] Cuadricula(" << (gShowTransformedLattice ? "ON" : "OFF")
              << ") | [H] Original(" << (gShowOriginal ? "ON" : "OFF")
              << ") | [R] Reset | [ESC] Salir\n"
              << "----------------------------------------------------------------------------\n";

    std::cout << "ACTIVO : " << cfg.title << "\n"
              << "ORDEN  : " << cg::formatApplicationOrder(cfg.steps) << "\n"
              << "PROD   : " << cg::formatMatrixProductOrder(cfg.steps) << "\n";

    std::cout << std::fixed << std::setprecision(2)
              << "PARAMS : kx=" << cg::cleanZero(gParams.shearX)
              << " ky=" << cg::cleanZero(gParams.shearY)
              << " sx=" << cg::cleanZero(gParams.scaleX)
              << " sy=" << cg::cleanZero(gParams.scaleY)
              << " ang=" << cg::cleanZero(gParams.angleDeg)
              << " phi=" << cg::cleanZero(gParams.refPhiDeg)
              << " t=(" << cg::cleanZero(gParams.tx) << "," << cg::cleanZero(gParams.ty) << ")\n";

    std::cout << "MATRIZ M (3x3, p' = M * p):\n" << cg::formatMatrix(rep.M, 4);

    std::cout << std::fixed << std::setprecision(4)
              << "det(A) : " << cg::cleanZero(rep.detA)
              << "  |  Estado: "
              << (rep.singular ? "SINGULAR (NO INVERTIBLE - sin inversa)" : "INVERTIBLE") << "\n";

    std::cout << "AREA   : Orig=" << rep.origSignedArea
              << " | Trans(signo)=" << cg::cleanZero(rep.transSignedArea)
              << " | |Trans|=" << cg::cleanZero(rep.transAbsArea)
              << " | |det|*Orig=" << cg::cleanZero(rep.expectedAbsArea) << "\n"
              << "ORIENT : " << rep.origOrientation << "  ->  " << rep.transOrientation << "\n";

    std::cout << "PARAL. : u'=" << cg::formatPoint(rep.transParallel.dir1, 2)
              << " v'=" << cg::formatPoint(rep.transParallel.dir2, 2)
              << " | seno=" << rep.transParallel.normalizedSine
              << " | sep=" << rep.transParallel.lineSeparation
              << " | " << rep.transParallel.status << "\n";

    const std::size_t idx = gScene.highlightVertexIndex;
    std::cout << "REF V4 : " << cg::formatPoint(gScene.polygon[idx], 2)
              << " -> " << cg::formatPoint(rep.transPoly[idx], 4)
              << "   |   Angulo V0: " << std::setprecision(2) << rep.origAngleAtV0
              << " -> " << rep.transAngleAtV0 << " deg\n";

    if (cfg.hasSecondBranch) {
        const cg::Matrix3 M2 = cg::composeInApplicationOrder(cfg.secondSteps);
        const auto rep2 = cg::evaluateTransform(gScene, M2);
        const double gap = cg::distanceBetween(rep.transPoly[idx], rep2.transPoly[idx]);
        std::cout << "----------------------------------------------------------------------------\n"
                  << "RAMA 2 (Naranja) : " << cg::formatMatrixProductOrder(cfg.secondSteps) << "\n"
                  << "V4 Azul=" << cg::formatPoint(rep.transPoly[idx], 4)
                  << " | V4 Naranja=" << cg::formatPoint(rep2.transPoly[idx], 4)
                  << " | ||Delta V4|| = " << std::setprecision(4) << gap << "\n";
    }
    std::cout << "============================================================================\n"
              << std::flush;
}

void runConsoleChainBuilder() {
    std::cout << "\n--- CONSTRUCTOR DE CADENA PERSONALIZADA (N >= 3 transformaciones) ---\n"
              << "Cantidad de transformaciones (0 para cancelar): " << std::flush;
    int count = 0;
    if (!(std::cin >> count) || count <= 0) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        renderTerminalDashboard();
        return;
    }

    std::vector<cg::NamedTransform> chain;
    for (int i = 0; i < count; ++i) {
        std::cout << "Paso " << (i + 1)
                  << " [1:T  2:R  3:S  4:Hx  5:Hy  6:Ref_x  7:Ref_y  8:Ref_phi]: " << std::flush;
        int op = 0;
        if (!(std::cin >> op)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            renderTerminalDashboard();
            return;
        }
        if (op == 1) {
            double x = 0.0, y = 0.0;
            std::cout << "  tx ty: ";
            if (!(std::cin >> x >> y)) { std::cin.clear(); std::cin.ignore(10000, '\n'); break; }
            chain.push_back({"T(" + fmtNum(x, 1) + "," + fmtNum(y, 1) + ")", cg::translationMatrix(x, y)});
        } else if (op == 2) {
            double d = 0.0;
            std::cout << "  grados: ";
            if (!(std::cin >> d)) { std::cin.clear(); std::cin.ignore(10000, '\n'); break; }
            chain.push_back({"R(" + fmtNum(d, 1) + ")", cg::rotationMatrix(d)});
        } else if (op == 3) {
            double sx = 1.0, sy = 1.0;
            std::cout << "  sx sy: ";
            if (!(std::cin >> sx >> sy)) { std::cin.clear(); std::cin.ignore(10000, '\n'); break; }
            chain.push_back({"S(" + fmtNum(sx) + "," + fmtNum(sy) + ")", cg::scalingMatrix(sx, sy)});
        } else if (op == 4) {
            double k = 0.0;
            std::cout << "  k: ";
            if (!(std::cin >> k)) { std::cin.clear(); std::cin.ignore(10000, '\n'); break; }
            chain.push_back({"Hx(" + fmtNum(k) + ")", cg::shearXMatrix(k)});
        } else if (op == 5) {
            double k = 0.0;
            std::cout << "  k: ";
            if (!(std::cin >> k)) { std::cin.clear(); std::cin.ignore(10000, '\n'); break; }
            chain.push_back({"Hy(" + fmtNum(k) + ")", cg::shearYMatrix(k)});
        } else if (op == 6) {
            chain.push_back({"Ref_x", cg::reflectionXMatrix()});
        } else if (op == 7) {
            chain.push_back({"Ref_y", cg::reflectionYMatrix()});
        } else if (op == 8) {
            double phi = 0.0;
            std::cout << "  phi (grados): ";
            if (!(std::cin >> phi)) { std::cin.clear(); std::cin.ignore(10000, '\n'); break; }
            chain.push_back({"Ref(" + fmtNum(phi, 1) + ")", cg::reflectionLineMatrix(phi)});
        }
    }

    if (!chain.empty()) {
        gCustomChain = std::move(chain);
        gMode = 9;
    }
    renderTerminalDashboard();
}

// Rejilla 40 x 30 del curso (drawGrid), activable con tecla G.
// Unico lugar donde se utiliza GL_LINES.
void drawGrid() {
    if (!gShowPixelGrid) return;
    glColor3f(cg::COL_GRID_LINE.r, cg::COL_GRID_LINE.g, cg::COL_GRID_LINE.b);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int x = 0; x <= cg::GRID_WIDTH; ++x) {
        glVertex2f(static_cast<float>(x), 0.0f);
        glVertex2f(static_cast<float>(x), static_cast<float>(cg::GRID_HEIGHT));
    }
    for (int y = 0; y <= cg::GRID_HEIGHT; ++y) {
        glVertex2f(0.0f, static_cast<float>(y));
        glVertex2f(static_cast<float>(cg::GRID_WIDTH), static_cast<float>(y));
    }
    glEnd();
}

// Dibuja exclusivamente las celdas logicas pintadas por putPixel()/putPoint()
// con margen m = 0.08.
void flushCanvasToOpenGL(const cg::RasterCanvas& canvas) {
    glClear(GL_COLOR_BUFFER_BIT);
    drawGrid();

    const auto& pix = canvas.pixels();
    const auto& occ = canvas.occupied();
    const int w = canvas.width();
    const int h = canvas.height();
    constexpr float m = cg::CELL_MARGIN;

    glBegin(GL_QUADS);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const std::size_t idx = static_cast<std::size_t>(y * w + x);
            if (!occ[idx]) continue;
            const cg::Color& c = pix[idx];
            glColor3f(c.r, c.g, c.b);
            glVertex2f(static_cast<float>(x) + m,     static_cast<float>(y) + m);
            glVertex2f(static_cast<float>(x + 1) - m, static_cast<float>(y) + m);
            glVertex2f(static_cast<float>(x + 1) - m, static_cast<float>(y + 1) - m);
            glVertex2f(static_cast<float>(x) + m,     static_cast<float>(y + 1) - m);
        }
    }
    glEnd();
}

void updateWindowTitle(GLFWwindow* window) {
    const auto cfg = buildActiveConfig();
    const std::string title = "UNSAAC CG1 - Transformaciones Afines 2D | " + cfg.title;
    glfwSetWindowTitle(window, title.c_str());
}

void renderFrame() {
    const auto cfg = buildActiveConfig();
    cg::renderExperimentToCanvas(gCanvas, gScene, cfg, gShowOriginal, gShowTransformedLattice);
    flushCanvasToOpenGL(gCanvas);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    bool changed = false;

    if (action == GLFW_PRESS && key >= GLFW_KEY_0 && key <= GLFW_KEY_8) {
        setModeDefaults(key - GLFW_KEY_0);
        changed = true;
    } else if (action == GLFW_PRESS && key == GLFW_KEY_G) {
        gShowPixelGrid = !gShowPixelGrid;
        changed = true;
    } else if (action == GLFW_PRESS && key == GLFW_KEY_M) {
        gShowTransformedLattice = !gShowTransformedLattice;
        changed = true;
    } else if (action == GLFW_PRESS && key == GLFW_KEY_H) {
        gShowOriginal = !gShowOriginal;
        changed = true;
    } else if (action == GLFW_PRESS && key == GLFW_KEY_R) {
        gParams = LiveParams{};
        setModeDefaults(gMode == 9 ? 0 : gMode);
        changed = true;
    } else if (action == GLFW_PRESS && key == GLFW_KEY_C) {
        runConsoleChainBuilder();
        updateWindowTitle(window);
        return;
    } else if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
        if (gMode == 1 || gMode == 5 || gMode == 6 || gMode == 8) gParams.shearX += 0.1;
        else if (gMode == 2) gParams.shearY += 0.1;
        else if (gMode == 4) gParams.refPhiDeg += 5.0;
        else if (gMode == 7) gParams.scaleX += 0.1;
        changed = true;
    } else if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
        if (gMode == 1 || gMode == 5 || gMode == 6 || gMode == 8) gParams.shearX -= 0.1;
        else if (gMode == 2) gParams.shearY -= 0.1;
        else if (gMode == 4) gParams.refPhiDeg -= 5.0;
        else if (gMode == 7) gParams.scaleX -= 0.1;
        changed = true;
    } else if (key == GLFW_KEY_A) { gParams.shearX -= 0.1; changed = true; }
    else if (key == GLFW_KEY_D) { gParams.shearX += 0.1; changed = true; }
    else if (key == GLFW_KEY_J) { gParams.shearY -= 0.1; changed = true; }
    else if (key == GLFW_KEY_L) { gParams.shearY += 0.1; changed = true; }
    else if (key == GLFW_KEY_Z) { gParams.scaleX -= 0.1; changed = true; }
    else if (key == GLFW_KEY_X) { gParams.scaleX += 0.1; changed = true; }
    else if (key == GLFW_KEY_S) { gParams.scaleY -= 0.1; changed = true; }
    else if (key == GLFW_KEY_W) { gParams.scaleY += 0.1; changed = true; }
    else if (key == GLFW_KEY_Q) {
        if (gMode == 4) gParams.refPhiDeg += 5.0;
        else gParams.angleDeg += 5.0;
        changed = true;
    } else if (key == GLFW_KEY_E) {
        if (gMode == 4) gParams.refPhiDeg -= 5.0;
        else gParams.angleDeg -= 5.0;
        changed = true;
    } else if (key == GLFW_KEY_LEFT)  { gParams.tx -= 0.5; changed = true; }
    else if (key == GLFW_KEY_RIGHT) { gParams.tx += 0.5; changed = true; }
    else if (key == GLFW_KEY_UP)    { gParams.ty += 0.5; changed = true; }
    else if (key == GLFW_KEY_DOWN)  { gParams.ty -= 0.5; changed = true; }
    else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
        gParams.reflectY = !gParams.reflectY;
        gMode = 8;
        changed = true;
    } else if (action == GLFW_PRESS && key == GLFW_KEY_O) {
        gParams.reverseOrder = !gParams.reverseOrder;
        gMode = 8;
        changed = true;
    }

    if (changed) {
        updateWindowTitle(window);
        renderTerminalDashboard();
    }
}

void framebufferSizeCallback(GLFWwindow*, int w, int h) {
    if (h <= 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(cg::GRID_WIDTH),
            0.0, static_cast<double>(cg::GRID_HEIGHT),
            -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int exportAllPresetsToPPM(const std::string& outDir) {
    std::filesystem::create_directories(outDir);
    const auto presets = cg::buildPresetExperiments();
    cg::RasterCanvas canvas(cg::GRID_WIDTH, cg::GRID_HEIGHT);
    for (const auto& exp : presets) {
        cg::renderExperimentToCanvas(canvas, gScene, exp, true, true);
        const std::string filePath = outDir + "/" + exp.id + ".ppm";
        if (!canvas.savePPM(filePath, true, 25)) {
            std::cerr << "Error guardando " << filePath << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Exportado raster real 40x30 (con rejilla): " << filePath << "\n";
    }
    return EXIT_SUCCESS;
}

void requireCheck(bool cond, const std::string& msg) {
    if (!cond) {
        throw std::runtime_error("FALLO EN SELF-TEST: " + msg);
    }
}

void requireNear(double actual, double expected, double tol, const std::string& msg) {
    if (std::abs(actual - expected) >= tol) {
        std::ostringstream oss;
        oss << msg << " (esperado=" << expected << ", obtenido=" << actual
            << ", diff=" << std::abs(actual - expected) << ")";
        throw std::runtime_error("FALLO EN SELF-TEST: " + oss.str());
    }
}

void requireMatrixIdentity(const cg::Matrix3& M, double tol, const std::string& msg) {
    const cg::Matrix3 I = cg::identityMatrix();
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            requireNear(M.a[r][c], I.a[r][c], tol, msg);
        }
    }
}

int runSelfTest() {
    try {
        constexpr double TOL = 1e-9;
        const cg::BaseScene scene;
        const auto presets = cg::buildPresetExperiments();

        // 0. Escena base: poligono asimetrico (6 vertices, area +24.0) y L1 || L2
        requireCheck(scene.polygon.size() == 6, "Poligono base debe tener 6 vertices");
        const double baseArea = cg::signedArea(scene.polygon);
        requireNear(baseArea, 24.0, TOL, "Area base con signo debe ser +24.0");
        const auto basePar = cg::measureParallelism(scene.seg1, scene.seg2);
        requireCheck(basePar.parallel && !basePar.coincident,
                     "L1 y L2 base deben ser paralelos distintos");
        std::cout << "[PASS] Modo 0 / BASE: poligono 6 vertices, area = +24.0, L1 || L2.\n";

        auto findPreset = [&](const std::string& id) -> const cg::ExperimentConfig& {
            for (const auto& p : presets) {
                if (p.id == id) return p;
            }
            throw std::runtime_error("Preset no encontrado: " + id);
        };

        // 1. E1 / Modo 1: Cizallamiento horizontal Hx(0.7)
        {
            const auto& exp = findPreset("SHEAR_X");
            const auto rep = cg::evaluateTransform(scene, cg::composeInApplicationOrder(exp.steps));
            requireNear(rep.detA, 1.0, TOL, "E1 Hx(0.7): det(A) debe ser 1");
            requireNear(rep.transSignedArea, 24.0, TOL, "E1 Hx(0.7): area debe ser +24");
            requireCheck(rep.transParallel.parallel && !rep.transParallel.coincident,
                         "E1 Hx(0.7): L1' y L2' deben seguir paralelos distintos");
            requireNear(rep.transParallel.normalizedSine, 0.0, TOL,
                        "E1 Hx(0.7): seno normalizado debe ser 0");
            requireCheck(std::abs(rep.transAngleAtV0 - 90.0) > 10.0,
                         "E1 Hx(0.7): el angulo recto en V0 debe alterarse");
            std::cout << "[PASS] Modo 1 / E1 (SHEAR_X): det=1, area=+24, paralelismo intacto, angulo alterado.\n";
        }

        // 2. Modo 2: Cizallamiento vertical Hy(0.4)
        {
            const cg::Matrix3 M = cg::shearYMatrix(0.4);
            const auto rep = cg::evaluateTransform(scene, M);
            requireNear(rep.detA, 1.0, TOL, "Modo 2 Hy(0.4): det(A) debe ser 1");
            requireNear(rep.transSignedArea, 24.0, TOL, "Modo 2 Hy(0.4): area debe ser +24");
            requireCheck(rep.transParallel.parallel && !rep.transParallel.coincident,
                         "Modo 2 Hy(0.4): L1' y L2' deben seguir paralelos distintos");
            requireNear(rep.transParallel.normalizedSine, 0.0, TOL,
                        "Modo 2 Hy(0.4): seno normalizado debe ser 0");
            std::cout << "[PASS] Modo 2 (SHEAR_Y): det=1, area=+24, paralelismo intacto.\n";
        }

        // 3. E2 / Modo 3: Reflexion respecto al eje vertical (Ref_y)
        {
            const auto& exp = findPreset("REF_Y");
            const auto rep = cg::evaluateTransform(scene, cg::composeInApplicationOrder(exp.steps));
            requireNear(rep.detA, -1.0, TOL, "E2 Ref_y: det(A) debe ser -1");
            requireNear(rep.transSignedArea, -24.0, TOL, "E2 Ref_y: area con signo debe ser -24");
            requireNear(rep.transAbsArea, 24.0, TOL, "E2 Ref_y: area absoluta debe ser 24");
            requireCheck(rep.transParallel.parallel && !rep.transParallel.coincident,
                         "E2 Ref_y: L1' y L2' paralelos");
            std::cout << "[PASS] Modo 3 / E2 (REF_Y): det=-1, area_signo=-24, |area|=24, orientacion CW.\n";
        }

        // 4. Modo 4: Reflexion oblicua Ref_30 = R(30) * Ref_x * R(-30)
        {
            const auto& exp = findPreset("REF_30");
            const cg::Matrix3 M = cg::composeInApplicationOrder(exp.steps);
            const auto rep = cg::evaluateTransform(scene, M);
            const double c60 = std::cos(60.0 * cg::PI / 180.0);
            const double s60 = std::sin(60.0 * cg::PI / 180.0);
            requireNear(M.a[0][0],  c60, TOL, "Modo 4 REF_30: a[0][0] == cos(60)");
            requireNear(M.a[0][1],  s60, TOL, "Modo 4 REF_30: a[0][1] == sin(60)");
            requireNear(M.a[1][0],  s60, TOL, "Modo 4 REF_30: a[1][0] == sin(60)");
            requireNear(M.a[1][1], -c60, TOL, "Modo 4 REF_30: a[1][1] == -cos(60)");
            requireNear(rep.detA, -1.0, TOL, "Modo 4 REF_30: det(A) debe ser -1");
            requireNear(rep.transSignedArea, -24.0, TOL, "Modo 4 REF_30: area con signo debe ser -24");
            requireMatrixIdentity(cg::multiplyMatrix(M, M), TOL, "Modo 4 REF_30: Ref^2 == I");
            std::cout << "[PASS] Modo 4 (REF_30): R(30)*Ref_x*R(-30) coincide analiticamente, Ref^2=I.\n";
        }

        // 5. E3 / Modo 5: Orden R(45)*Hx(0.7) vs Hx(0.7)*R(45)
        {
            const auto& expComp = findPreset("ORDEN_COMP");
            const auto repRH = cg::evaluateTransform(scene, cg::composeInApplicationOrder(expComp.steps));
            const auto repHR = cg::evaluateTransform(scene, cg::composeInApplicationOrder(expComp.secondSteps));
            const cg::Point v4RH = repRH.transPoly[scene.highlightVertexIndex];
            const cg::Point v4HR = repHR.transPoly[scene.highlightVertexIndex];
            const double diffV4 = cg::distanceBetween(v4RH, v4HR);
            requireCheck(diffV4 > 1.0, "E3 Orden: R*Hx y Hx*R deben diferir en V4");
            requireNear(repRH.transSignedArea, 24.0, TOL, "E3 Orden R*Hx: area = +24");
            requireNear(repHR.transSignedArea, 24.0, TOL, "E3 Orden Hx*R: area = +24");
            requireCheck(repRH.transParallel.parallel && repHR.transParallel.parallel,
                         "E3 Orden: ambos ordenes conservan paralelismo");
            std::cout << "[PASS] Modo 5 / E3 (ORDEN_COMP): ||Delta V4|| = "
                      << std::fixed << std::setprecision(6) << diffV4 << ", area=+24.\n";
        }

        // 6. E4 / Modo 6: Cambio de area T(2,-2)*R(25)*S(1.5,0.8)*Hx(0.5)
        {
            const auto& exp = findPreset("AREA_COMP");
            const auto rep = cg::evaluateTransform(scene, cg::composeInApplicationOrder(exp.steps));
            requireNear(std::abs(rep.detA), 1.2, TOL, "E4 Cambio de area: |det(A)| debe ser 1.2");
            requireNear(rep.areaAbsError, 0.0, TOL, "E4 Cambio de area: |Area' - |det(A)|*Area_orig| < 1e-9");
            requireCheck(!rep.singular && rep.invertible, "E4 debe ser invertible");
            cg::Matrix3 inv{};
            requireCheck(cg::tryInvertAffine(rep.M, inv, TOL), "E4 tryInvertAffine debe tener exito");
            requireMatrixIdentity(cg::multiplyMatrix(inv, rep.M), TOL, "E4 M^-1 * M == I");
            requireMatrixIdentity(cg::multiplyMatrix(rep.M, inv), TOL, "E4 M * M^-1 == I");
            requireCheck(rep.transParallel.parallel && !rep.transParallel.coincident,
                         "E4 debe conservar paralelismo");
            std::cout << "[PASS] Modo 6 / E4 (AREA_COMP): |det|=1.2, error_area="
                      << std::scientific << std::setprecision(2) << rep.areaAbsError
                      << ", M^-1*M=I.\n";
        }

        // 7. E5 / Modo 7: Casos degenerados S(1,0) y S(0,1)
        {
            const auto& exp = findPreset("SINGULAR");
            const auto rep = cg::evaluateTransform(scene, cg::composeInApplicationOrder(exp.steps));
            requireCheck(rep.singular && !rep.invertible, "E5 S(1,0) debe marcarse singular");
            requireNear(rep.detA, 0.0, TOL, "E5 S(1,0): det(A) debe ser 0");
            requireNear(rep.transAbsArea, 0.0, TOL, "E5 S(1,0): area debe ser 0");
            cg::Matrix3 dummyInv{};
            requireCheck(!cg::tryInvertAffine(rep.M, dummyInv, TOL),
                         "E5 S(1,0): tryInvertAffine debe rechazar matriz singular");
            requireCheck(rep.transParallel.coincident,
                         "E5 S(1,0): L1' y L2' colapsan sobre y = 0");

            const auto repX0 = cg::evaluateTransform(scene, cg::scalingMatrix(0.0, 1.0));
            requireCheck(repX0.singular && !repX0.invertible, "E5 S(0,1) debe marcarse singular");
            requireNear(repX0.detA, 0.0, TOL, "E5 S(0,1): det(A) debe ser 0");
            requireNear(repX0.transAbsArea, 0.0, TOL, "E5 S(0,1): area debe ser 0");
            requireCheck(!cg::tryInvertAffine(repX0.M, dummyInv, TOL),
                         "E5 S(0,1): tryInvertAffine debe rechazar matriz singular");
            requireCheck(repX0.transParallel.coincident,
                         "E5 S(0,1): L1' y L2' colapsan sobre x = 0");
            std::cout << "[PASS] Modo 7 / E5 (SINGULAR): S(1,0) y S(0,1) con det=0, area=0, sin inversa.\n";
        }

        // 8. Modo 8 (Libre) + Cadena N>=3 + RasterCanvas 40x30
        {
            setModeDefaults(8);
            gParams.reflectY = true;
            gParams.reverseOrder = false;
            const auto cfgFwd = buildActiveConfig();
            const auto repFwd = cg::evaluateTransform(scene, cg::composeInApplicationOrder(cfgFwd.steps));
            requireNear(repFwd.areaAbsError, 0.0, TOL, "Modo 8 forward: |Area'| == |det| * Area_orig");
            requireCheck(repFwd.detA < 0.0, "Modo 8 con Ref_y: det(A) debe ser negativo");

            gParams.reverseOrder = true;
            const auto cfgRev = buildActiveConfig();
            const auto repRev = cg::evaluateTransform(scene, cg::composeInApplicationOrder(cfgRev.steps));
            requireNear(repRev.areaAbsError, 0.0, TOL, "Modo 8 reverse: |Area'| == |det| * Area_orig");
            const double gapLive = cg::distanceBetween(
                repFwd.transPoly[scene.highlightVertexIndex],
                repRev.transPoly[scene.highlightVertexIndex]);
            requireCheck(gapLive > 0.1, "Modo 8: invertir orden debe cambiar V4");

            cg::RasterCanvas testCanvas(cg::GRID_WIDTH, cg::GRID_HEIGHT);
            cg::renderExperimentToCanvas(testCanvas, scene, findPreset("AREA_COMP"), true, true);
            std::size_t paintedCount = 0;
            for (bool occ : testCanvas.occupied()) {
                if (occ) ++paintedCount;
            }
            requireCheck(paintedCount > 50, "RasterCanvas 40x30 debe rasterizar celdas con putPoint/Bresenham");
            std::cout << "[PASS] Modo 8 (LIVE) y RasterCanvas 40x30: " << paintedCount
                      << " celdas rasterizadas correctamente.\n";
        }

        // Restaurar modo por defecto
        gParams = LiveParams{};
        setModeDefaults(0);

        std::cout << "\n[OK] SELF-TEST COMPLETADO: todas las verificaciones pasaron (tolerancia < 1e-9).\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc >= 2 && std::string(argv[1]) == "--self-test") {
        return runSelfTest();
    }
    if (argc >= 3 && std::string(argv[1]) == "--export-ppm") {
        return exportAllPresetsToPPM(argv[2]);
    }

    if (!glfwInit()) {
        std::cerr << "Error: no se pudo inicializar GLFW.\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(
        cg::WINDOW_WIDTH, cg::WINDOW_HEIGHT,
        "UNSAAC CG1 - Laboratorio de Transformaciones Afines 2D",
        nullptr, nullptr);
    if (!window) {
        std::cerr << "Error: no se pudo crear la ventana OpenGL.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    framebufferSizeCallback(window, fbW, fbH);

    updateWindowTitle(window);
    renderTerminalDashboard();

    while (!glfwWindowShouldClose(window)) {
        renderFrame();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
