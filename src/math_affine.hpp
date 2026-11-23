#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
