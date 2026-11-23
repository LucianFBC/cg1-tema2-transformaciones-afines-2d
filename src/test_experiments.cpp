#include "experiments.hpp"
#include "math_affine.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// ============================================================================
// Validacion numerica automatizada (headless) de E1-E5 y propiedades afines.
// Exporta tablas reproducibles a ../experimentos/ para mantener coherencia
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

        // 0. Verificar escena base
        const double baseArea = cg::signedArea(scene.polygon);
        require(std::abs(baseArea - 10.0) < 1e-9, "Area base del poligono debe ser +10.0");
        const auto basePar = cg::measureParallelism(scene.seg1, scene.seg2);
        require(basePar.parallel && !basePar.coincident, "L1 y L2 deben ser paralelos distintos");

        std::ofstream txtOut(outDir + "/resumen_experimentos.txt");
        std::ofstream jsonOut(outDir + "/resultados_e1_e5.json");
        jsonOut << "{\n  \"base_polygon\": [";
        for (std::size_t i = 0; i < scene.polygon.size(); ++i) {
            if (i > 0) jsonOut << ", ";
            writeJsonPoint(jsonOut, scene.polygon[i]);
        }
        jsonOut << "],\n  \"base_area_signed\": " << std::fixed << std::setprecision(6) << baseArea
                << ",\n  \"experiments\": [\n";

        cg::TransformReport repE1{}, repE2{}, repE3a{}, repE3b{}, repE4{}, repE5{}, repRef30{};

        bool firstJson = true;
        for (const auto& exp : presets) {
            if (exp.id == "E3") continue;  // E3a y E3b se reportan individualmente y luego se contrastan
            const cg::Matrix3 M = cg::composeInApplicationOrder(exp.steps);
            const auto rep = cg::evaluateTransform(scene, M);
            cg::printReportToStream(txtOut, scene, exp.title, exp.steps, rep);

            if (exp.id == "E1") repE1 = rep;
            else if (exp.id == "E2") repE2 = rep;
            else if (exp.id == "E3a") repE3a = rep;
            else if (exp.id == "E3b") repE3b = rep;
            else if (exp.id == "E4") repE4 = rep;
            else if (exp.id == "E5") repE5 = rep;
            else if (exp.id == "REF30") repRef30 = rep;

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
                    << "      \"edge_v0v1_orig\": " << rep.origEdgeV0V1 << ",\n"
                    << "      \"edge_v0v1_trans\": " << rep.transEdgeV0V1 << ",\n"
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

        // Validaciones matematicas estrictas de E1-E5
        // E1: Hx(0.7) -> det=1, area=10, paralelismo exacto, angulo V0 alterado (ya no 90 deg)
        require(std::abs(repE1.detA - 1.0) < 1e-9, "E1: det(Hx) debe ser 1");
        require(std::abs(repE1.transSignedArea - 10.0) < 1e-9, "E1: area debe conservarse (+10)");
        require(repE1.transParallel.parallel && !repE1.transParallel.coincident,
                "E1: L1' y L2' deben seguir siendo paralelos distintos");
        require(std::abs(repE1.transAngleAtV0 - 90.0) > 10.0,
                "E1: el cizallamiento debe alterar el angulo recto en V0");

        // E2: Ref_y -> det=-1, area_signo=-10, area_abs=10, orientacion invertida
        require(std::abs(repE2.detA + 1.0) < 1e-9, "E2: det(Ref_y) debe ser -1");
        require(std::abs(repE2.transSignedArea + 10.0) < 1e-9, "E2: area con signo debe ser -10");
        require(std::abs(repE2.transAbsArea - 10.0) < 1e-9, "E2: area absoluta debe ser 10");

        // E3: R(45)*Hx(0.7) != Hx(0.7)*R(45)
        const cg::Point v4_a = repE3a.transPoly[scene.highlightVertexIndex];
        const cg::Point v4_b = repE3b.transPoly[scene.highlightVertexIndex];
        const double distE3 = cg::distanceBetween(v4_a, v4_b);
        require(distE3 > 1.0, "E3: ambos ordenes deben producir resultados claramente distintos en V4");
        txtOut << "\n========================================================================\n"
               << "  CONTRASTE E3 (No conmutatividad en V4 = (4, 5))\n"
               << "========================================================================\n"
               << "R(45)*Hx(0.7) * V4 = " << cg::formatPoint(v4_a, 6) << "\n"
               << "Hx(0.7)*R(45) * V4 = " << cg::formatPoint(v4_b, 6) << "\n"
               << "Diferencia euclidiana ||p1 - p2|| = " << std::fixed << std::setprecision(6) << distE3 << "\n";

        // E4: |det(A)| = 1.2 != 1, Area' == |det(A)| * Area_orig
        require(std::abs(std::abs(repE4.detA) - 1.2) < 1e-9, "E4: |det(A)| debe ser 1.2");
        require(repE4.areaAbsError < 1e-9, "E4: Area' debe coincidir con |det(A)| * Area_orig");
        cg::Matrix3 invE4{};
        require(cg::tryInvertAffine(repE4.M, invE4), "E4: debe ser invertible");
        const cg::Matrix3 prodId = cg::multiplyMatrix(invE4, repE4.M);
        require(std::abs(prodId.a[0][0] - 1.0) < 1e-9 && std::abs(prodId.a[1][1] - 1.0) < 1e-9 &&
                std::abs(prodId.a[0][2]) < 1e-9 && std::abs(prodId.a[1][2]) < 1e-9,
                "E4: M^-1 * M debe ser la identidad");

        // E5: S(1, 0) -> det=0, singular, area=0, NO invertible
        require(repE5.singular && !repE5.invertible, "E5: debe detectarse como matriz singular");
        require(std::abs(repE5.detA) < 1e-9, "E5: det(A) debe ser 0");
        require(std::abs(repE5.transAbsArea) < 1e-9, "E5: area debe colapsar a 0");
        cg::Matrix3 dummyInv{};
        require(!cg::tryInvertAffine(repE5.M, dummyInv), "E5: tryInvertAffine debe rechazar matriz singular");
        require(repE5.transParallel.coincident, "E5: L1' y L2' colapsan a la misma recta y = 0");

        // Auxiliar: Ref_30 = R(30) * Ref_x * R(-30) coincide con formula cerrada [cos(60), sin(60); sin(60), -cos(60)]
        const double c60 = std::cos(60.0 * cg::PI / 180.0);
        const double s60 = std::sin(60.0 * cg::PI / 180.0);
        require(std::abs(repRef30.M.a[0][0] - c60) < 1e-9 &&
                std::abs(repRef30.M.a[0][1] - s60) < 1e-9 &&
                std::abs(repRef30.M.a[1][0] - s60) < 1e-9 &&
                std::abs(repRef30.M.a[1][1] + c60) < 1e-9,
                "REF30: R(30)*Ref_x*R(-30) debe coincidir con la matriz analitica de reflexion");

        jsonOut << "\n  ],\n  \"e3_v4_comparison\": {\n"
                << "    \"v4_orig\": ";
        writeJsonPoint(jsonOut, scene.polygon[scene.highlightVertexIndex]);
        jsonOut << ",\n    \"v4_R_Hx\": ";
        writeJsonPoint(jsonOut, v4_a);
        jsonOut << ",\n    \"v4_Hx_R\": ";
        writeJsonPoint(jsonOut, v4_b);
        jsonOut << ",\n    \"euclidean_diff\": " << std::fixed << std::setprecision(6) << distE3
                << "\n  }\n}\n";

        std::cout << "[OK] Todas las validaciones matematicas E1-E5 y Ref_phi pasaron (error < 1e-9).\n"
                  << "[OK] Resultados exportados en: " << outDir << "/resumen_experimentos.txt y resultados_e1_e5.json\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}
