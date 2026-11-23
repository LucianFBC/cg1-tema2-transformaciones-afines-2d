#pragma once

#include "math_affine.hpp"
#include "raster.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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
