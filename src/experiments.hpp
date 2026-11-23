#pragma once

#include "math_affine.hpp"
#include "raster.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
// Escena base y definicion reproducible de los 5 experimentos obligatorios
// (E1-E5) + caso auxiliar de reflexion sobre recta oblicua (Sec. 2.3.3).
// ============================================================================

namespace cg {

struct BaseScene {
    // Poligono asimetrico de 6 vertices (concavo, sin ejes de simetria).
    // Area exacta con signo (zapatero): +10.0 u^2 (recorrido antihorario CCW).
    std::vector<Point> polygon = {
        {1.0, 1.0},  // V0
        {5.0, 1.0},  // V1
        {5.0, 2.0},  // V2
        {3.0, 2.0},  // V3 (vertice concavo)
        {4.0, 5.0},  // V4 (vertice seleccionado para analisis cuantitativo)
        {1.0, 4.0}   // V5
    };
    std::vector<std::string> vertexNames = {"V0", "V1", "V2", "V3", "V4", "V5"};
    std::size_t highlightVertexIndex = 4;  // V4 = (4, 5)

    // Dos segmentos originalmente paralelos con vector director u = v = (4, 2)
    Segment seg1 = {{1.0, -1.0}, {5.0,  1.0}, "L1"};
    Segment seg2 = {{1.0, -3.0}, {5.0, -1.0}, "L2"};

    // Limites de la cuadricula local transformable
    int gridXMin = 0;
    int gridXMax = 6;
    int gridYMin = -4;
    int gridYMax = 6;
};

struct ExperimentConfig {
    std::string id;
    std::string title;
    std::string description;
    std::vector<NamedTransform> steps;
    bool hasSecondBranch{false};  // Usado en E3 comparativo
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

    // Angulo en V0 entre V5-V0-V1 (originalmente 90 grados) y longitud de V0-V1 (4.0)
    r.origAngleAtV0 = interiorAngleDegrees(scene.polygon[5], scene.polygon[0], scene.polygon[1]);
    r.transAngleAtV0 = interiorAngleDegrees(r.transPoly[5], r.transPoly[0], r.transPoly[1]);
    r.origEdgeV0V1 = distanceBetween(scene.polygon[0], scene.polygon[1]);
    r.transEdgeV0V1 = distanceBetween(r.transPoly[0], r.transPoly[1]);
    return r;
}

inline std::vector<ExperimentConfig> buildPresetExperiments() {
    return {
        {
            "E0",
            "Escena base (Identidad)",
            "Poligono asimetrico de 6 vertices, cuadricula local y dos segmentos paralelos L1, L2.",
            {}, false, {}, false, 0.0
        },
        {
            "E1",
            "E1 - Cizallamiento horizontal Hx(0.7)",
            "Aplica Hx(0.7) y verifica con coordenadas la conservacion del paralelismo y del area (det = 1), pese a alterar angulos y longitudes.",
            {{"Hx(0.7)", shearXMatrix(0.7)}}, false, {}, false, 0.0
        },
        {
            "E2",
            "E2 - Reflexion respecto al eje vertical (Ref_y)",
            "Refleja x -> -x; compara area con signo, area absoluta y cambio de orientacion CCW -> CW (det = -1).",
            {{"Ref_y", reflectionYMatrix()}}, false, {}, false, 0.0
        },
        {
            "E3a",
            "E3a - Orden 1: R(45) * Hx(0.7)",
            "Aplica primero Hx(0.7) y luego R(45 grados).",
            {{"Hx(0.7)", shearXMatrix(0.7)}, {"R(45)", rotationMatrix(45.0)}}, false, {}, false, 0.0
        },
        {
            "E3b",
            "E3b - Orden 2: Hx(0.7) * R(45)",
            "Aplica primero R(45 grados) y luego Hx(0.7).",
            {{"R(45)", rotationMatrix(45.0)}, {"Hx(0.7)", shearXMatrix(0.7)}}, false, {}, false, 0.0
        },
        {
            "E3",
            "E3 - Comparacion de orden: R(45)*Hx(0.7) vs Hx(0.7)*R(45)",
            "Superpone ambos ordenes (azul: R*Hx, naranja: Hx*R) y cuantifica la diferencia en V4 = (4, 5).",
            {{"Hx(0.7)", shearXMatrix(0.7)}, {"R(45)", rotationMatrix(45.0)}},
            true,
            {{"R(45)", rotationMatrix(45.0)}, {"Hx(0.7)", shearXMatrix(0.7)}},
            false, 0.0
        },
        {
            "E4",
            "E4 - Cambio de area (composicion de 4 transformaciones, |det| = 1.2)",
            "M = T(1.5,-1.0) * R(25) * S(1.5,0.8) * Hx(0.5); contrasta Area' con |det(A)| * Area_orig.",
            {
                {"Hx(0.5)", shearXMatrix(0.5)},
                {"S(1.5, 0.8)", scalingMatrix(1.5, 0.8)},
                {"R(25)", rotationMatrix(25.0)},
                {"T(1.5, -1.0)", translationMatrix(1.5, -1.0)}
            },
            false, {}, false, 0.0
        },
        {
            "E5",
            "E5 - Caso degenerado / matriz singular S(1.0, 0.0)",
            "Escalamiento con sy = 0 (det(A) = 0): colapso al eje X, area nula y perdida de invertibilidad.",
            {{"S(1.0, 0.0)", scalingMatrix(1.0, 0.0)}}, false, {}, false, 0.0
        },
        {
            "REF30",
            "Auxiliar - Reflexion sobre recta por el origen (phi = 30 grados)",
            "Demostracion de Ref_phi = R(30) * Ref_x * R(-30) sobre el eje morado y = tan(30)*x.",
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

    os << "\n--- Angulos y longitudes (no invariantes en general) ---\n";
    os << "Angulo interno en V0 (V5-V0-V1): " << rep.origAngleAtV0
       << " deg -> " << rep.transAngleAtV0 << " deg\n";
    os << "Longitud arista V0-V1          : " << rep.origEdgeV0V1
       << " -> " << rep.transEdgeV0V1 << "\n";

    os << "\n--- Vertices del poligono ---\n";
    for (std::size_t i = 0; i < scene.polygon.size(); ++i) {
        os << "  " << scene.vertexNames[i] << ": "
           << std::setw(18) << formatPoint(scene.polygon[i], 3)
           << "  -->  " << formatPoint(rep.transPoly[i], 4);
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
                                     bool showOriginal, bool showGrid) {
    canvas.clear(COL_BG);

    const Matrix3 I = identityMatrix();
    const Matrix3 M1 = composeInApplicationOrder(exp.steps);

    // 1. Cuadriculas (original punteada/tenue y transformada solida tenue)
    if (showGrid) {
        if (showOriginal) {
            canvas.drawTransformedGrid(I, scene.gridXMin, scene.gridXMax,
                                       scene.gridYMin, scene.gridYMax,
                                       COL_GRID_ORIG, LineStyle::Dotted);
        }
        if (!exp.steps.empty()) {
            canvas.drawTransformedGrid(M1, scene.gridXMin, scene.gridXMax,
                                       scene.gridYMin, scene.gridYMax,
                                       COL_GRID_TRANS, LineStyle::Solid);
        }
    }

    // 2. Ejes cartesianos X, Y
    canvas.drawCartesianAxes();

    // 2b. Recta de reflexion oblicua (si aplica)
    if (exp.drawReflectionAxis) {
        const double rad = exp.reflectionAxisDeg * (PI / 180.0);
        const double L = 16.0;
        canvas.drawWorldLine({-L * std::cos(rad), -L * std::sin(rad)},
                             { L * std::cos(rad),  L * std::sin(rad)},
                             COL_REF_LINE, LineStyle::Dashed, 0);
    }

    // 3. Figura y segmentos originales (estilo discontinuo)
    if (showOriginal) {
        canvas.drawWorldLine(scene.seg1.p, scene.seg1.q, COL_SEG_ORIG, LineStyle::Dashed, 0);
        canvas.drawWorldLine(scene.seg2.p, scene.seg2.q, COL_SEG_ORIG, LineStyle::Dashed, 0);
        canvas.drawPolygon(scene.polygon, COL_POLY_ORIG, LineStyle::Dashed, 0);
        canvas.drawVertexMarkers(scene.polygon, COL_VERTEX_ORIG, 1);
    }

    // 4. Figura y segmentos transformados (rama principal, estilo solido reforzado)
    if (!exp.steps.empty()) {
        const auto poly1 = transformPolygon(M1, scene.polygon);
        const auto s1 = transformSegment(M1, scene.seg1);
        const auto s2 = transformSegment(M1, scene.seg2);

        canvas.drawWorldLine(s1.p, s1.q, COL_SEG_TRANS, LineStyle::Solid, 1);
        canvas.drawWorldLine(s2.p, s2.q, COL_SEG_TRANS, LineStyle::Solid, 1);
        canvas.drawPolygon(poly1, COL_POLY_TRANS, LineStyle::Solid, 1);
        canvas.drawVertexMarkers(poly1, COL_VERTEX_TRANS, 2);
    }

    // 5. Segunda rama comparativa (para E3: Hx*R vs R*Hx)
    if (exp.hasSecondBranch) {
        const Matrix3 M2 = composeInApplicationOrder(exp.secondSteps);
        const auto poly2 = transformPolygon(M2, scene.polygon);
        const auto s1b = transformSegment(M2, scene.seg1);
        const auto s2b = transformSegment(M2, scene.seg2);

        canvas.drawWorldLine(s1b.p, s1b.q, COL_SEG_ALT, LineStyle::Solid, 0);
        canvas.drawWorldLine(s2b.p, s2b.q, COL_SEG_ALT, LineStyle::Solid, 0);
        canvas.drawPolygon(poly2, COL_POLY_ALT, LineStyle::Solid, 1);
        canvas.drawVertexMarkers(poly2, COL_POLY_ALT, 2);
    }
}

}  // namespace cg
