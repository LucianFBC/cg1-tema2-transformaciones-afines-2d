#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
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
// Sincronizado al 100% con la version modular (src/):
//   - Rejilla discreta 160 x 120 (4 px/unidad, ventana 1280 x 960, 8x8 px/celda)
//   - Rasterizacion propia con putPixel()/putPoint(), Bresenham y drawPolygon()
//   - Cero glTranslatef/glRotatef/glScalef/GL_POLYGON; GL_LINES solo en drawGrid()
//   - Autoprueba headless integrada con --self-test (tolerancia < 1e-9)
// ============================================================================

// ============================================================================
// UNSAAC - Computacion Grafica I (IF455AIN) - Semestre 2026-02
// Segunda Parcial - Tema 2: Transformaciones afines, deformacion y propiedades
// Modulo matematico propio (sin bibliotecas externas ni funciones de OpenGL).
//
// Convencion obligatoria:
//   - Vectores columna homogeneos: p = [x, y, 1]^T
//   - Transformacion: p' = M * p
//   - Composicion: en M = A * B * C, se aplica primero C, luego B y al final A.
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

// Reflexion respecto a una recta que pasa por el origen con angulo phi (grados):
// Ref_phi = R(phi) * Ref_x * R(-phi)
inline Matrix3 reflectionLineMatrix(double phiDegrees) {
    const Matrix3 rNeg = rotationMatrix(-phiDegrees);
    const Matrix3 refX = reflectionXMatrix();
    const Matrix3 rPos = rotationMatrix(phiDegrees);
    return multiplyMatrix(rPos, multiplyMatrix(refX, rNeg));
}

// Aplica una cadena de transformaciones en el orden cronologico entregado:
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

// Determinante de la submatriz lineal A (2x2 superior izquierda)
inline double linearDeterminant(const Matrix3& M) {
    return M.a[0][0] * M.a[1][1] - M.a[0][1] * M.a[1][0];
}

inline bool isSingular(const Matrix3& M, double tol = EPSILON) {
    return std::abs(linearDeterminant(M)) < tol;
}

// Intenta calcular la inversa de una matriz afin M solo si |det(A)| >= tol.
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
// > 0: recorrido antihorario (CCW)
// < 0: recorrido horario (CW)
// = 0: poligono colineal / degenerado
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
    double normalizedSine{0.0};   // |u x v| / (||u|| ||v||) in [0, 1], 0 = paralelos
    double angleDegrees{0.0};     // angulo entre rectas soporte in [0, 90]
    double lineSeparation{0.0};   // distancia perpendicular de P2 a la recta soporte de L1
    bool defined{true};
    bool parallel{false};
    bool coincident{false};       // cierto si las rectas soporte coinciden (colapso)
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

    // Distancia de P2 a la recta definida por P1 y dir1
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

// Angulo interno en un vertice (grados) para evidenciar deformacion angular.
// Devuelve false si alguna de las aristas incidentes colapso a longitud nula.
inline bool tryInteriorAngleDegrees(const Point& prev, const Point& curr, const Point& next, double& outDeg) {
    const Point u = {prev.x - curr.x, prev.y - curr.y};
    const Point v = {next.x - curr.x, next.y - curr.y};
    const double nu = std::hypot(u.x, u.y);
    const double nv = std::hypot(v.x, v.y);
    if (nu < EPSILON || nv < EPSILON) {
        outDeg = 0.0;
        return false;
    }
    const double dot = std::clamp((u.x * v.x + u.y * v.y) / (nu * nv), -1.0, 1.0);
    outDeg = std::acos(dot) * (180.0 / PI);
    return true;
}

inline double interiorAngleDegrees(const Point& prev, const Point& curr, const Point& next) {
    double deg = 0.0;
    tryInteriorAngleDegrees(prev, curr, next, deg);
    return deg;
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

}  // namespace cg

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

// ============================================================================
// Escena base y casos experimentales para transformaciones afines 2D.
// Coordenadas cartesianas centradas en (0,0) -> celda (80,60) en rejilla 160x120
// (escala: PX_PER_UNIT = 4 celdas discretas por cada unidad matematica).
// ============================================================================

namespace cg {

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
    bool transAngleAtV0Defined{true};
    double origEdgeV0V1{0.0};
    double transEdgeV0V1{0.0};
    double origEdgeV5V0{0.0};
    double transEdgeV5V0{0.0};
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
    r.transAngleAtV0Defined = tryInteriorAngleDegrees(r.transPoly[5], r.transPoly[0], r.transPoly[1], r.transAngleAtV0);
    r.origEdgeV0V1 = distanceBetween(scene.polygon[0], scene.polygon[1]);
    r.transEdgeV0V1 = distanceBetween(r.transPoly[0], r.transPoly[1]);
    r.origEdgeV5V0 = distanceBetween(scene.polygon[5], scene.polygon[0]);
    r.transEdgeV5V0 = distanceBetween(r.transPoly[5], r.transPoly[0]);
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

inline void printReportToStream(std::ostream& os, const BaseScene& scene,
                                const std::string& title,
                                const std::vector<NamedTransform>& steps,
                                const TransformReport& rep) {
    os << "\n========================================================================\n";
    os << "  " << title << "\n";
    os << "========================================================================\n";
    os << "Orden de aplicacion : " << formatApplicationOrder(steps) << "\n";
    os << "Producto matricial  : " << formatMatrixProductOrder(steps) << "\n";
    os << "Matriz compuesta 3x3 (convencion p' = M * p):\n" << formatMatrix(rep.M, 4);

    os << std::fixed << std::setprecision(6);
    os << "det(A)              : " << cleanZero(rep.detA)
       << "   (|det(A)| = " << cleanZero(std::abs(rep.detA)) << ")\n";

    if (rep.singular) {
        os << "ADVERTENCIA SINGULAR: det(A) = 0 (|det(A)| < 1e-9). La transformacion NO es\n"
           << "                      invertible ni inyectiva; NO se calcula matriz inversa.\n";
    } else {
        os << "Invertibilidad      : INVERTIBLE (det(A) != 0)\n";
    }

    os << "\n--- Area y orientacion (formula del zapatero) ---\n";
    os << "Original    : Area_signo = " << cleanZero(rep.origSignedArea)
       << " | Area_abs = " << cleanZero(rep.origAbsArea)
       << " | " << rep.origOrientation << "\n";
    os << "Transformada: Area_signo = " << cleanZero(rep.transSignedArea)
       << " | Area_abs = " << cleanZero(rep.transAbsArea)
       << " | " << rep.transOrientation << "\n";
    os << "Contraste   : |det(A)| * Area_orig = " << cleanZero(rep.expectedAbsArea)
       << " | Error absoluto = " << std::scientific << std::setprecision(2) << rep.areaAbsError
       << std::fixed << std::setprecision(6) << "\n";

    os << "\n--- Angulos y longitudes ---\n";
    os << "Angulo interno en V0 (V5-V0-V1): " << rep.origAngleAtV0 << " deg -> ";
    if (rep.transAngleAtV0Defined) {
        os << rep.transAngleAtV0 << " deg\n";
    } else {
        os << "INDEFINIDO (arista colapsada)\n";
    }
    os << "Longitud arista V0-V1 (horiz.) : " << rep.origEdgeV0V1
       << " -> " << rep.transEdgeV0V1 << "\n";
    os << "Longitud arista V5-V0 (vert.)  : " << rep.origEdgeV5V0
       << " -> " << rep.transEdgeV5V0 << "\n";

    os << "\n--- Vertices del poligono ---\n";
    for (std::size_t i = 0; i < scene.polygon.size(); ++i) {
        os << "  " << scene.vertexNames[i] << ": "
           << std::setw(18) << formatPoint(scene.polygon[i], 3)
           << "  ->  " << formatPoint(rep.transPoly[i], 4);
        if (i == scene.highlightVertexIndex) os << "  <-- vertice de referencia";
        os << "\n";
    }

    os << "\n--- Medida de paralelismo (L1 y L2) ---\n";
    os << "Original    : L1=" << formatPoint(scene.seg1.p, 2) << "->" << formatPoint(scene.seg1.q, 2)
       << " u=" << formatPoint(rep.origParallel.dir1, 2)
       << " | L2=" << formatPoint(scene.seg2.p, 2) << "->" << formatPoint(scene.seg2.q, 2)
       << " v=" << formatPoint(rep.origParallel.dir2, 2) << "\n";
    os << "              |u x v|/(||u||||v||) = " << rep.origParallel.normalizedSine
       << " | angulo = " << rep.origParallel.angleDegrees
       << " deg | sep = " << rep.origParallel.lineSeparation
       << " | " << rep.origParallel.status << "\n";

    os << "Transformada: L1'=" << formatPoint(rep.transSeg1.p, 3) << "->" << formatPoint(rep.transSeg1.q, 3)
       << " u'=" << formatPoint(rep.transParallel.dir1, 3) << "\n"
       << "              L2'=" << formatPoint(rep.transSeg2.p, 3) << "->" << formatPoint(rep.transSeg2.q, 3)
       << " v'=" << formatPoint(rep.transParallel.dir2, 3) << "\n";
    if (rep.transParallel.defined) {
        os << "              |u' x v'|/(||u'||||v'||) = " << rep.transParallel.normalizedSine
           << " | angulo = " << rep.transParallel.angleDegrees
           << " deg | sep = " << rep.transParallel.lineSeparation
           << " | " << rep.transParallel.status << "\n";
    } else {
        os << "              " << rep.transParallel.status << "\n";
    }
}

inline void renderExperimentToCanvas(RasterCanvas& canvas, const BaseScene& scene,
                                     const ExperimentConfig& exp,
                                     bool showOriginal, bool showTransformedLattice) {
    canvas.clear(COL_BG);

    const Matrix3 I = identityMatrix();
    const Matrix3 M1 = composeInApplicationOrder(exp.steps);

    // 1. Cuadricula local transformable (primero en el fondo, 1 px continuo Bresenham)
    if (showTransformedLattice) {
        if (exp.steps.empty()) {
            canvas.drawTransformedLattice(I, scene.latticeXMin, scene.latticeXMax,
                                          scene.latticeYMin, scene.latticeYMax,
                                          scene.latticeStep, COL_TGRID_TRANS, 1);
        } else {
            const Color latCol = exp.hasSecondBranch ? COL_TGRID_FAINT : COL_TGRID_TRANS;
            canvas.drawTransformedLattice(M1, scene.latticeXMin, scene.latticeXMax,
                                          scene.latticeYMin, scene.latticeYMax,
                                          scene.latticeStep, latCol, 1);
        }
    }

    // 2. Ejes cartesianos delgados (1 px) con marcas cada 5 unidades
    canvas.drawAxes();

    // 3. Recta oblicua de reflexion mediante Bresenham discontinuo limpio (6px on / 4px off)
    if (exp.drawReflectionAxis) {
        const double rad = exp.reflectionAxisDeg * (PI / 180.0);
        const double L = 22.0;
        canvas.drawWorldSegment({-L * std::cos(rad), -L * std::sin(rad)},
                                { L * std::cos(rad),  L * std::sin(rad)},
                                COL_MORADO, 3, 1);
    }

    // 4. Figura y segmentos originales de referencia
    if (showOriginal) {
        if (exp.steps.empty()) {
            // En escena base (modo 0), mostrar la geometria inicial con trazo de 2 px
            canvas.drawWorldSegment(scene.seg1.p, scene.seg1.q, COL_VERDE, 1, 2);
            canvas.drawWorldSegment(scene.seg2.p, scene.seg2.q, COL_VERDE, 1, 2);
            canvas.drawSquareMarker(canvas.worldToGridX(scene.seg1.p.x),
                                    canvas.worldToGridY(scene.seg1.p.y), COL_VERDE, 1, false);
            canvas.drawSquareMarker(canvas.worldToGridX(scene.seg1.q.x),
                                    canvas.worldToGridY(scene.seg1.q.y), COL_VERDE, 1, false);
            canvas.drawSquareMarker(canvas.worldToGridX(scene.seg2.p.x),
                                    canvas.worldToGridY(scene.seg2.p.y), COL_VERDE, 1, false);
            canvas.drawSquareMarker(canvas.worldToGridX(scene.seg2.q.x),
                                    canvas.worldToGridY(scene.seg2.q.y), COL_VERDE, 1, false);

            canvas.drawPolygon(scene.polygon, COL_NEGRO, 1, 2);
            for (std::size_t i = 0; i < scene.polygon.size(); ++i) {
                const int gx = canvas.worldToGridX(scene.polygon[i].x);
                const int gy = canvas.worldToGridY(scene.polygon[i].y);
                if (i == scene.highlightVertexIndex) {
                    canvas.drawSquareMarker(gx, gy, COL_ROJO, 1, true);
                } else {
                    canvas.drawCrossMarker(gx, gy, COL_ROJO, 1);
                }
            }
        } else {
            // Como referencia frente a la transformada: trazo 1 px guionado (4px on / 2px off)
            canvas.drawWorldSegment(scene.seg1.p, scene.seg1.q, COL_GRIS, 2, 1);
            canvas.drawWorldSegment(scene.seg2.p, scene.seg2.q, COL_GRIS, 2, 1);
            canvas.drawCrossMarker(canvas.worldToGridX(scene.seg1.p.x),
                                   canvas.worldToGridY(scene.seg1.p.y), COL_GRIS, 1);
            canvas.drawCrossMarker(canvas.worldToGridX(scene.seg1.q.x),
                                   canvas.worldToGridY(scene.seg1.q.y), COL_GRIS, 1);
            canvas.drawCrossMarker(canvas.worldToGridX(scene.seg2.p.x),
                                   canvas.worldToGridY(scene.seg2.p.y), COL_GRIS, 1);
            canvas.drawCrossMarker(canvas.worldToGridX(scene.seg2.q.x),
                                   canvas.worldToGridY(scene.seg2.q.y), COL_GRIS, 1);

            canvas.drawPolygon(scene.polygon, COL_NEGRO, 2, 1);
            for (const auto& v : scene.polygon) {
                canvas.drawCrossMarker(canvas.worldToGridX(v.x),
                                       canvas.worldToGridY(v.y), COL_NEGRO, 1);
            }
        }
    }

    // 5. Figura y segmentos transformados principales (trazo 2 px continuo vibrante)
    if (!exp.steps.empty()) {
        const auto poly1 = transformPolygon(M1, scene.polygon);
        const auto s1 = transformSegment(M1, scene.seg1);
        const auto s2 = transformSegment(M1, scene.seg2);
        const bool singular = isSingular(M1);

        if (singular) {
            // En colapso singular (p. ej. S(1,0) sobre y=0), hacer evidente tanto el intervalo
            // de los segmentos colapsados L1'=L2' como el del poligono colapsado P'.
            const int x1a = canvas.worldToGridX(s1.p.x);
            const int y1a = canvas.worldToGridY(s1.p.y);
            const int x1b = canvas.worldToGridX(s1.q.x);
            const int y1b = canvas.worldToGridY(s1.q.y);
            canvas.drawLine(x1a, y1a - 1, x1b, y1b - 1, COL_VERDE, 1, 2);
            canvas.drawSquareMarker(x1a, y1a - 1, COL_VERDE, 1, false);
            canvas.drawSquareMarker(x1b, y1b - 1, COL_VERDE, 1, false);
        } else {
            canvas.drawWorldSegment(s1.p, s1.q, COL_VERDE, 1, 2);
            canvas.drawWorldSegment(s2.p, s2.q, COL_VERDE, 1, 2);
            canvas.drawSquareMarker(canvas.worldToGridX(s1.p.x),
                                    canvas.worldToGridY(s1.p.y), COL_VERDE, 1, false);
            canvas.drawSquareMarker(canvas.worldToGridX(s1.q.x),
                                    canvas.worldToGridY(s1.q.y), COL_VERDE, 1, false);
            canvas.drawSquareMarker(canvas.worldToGridX(s2.p.x),
                                    canvas.worldToGridY(s2.p.y), COL_VERDE, 1, false);
            canvas.drawSquareMarker(canvas.worldToGridX(s2.q.x),
                                    canvas.worldToGridY(s2.q.y), COL_VERDE, 1, false);
        }

        canvas.drawPolygon(poly1, COL_AZUL, 1, 2);
        for (std::size_t i = 0; i < poly1.size(); ++i) {
            const int gx = canvas.worldToGridX(poly1[i].x);
            const int gy = canvas.worldToGridY(poly1[i].y);
            if (i == scene.highlightVertexIndex) {
                canvas.drawSquareMarker(gx, gy, COL_ROJO, 1, true);
            } else {
                canvas.drawCrossMarker(gx, gy, COL_ROJO, 1);
            }
        }
    }

    // 6. Segunda rama para comparacion simultanea de orden (naranja 2 px continuo)
    if (exp.hasSecondBranch) {
        const Matrix3 M2 = composeInApplicationOrder(exp.secondSteps);
        const auto poly2 = transformPolygon(M2, scene.polygon);
        const auto s1b = transformSegment(M2, scene.seg1);
        const auto s2b = transformSegment(M2, scene.seg2);

        canvas.drawWorldSegment(s1b.p, s1b.q, COL_MORADO, 2, 1);
        canvas.drawWorldSegment(s2b.p, s2b.q, COL_MORADO, 2, 1);
        canvas.drawCrossMarker(canvas.worldToGridX(s1b.p.x),
                               canvas.worldToGridY(s1b.p.y), COL_MORADO, 1);
        canvas.drawCrossMarker(canvas.worldToGridX(s1b.q.x),
                               canvas.worldToGridY(s1b.q.y), COL_MORADO, 1);
        canvas.drawCrossMarker(canvas.worldToGridX(s2b.p.x),
                               canvas.worldToGridY(s2b.p.y), COL_MORADO, 1);
        canvas.drawCrossMarker(canvas.worldToGridX(s2b.q.x),
                               canvas.worldToGridY(s2b.q.y), COL_MORADO, 1);

        canvas.drawPolygon(poly2, COL_NARANJA, 1, 2);
        for (std::size_t i = 0; i < poly2.size(); ++i) {
            const int gx = canvas.worldToGridX(poly2[i].x);
            const int gy = canvas.worldToGridY(poly2[i].y);
            if (i == scene.highlightVertexIndex) {
                canvas.drawSquareMarker(gx, gy, COL_ROJO, 1, true);
            } else {
                canvas.drawCrossMarker(gx, gy, COL_NARANJA, 1);
            }
        }
    }
}

}  // namespace cg

// ============================================================================
// Laboratorio Interactivo de Transformaciones Afines 2D
// UNSAAC - Computacion Grafica I (IF455AIN) - Semestre 2026-02
//
// Estilo visual basado en el entorno de laboratorio del curso:
//   - Rejilla discreta 160 x 120 (4 px/unidad, ventana 1280 x 960, 8x8 px/celda)
//     con opcion G para mostrar u ocultar la rejilla discreta y por unidad.
//   - Rasterizacion propia mediante putPixel()/putPoint(), Bresenham y
//     drawPolygon() sin glTranslatef/glRotatef/glScalef ni GL_POLYGON.
//   - Panel compacto en consola que limpia la salida anterior y muestra
//     siempre el menu de opciones seguido de la informacion en vivo.
// ============================================================================

namespace {

cg::BaseScene gScene;
cg::RasterCanvas gCanvas(cg::GRID_WIDTH, cg::GRID_HEIGHT);

bool gShowPixelGrid = true;          // Tecla G: rejilla de pixeles drawGrid()
bool gShowTransformedLattice = true; // Tecla M: cuadricula local transformada
bool gShowOriginal = true;           // Tecla H: figura original de referencia
int gMode = 0;                       // 0..8 o 9 (cadena personalizada tecla C)

// Parametros interactivos editables en vivo
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
    bool reflectX = false;
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
            if (gParams.reflectX) {
                cfg.id = "REF_X";
                cfg.title = "3: Reflexion respecto al eje horizontal (Ref_x)";
                cfg.steps = {{"Ref_x", cg::reflectionXMatrix()}};
            } else {
                cfg.id = "REF_Y";
                cfg.title = "3: Reflexion respecto al eje vertical (Ref_y)";
                cfg.steps = {{"Ref_y", cg::reflectionYMatrix()}};
            }
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
            if (gParams.reflectX) st.push_back({"Ref_x", cg::reflectionXMatrix()});
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
    } else if (mode == 3) {
        gParams.reflectX = false;
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
        gParams.reflectX = false;
        gParams.reverseOrder = false;
    }
}

// Limpia la consola y redibuja el menu compacto + estado en vivo de la opcion activa
void renderTerminalDashboard() {
    std::cout << "\033[2J\033[H";

    const auto cfg = buildActiveConfig();
    const cg::Matrix3 M1 = cg::composeInApplicationOrder(cfg.steps);
    const auto rep = cg::evaluateTransform(gScene, M1);

    std::cout << "============================================================================\n"
              << "  LABORATORIO DE TRANSFORMACIONES AFINES 2D | Rejilla 160x120 (4 px/u)\n"
              << "============================================================================\n"
              << "MODOS : [0] Base  [1] Hx  [2] Hy  [3] Ref_y/x  [4] Ref_phi  [5] Orden R*Hx\n"
              << "        [6] Cambio de area (4 pasos)  [7] Singular  [8] Libre  [C] Cadena N>=3\n"
              << "AJUSTE: [+/-] Parametro activo | [A/D] kx | [J/L] ky | [Z/X] sx | [W/S] sy\n"
              << "        [Q/E] angulo | [Flechas] tx,ty | [F] Ref_y/x | [O] Invertir orden\n"
              << "VISTA : [G] Rejilla(" << (gShowPixelGrid ? "ON" : "OFF")
              << ") | [M] Cuadricula(" << (gShowTransformedLattice ? "ON" : "OFF")
              << ") | [H] Original(" << (gShowOriginal ? "ON" : "OFF")
              << ") | [V] Vertice(" << gScene.vertexNames[gScene.highlightVertexIndex]
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

    std::cout << "VERT.  : ";
    for (std::size_t i = 0; i < gScene.polygon.size(); ++i) {
        if (i == gScene.highlightVertexIndex) std::cout << "*";
        std::cout << gScene.vertexNames[i] << cg::formatPoint(gScene.polygon[i], 0)
                  << "->" << cg::formatPoint(rep.transPoly[i], 2);
        if (i == 2) std::cout << "\n         ";
        else if (i + 1 < gScene.polygon.size()) std::cout << " | ";
    }
    std::cout << "\n";

    const std::size_t idx = gScene.highlightVertexIndex;
    std::cout << "SEL " << gScene.vertexNames[idx] << " : " << cg::formatPoint(gScene.polygon[idx], 2)
              << " -> " << cg::formatPoint(rep.transPoly[idx], 4)
              << " | Ang V0: " << std::setprecision(2) << rep.origAngleAtV0 << "->";
    if (rep.transAngleAtV0Defined) {
        std::cout << rep.transAngleAtV0 << " deg";
    } else {
        std::cout << "INDEF.";
    }
    std::cout << " | |V5-V0|: " << rep.origEdgeV5V0 << "->" << rep.transEdgeV5V0 << "\n";

    if (cfg.hasSecondBranch) {
        const cg::Matrix3 M2 = cg::composeInApplicationOrder(cfg.secondSteps);
        const auto rep2 = cg::evaluateTransform(gScene, M2);
        const double gap = cg::distanceBetween(rep.transPoly[idx], rep2.transPoly[idx]);
        std::cout << "----------------------------------------------------------------------------\n"
                  << "RAMA 2 (Naranja) : " << cg::formatMatrixProductOrder(cfg.secondSteps) << "\n"
                  << gScene.vertexNames[idx] << " Azul=" << cg::formatPoint(rep.transPoly[idx], 4)
                  << " | " << gScene.vertexNames[idx] << " Naranja=" << cg::formatPoint(rep2.transPoly[idx], 4)
                  << " | ||Delta " << gScene.vertexNames[idx] << "|| = " << std::setprecision(4) << gap << "\n";
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
        } else {
            std::cout << "  Opcion invalida (use 1..8). Intente de nuevo.\n";
            --i;
        }
    }

    if (!chain.empty()) {
        gCustomChain = std::move(chain);
        gMode = 9;
    }
    renderTerminalDashboard();
}

// Rejilla de pixeles del curso (drawGrid), activable con G.
// Dibuja sub-rejilla discreta (160x120) y rejilla por unidad matematica (cada 4 celdas).
void drawPixelGridOpenGL() {
    if (!gShowPixelGrid) return;
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int x = 0; x <= cg::GRID_WIDTH; ++x) {
        const cg::Color c = (x % cg::PX_PER_UNIT == 0) ? cg::COL_GRID_LINE : cg::COL_GRID_SUB;
        glColor3f(c.r, c.g, c.b);
        glVertex2f(static_cast<float>(x), 0.0f);
        glVertex2f(static_cast<float>(x), static_cast<float>(cg::GRID_HEIGHT));
    }
    for (int y = 0; y <= cg::GRID_HEIGHT; ++y) {
        const cg::Color c = (y % cg::PX_PER_UNIT == 0) ? cg::COL_GRID_LINE : cg::COL_GRID_SUB;
        glColor3f(c.r, c.g, c.b);
        glVertex2f(0.0f, static_cast<float>(y));
        glVertex2f(static_cast<float>(cg::GRID_WIDTH), static_cast<float>(y));
    }
    glEnd();
}

// Dibuja exclusivamente las celdas logicas pintadas por putPoint() con margen m = 0.06
// (fiel a putPixel del Examen de Laboratorio; GL_LINES solo se usa en drawGrid()).
void flushCanvasToOpenGL(const cg::RasterCanvas& canvas) {
    glClear(GL_COLOR_BUFFER_BIT);
    drawPixelGridOpenGL();

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
            glVertex2f(static_cast<float>(x) + m,       static_cast<float>(y) + m);
            glVertex2f(static_cast<float>(x + 1) - m,   static_cast<float>(y) + m);
            glVertex2f(static_cast<float>(x + 1) - m,   static_cast<float>(y + 1) - m);
            glVertex2f(static_cast<float>(x) + m,       static_cast<float>(y + 1) - m);
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
    } else if (action == GLFW_PRESS && key == GLFW_KEY_V) {
        gScene.highlightVertexIndex = (gScene.highlightVertexIndex + 1) % gScene.polygon.size();
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
        if (gMode == 3) {
            gParams.reflectX = !gParams.reflectX;
        } else {
            gParams.reflectY = !gParams.reflectY;
            gMode = 8;
        }
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
    // Eje Y crece hacia arriba (coherente con el plano cartesiano matematico)
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
        if (!canvas.savePPM(filePath, true, cg::CELL_PX)) {
            std::cerr << "Error guardando " << filePath << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Exportado raster real 160x120 (4 px/u, 1280x960): " << filePath << "\n";
    }
    return EXIT_SUCCESS;
}


void requireCheck(bool cond, const std::string& msg) {
    if (!cond) {
        throw std::runtime_error("FALLO DE AUTOPRUEBA: " + msg);
    }
}

void requireNear(double actual, double expected, double tol, const std::string& msg) {
    if (std::abs(actual - expected) >= tol) {
        std::ostringstream oss;
        oss << msg << " (esperado=" << expected << ", obtenido=" << actual << ")";
        throw std::runtime_error("FALLO DE AUTOPRUEBA: " + oss.str());
    }
}

void requireMatrixIdentity(const cg::Matrix3& M, double tol, const std::string& msg) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            const double expVal = (r == c) ? 1.0 : 0.0;
            requireNear(M.a[r][c], expVal, tol, msg);
        }
    }
}

int runSelfTest() {
    try {
        constexpr double TOL = 1e-9;
        const cg::BaseScene scene;
        const auto presets = cg::buildPresetExperiments();

        auto findPreset = [&](const std::string& id) -> const cg::ExperimentConfig& {
            for (const auto& e : presets) {
                if (e.id == id) return e;
            }
            throw std::runtime_error("Preset no encontrado: " + id);
        };

        std::cout << "============================================================================\n"
                  << "  AUTOPRUEBA NUMERICA Y DE RASTERIZACION (VERSION ARCHIVO UNICO)\n"
                  << "============================================================================\n";

        // 1. Escena base
        {
            const double baseArea = cg::signedArea(scene.polygon);
            requireNear(baseArea, 24.0, TOL, "Area base debe ser +24.0");
            const auto basePar = cg::measureParallelism(scene.seg1, scene.seg2);
            requireCheck(basePar.parallel && !basePar.coincident, "L1 y L2 deben ser paralelos distintos");
            std::cout << "[PASS] Escena base: poligono de 6 vertices, area=+24.0, L1||L2.\n";
        }

        // 2. Cizallamiento horizontal Hx(0.7)
        {
            const auto& exp = findPreset("SHEAR_X");
            const auto rep = cg::evaluateTransform(scene, cg::composeInApplicationOrder(exp.steps));
            requireNear(rep.detA, 1.0, TOL, "Hx(0.7): det(A) debe ser 1");
            requireNear(rep.transSignedArea, 24.0, TOL, "Hx(0.7): area debe ser +24");
            requireCheck(rep.transParallel.parallel && !rep.transParallel.coincident,
                         "Hx(0.7): debe conservar paralelismo");
            requireCheck(std::abs(rep.transAngleAtV0 - 90.0) > 10.0,
                         "Hx(0.7): el angulo recto en V0 debe alterarse");
            requireNear(rep.transEdgeV5V0, std::hypot(3.5, 5.0), TOL,
                        "Hx(0.7): arista V5-V0 debe medir sqrt(37.25)");
            std::cout << "[PASS] Cizallamiento Hx(0.7): det=1, area=+24, paralelismo conservado, |V5-V0|=6.1033.\n";
        }

        // 3. Reflexion vertical Ref_y
        {
            const auto& exp = findPreset("REF_Y");
            const auto rep = cg::evaluateTransform(scene, cg::composeInApplicationOrder(exp.steps));
            requireNear(rep.detA, -1.0, TOL, "Ref_y: det(A) debe ser -1");
            requireNear(rep.transSignedArea, -24.0, TOL, "Ref_y: area signo = -24");
            requireNear(rep.transAbsArea, 24.0, TOL, "Ref_y: |area| = 24");
            std::cout << "[PASS] Reflexion vertical Ref_y: det=-1, area signo=-24 (CW), |area|=24.\n";
        }

        // 4. Reflexion oblicua Ref_30 = R(30)*Ref_x*R(-30)
        {
            const auto& exp = findPreset("REF_30");
            const cg::Matrix3 M = cg::composeInApplicationOrder(exp.steps);
            const auto rep = cg::evaluateTransform(scene, M);
            const double c60 = std::cos(60.0 * cg::PI / 180.0);
            const double s60 = std::sin(60.0 * cg::PI / 180.0);
            requireNear(M.a[0][0],  c60, TOL, "Ref_30: a[0][0] == cos(60)");
            requireNear(M.a[0][1],  s60, TOL, "Ref_30: a[0][1] == sin(60)");
            requireNear(M.a[1][0],  s60, TOL, "Ref_30: a[1][0] == sin(60)");
            requireNear(M.a[1][1], -c60, TOL, "Ref_30: a[1][1] == -cos(60)");
            requireNear(rep.detA, -1.0, TOL, "Ref_30: det(A) debe ser -1");
            requireMatrixIdentity(cg::multiplyMatrix(M, M), TOL, "Ref_30: Ref^2 == I");
            std::cout << "[PASS] Reflexion oblicua Ref_30: R(30)*Ref_x*R(-30) exacta y Ref^2=I.\n";
        }

        // 5. Comparacion de orden R(45)*Hx(0.7) vs Hx(0.7)*R(45)
        {
            const auto& expComp = findPreset("ORDEN_COMP");
            const auto repRH = cg::evaluateTransform(scene, cg::composeInApplicationOrder(expComp.steps));
            const auto repHR = cg::evaluateTransform(scene, cg::composeInApplicationOrder(expComp.secondSteps));
            const double diffV4 = cg::distanceBetween(repRH.transPoly[4], repHR.transPoly[4]);
            requireCheck(diffV4 > 4.0, "Orden: R*Hx y Hx*R deben diferir en V4");
            requireNear(repRH.transSignedArea, 24.0, TOL, "Orden R*Hx: area = +24");
            requireNear(repHR.transSignedArea, 24.0, TOL, "Orden Hx*R: area = +24");
            std::cout << "[PASS] Comparacion de orden: ||Delta V4|| = "
                      << std::fixed << std::setprecision(6) << diffV4 << ", area=+24.\n";
        }

        // 6. Composicion de 4 pasos con cambio de area
        {
            const auto& exp = findPreset("AREA_COMP");
            const auto rep = cg::evaluateTransform(scene, cg::composeInApplicationOrder(exp.steps));
            requireNear(std::abs(rep.detA), 1.2, TOL, "Cambio de area: |det(A)| debe ser 1.2");
            requireNear(rep.areaAbsError, 0.0, TOL, "Cambio de area: |Area' - |det(A)|*Area_orig| < 1e-9");
            cg::Matrix3 inv{};
            requireCheck(cg::tryInvertAffine(rep.M, inv, TOL), "Cambio de area debe ser invertible");
            requireMatrixIdentity(cg::multiplyMatrix(inv, rep.M), TOL, "M^-1 * M == I");
            requireMatrixIdentity(cg::multiplyMatrix(rep.M, inv), TOL, "M * M^-1 == I");
            std::cout << "[PASS] Composicion 4 pasos: |det|=1.2, error_area="
                      << std::scientific << std::setprecision(2) << rep.areaAbsError
                      << ", M^-1*M=I.\n";
        }

        // 7. Casos degenerados singulares S(1,0) y S(0,1)
        {
            const auto& exp = findPreset("SINGULAR");
            const auto rep = cg::evaluateTransform(scene, cg::composeInApplicationOrder(exp.steps));
            requireCheck(rep.singular && !rep.invertible, "S(1,0) debe marcarse singular");
            requireNear(rep.detA, 0.0, TOL, "S(1,0): det(A) debe ser 0");
            requireNear(rep.transAbsArea, 0.0, TOL, "S(1,0): area debe ser 0");
            requireCheck(!rep.transAngleAtV0Defined, "S(1,0): angulo en V0 indefinido");
            cg::Matrix3 dummyInv{};
            requireCheck(!cg::tryInvertAffine(rep.M, dummyInv, TOL), "S(1,0) no debe admitir inversa");
            requireCheck(rep.transParallel.coincident, "S(1,0): L1' y L2' colapsan sobre y=0");

            const auto repX0 = cg::evaluateTransform(scene, cg::scalingMatrix(0.0, 1.0));
            requireCheck(repX0.singular && !repX0.invertible, "S(0,1) debe marcarse singular");
            requireCheck(!cg::tryInvertAffine(repX0.M, dummyInv, TOL), "S(0,1) no debe admitir inversa");
            std::cout << "[PASS] Casos singulares S(1,0) y S(0,1): det=0, area=0, angulo indefinido, sin inversa.\n";
        }

        // 8. Modo libre y RasterCanvas 160x120
        {
            setModeDefaults(8);
            gParams.reflectY = true;
            gParams.reverseOrder = false;
            const auto repFwd = cg::evaluateTransform(scene, cg::composeInApplicationOrder(buildActiveConfig().steps));
            requireNear(repFwd.areaAbsError, 0.0, TOL, "Modo libre forward");
            requireCheck(repFwd.detA < 0.0, "Modo libre con Ref_y: det < 0");

            gParams.reverseOrder = true;
            const auto repRev = cg::evaluateTransform(scene, cg::composeInApplicationOrder(buildActiveConfig().steps));
            requireNear(repRev.areaAbsError, 0.0, TOL, "Modo libre reverse");
            requireCheck(cg::distanceBetween(repFwd.transPoly[4], repRev.transPoly[4]) > 0.1,
                         "Modo libre: invertir orden cambia V4");

            cg::RasterCanvas testCanvas(cg::GRID_WIDTH, cg::GRID_HEIGHT);
            cg::renderExperimentToCanvas(testCanvas, scene, findPreset("AREA_COMP"), true, true);
            std::size_t paintedCount = 0;
            for (bool occ : testCanvas.occupied()) {
                if (occ) ++paintedCount;
            }
            requireCheck(paintedCount > 200, "RasterCanvas 160x120 debe rasterizar celdas");
            std::cout << "[PASS] Modo libre y RasterCanvas 160x120: " << paintedCount
                      << " celdas rasterizadas correctamente.\n";
        }

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
