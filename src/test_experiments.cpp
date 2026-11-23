#include "experiments.hpp"
#include "math_affine.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

// ============================================================================
// Validacion numerica automatizada (headless) de los casos de prueba afines.
// Exporta resultados reproducibles a ../experimentos/ para mantener coherencia
// exacta entre el programa C++, el informe LaTeX y la presentacion.
// ============================================================================

namespace {

void require(bool cond, const std::string& msg) {
    if (!cond) {
        throw std::runtime_error("FALLO DE VALIDACION: " + msg);
    }
}

void writeJsonPoint(std::ostream& os, const cg::Point& p) {
    os << std::fixed << std::setprecision(6)
       << "[" << cg::cleanZero(p.x) << ", " << cg::cleanZero(p.y) << "]";
}

void writeJsonMatrix(std::ostream& os, const cg::Matrix3& M) {
    os << std::fixed << std::setprecision(6) << "[";
    for (int r = 0; r < 3; ++r) {
        if (r > 0) os << ", ";
        os << "[" << cg::cleanZero(M.a[r][0]) << ", "
           << cg::cleanZero(M.a[r][1]) << ", "
           << cg::cleanZero(M.a[r][2]) << "]";
    }
    os << "]";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const std::string outDir = (argc >= 2) ? std::string(argv[1]) : "../experimentos";
        std::filesystem::create_directories(outDir);

        const cg::BaseScene scene;
        const auto presets = cg::buildPresetExperiments();

        // 0. Verificar escena base (area exacta +24.0 u^2)
        const double baseArea = cg::signedArea(scene.polygon);
        require(std::abs(baseArea - 24.0) < 1e-9, "Area base del poligono debe ser +24.0");
        const auto basePar = cg::measureParallelism(scene.seg1, scene.seg2);
        require(basePar.parallel && !basePar.coincident, "L1 y L2 deben ser paralelos distintos");

        std::ofstream txtOut(outDir + "/resumen_experimentos.txt");
        std::ofstream jsonOut(outDir + "/resultados_experimentos.json");
        jsonOut << "{\n  \"grid_width\": " << cg::GRID_WIDTH
                << ",\n  \"grid_height\": " << cg::GRID_HEIGHT
                << ",\n  \"px_per_unit\": " << cg::PX_PER_UNIT
                << ",\n  \"window_width\": " << cg::WINDOW_WIDTH
                << ",\n  \"window_height\": " << cg::WINDOW_HEIGHT
                << ",\n  \"base_polygon\": [";
        for (std::size_t i = 0; i < scene.polygon.size(); ++i) {
            if (i > 0) jsonOut << ", ";
            writeJsonPoint(jsonOut, scene.polygon[i]);
        }
        jsonOut << "],\n  \"base_area_signed\": " << std::fixed << std::setprecision(6) << baseArea
                << ",\n  \"experiments\": [\n";

        cg::TransformReport repShear{}, repRefY{}, repRH{}, repHR{}, repArea{}, repSing{}, repRef30{};

        bool firstJson = true;
        for (const auto& exp : presets) {
            if (exp.id == "ORDEN_COMP") continue;
            const cg::Matrix3 M = cg::composeInApplicationOrder(exp.steps);
            const auto rep = cg::evaluateTransform(scene, M);
            cg::printReportToStream(txtOut, scene, exp.title, exp.steps, rep);

            if (exp.id == "SHEAR_X") repShear = rep;
            else if (exp.id == "REF_Y") repRefY = rep;
            else if (exp.id == "ORDEN_RH") repRH = rep;
            else if (exp.id == "ORDEN_HR") repHR = rep;
            else if (exp.id == "AREA_COMP") repArea = rep;
            else if (exp.id == "SINGULAR") repSing = rep;
            else if (exp.id == "REF_30") repRef30 = rep;

            if (!firstJson) jsonOut << ",\n";
            firstJson = false;
            jsonOut << "    {\n"
                    << "      \"id\": \"" << exp.id << "\",\n"
                    << "      \"title\": \"" << exp.title << "\",\n"
                    << "      \"application_order\": \"" << cg::formatApplicationOrder(exp.steps) << "\",\n"
                    << "      \"matrix_product\": \"" << cg::formatMatrixProductOrder(exp.steps) << "\",\n"
                    << "      \"matrix\": ";
            writeJsonMatrix(jsonOut, rep.M);
            jsonOut << ",\n"
                    << "      \"detA\": " << std::fixed << std::setprecision(6) << cg::cleanZero(rep.detA) << ",\n"
                    << "      \"singular\": " << (rep.singular ? "true" : "false") << ",\n"
                    << "      \"orig_signed_area\": " << cg::cleanZero(rep.origSignedArea) << ",\n"
                    << "      \"trans_signed_area\": " << cg::cleanZero(rep.transSignedArea) << ",\n"
                    << "      \"trans_abs_area\": " << cg::cleanZero(rep.transAbsArea) << ",\n"
                    << "      \"expected_abs_area\": " << cg::cleanZero(rep.expectedAbsArea) << ",\n"
                    << "      \"area_abs_error\": " << std::scientific << std::setprecision(3) << rep.areaAbsError
                    << std::fixed << std::setprecision(6) << ",\n"
                    << "      \"orig_orientation\": \"" << rep.origOrientation << "\",\n"
                    << "      \"trans_orientation\": \"" << rep.transOrientation << "\",\n"
                    << "      \"parallel_sine\": " << cg::cleanZero(rep.transParallel.normalizedSine) << ",\n"
                    << "      \"parallel_angle_deg\": " << cg::cleanZero(rep.transParallel.angleDegrees) << ",\n"
                    << "      \"parallel_sep\": " << cg::cleanZero(rep.transParallel.lineSeparation) << ",\n"
                    << "      \"parallel_status\": \"" << rep.transParallel.status << "\",\n"
                    << "      \"angle_v0_orig\": " << rep.origAngleAtV0 << ",\n"
                    << "      \"angle_v0_trans\": " << rep.transAngleAtV0 << ",\n"
                    << "      \"angle_v0_defined\": " << (rep.transAngleAtV0Defined ? "true" : "false") << ",\n"
                    << "      \"edge_v0v1_orig\": " << rep.origEdgeV0V1 << ",\n"
                    << "      \"edge_v0v1_trans\": " << rep.transEdgeV0V1 << ",\n"
                    << "      \"edge_v5v0_orig\": " << rep.origEdgeV5V0 << ",\n"
                    << "      \"edge_v5v0_trans\": " << rep.transEdgeV5V0 << ",\n"
                    << "      \"trans_polygon\": [";
            for (std::size_t i = 0; i < rep.transPoly.size(); ++i) {
                if (i > 0) jsonOut << ", ";
                writeJsonPoint(jsonOut, rep.transPoly[i]);
            }
            jsonOut << "],\n      \"trans_seg1\": [";
            writeJsonPoint(jsonOut, rep.transSeg1.p);
            jsonOut << ", ";
            writeJsonPoint(jsonOut, rep.transSeg1.q);
            jsonOut << "],\n      \"trans_seg2\": [";
            writeJsonPoint(jsonOut, rep.transSeg2.p);
            jsonOut << ", ";
            writeJsonPoint(jsonOut, rep.transSeg2.q);
            jsonOut << "]\n    }";
        }

        // Validaciones matematicas estrictas
        require(std::abs(repShear.detA - 1.0) < 1e-9, "Cizallamiento: det(Hx) debe ser 1");
        require(std::abs(repShear.transSignedArea - 24.0) < 1e-9, "Cizallamiento: area debe ser +24");
        require(repShear.transParallel.parallel && !repShear.transParallel.coincident,
                "Cizallamiento: L1' y L2' deben seguir siendo paralelos distintos");
        require(std::abs(repShear.transAngleAtV0 - 90.0) > 10.0,
                "Cizallamiento: el angulo recto en V0 debe alterarse");
        require(std::abs(repShear.transEdgeV5V0 - std::hypot(3.5, 5.0)) < 1e-9,
                "Cizallamiento: la arista vertical V5-V0 debe alargarse a sqrt(37.25)");

        require(std::abs(repRefY.detA + 1.0) < 1e-9, "Reflexion vertical: det(Ref_y) debe ser -1");
        require(std::abs(repRefY.transSignedArea + 24.0) < 1e-9, "Reflexion vertical: area signo = -24");
        require(std::abs(repRefY.transAbsArea - 24.0) < 1e-9, "Reflexion vertical: |area| = 24");

        const cg::Point v4_a = repRH.transPoly[scene.highlightVertexIndex];
        const cg::Point v4_b = repHR.transPoly[scene.highlightVertexIndex];
        const double distOrden = cg::distanceBetween(v4_a, v4_b);
        require(distOrden > 1.0, "Orden: R*Hx y Hx*R deben producir resultados distintos en V4");
        txtOut << "\n========================================================================\n"
               << "  CONTRASTE DE ORDEN EN V4 = (5, 7)\n"
               << "========================================================================\n"
               << "R(45)*Hx(0.7) * V4 = " << cg::formatPoint(v4_a, 6) << "\n"
               << "Hx(0.7)*R(45) * V4 = " << cg::formatPoint(v4_b, 6) << "\n"
               << "Diferencia euclidiana ||p1 - p2|| = " << std::fixed << std::setprecision(6) << distOrden << "\n";

        require(std::abs(std::abs(repArea.detA) - 1.2) < 1e-9, "Cambio de area: |det(A)| debe ser 1.2");
        require(repArea.areaAbsError < 1e-9, "Cambio de area: Area' == |det(A)| * Area_orig");
        cg::Matrix3 invArea{};
        require(cg::tryInvertAffine(repArea.M, invArea), "Cambio de area: debe ser invertible");
        const cg::Matrix3 prodId = cg::multiplyMatrix(invArea, repArea.M);
        require(std::abs(prodId.a[0][0] - 1.0) < 1e-9 && std::abs(prodId.a[1][1] - 1.0) < 1e-9 &&
                std::abs(prodId.a[0][2]) < 1e-9 && std::abs(prodId.a[1][2]) < 1e-9,
                "Cambio de area: M^-1 * M debe ser la identidad");

        require(repSing.singular && !repSing.invertible, "Caso singular: debe detectarse como singular");
        require(std::abs(repSing.detA) < 1e-9, "Caso singular: det(A) debe ser 0");
        require(std::abs(repSing.transAbsArea) < 1e-9, "Caso singular: area debe ser 0");
        cg::Matrix3 dummyInv{};
        require(!cg::tryInvertAffine(repSing.M, dummyInv), "Caso singular: tryInvertAffine debe rechazarla");
        require(!repSing.transAngleAtV0Defined, "Caso singular: angulo en V0 debe ser indefinido");
        require(repSing.transParallel.coincident, "Caso singular: L1' y L2' colapsan sobre y = 0");

        const double c60 = std::cos(60.0 * cg::PI / 180.0);
        const double s60 = std::sin(60.0 * cg::PI / 180.0);
        require(std::abs(repRef30.M.a[0][0] - c60) < 1e-9 &&
                std::abs(repRef30.M.a[0][1] - s60) < 1e-9 &&
                std::abs(repRef30.M.a[1][0] - s60) < 1e-9 &&
                std::abs(repRef30.M.a[1][1] + c60) < 1e-9,
                "REF_30: R(30)*Ref_x*R(-30) debe coincidir con la matriz analitica");

        jsonOut << "\n  ],\n  \"order_v4_comparison\": {\n"
                << "    \"v4_orig\": ";
        writeJsonPoint(jsonOut, scene.polygon[scene.highlightVertexIndex]);
        jsonOut << ",\n    \"v4_R_Hx\": ";
        writeJsonPoint(jsonOut, v4_a);
        jsonOut << ",\n    \"v4_Hx_R\": ";
        writeJsonPoint(jsonOut, v4_b);
        jsonOut << ",\n    \"euclidean_diff\": " << std::fixed << std::setprecision(6) << distOrden
                << "\n  }\n}\n";

        std::cout << "[OK] Todas las validaciones matematicas pasaron (error < 1e-9).\n"
                  << "[OK] Resultados exportados en: " << outDir
                  << "/resumen_experimentos.txt y resultados_experimentos.json\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}
