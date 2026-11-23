#include <GLFW/glfw3.h>

#include "experiments.hpp"
#include "math_affine.hpp"
#include "raster.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
// Laboratorio Interactivo de Transformaciones Afines 2D
// UNSAAC - Computacion Grafica I (IF455AIN) - Semestre 2026-02
//
// Estilo visual basado en el entorno de laboratorio del curso:
//   - Rejilla discreta 40 x 30 (ventana 1000 x 750) con pixeles visibles
//     (margen m = 0.08) y opcion G para mostrar u ocultar la rejilla.
//   - Rasterizacion propia mediante putPixel()/putPoint(), Bresenham y
//     drawPolygon() sin glTranslatef/glRotatef/glScalef ni GL_POLYGON.
//   - Panel compacto en consola que limpia la salida anterior y muestra
//     siempre el menu de opciones seguido de la informacion en vivo.
// ============================================================================

namespace {

cg::BaseScene gScene;
cg::RasterCanvas gCanvas(cg::GRID_WIDTH, cg::GRID_HEIGHT);

bool gShowPixelGrid = true;         // Tecla G: rejilla de pixeles drawGrid()
bool gShowTransformedLattice = true;// Tecla M: cuadricula local transformada
bool gShowOriginal = true;          // Tecla H: figura original de referencia
int gMode = 0;                      // 0..8 o 9 (cadena personalizada tecla C)

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

// Limpia la consola y redibuja el menu compacto + estado en vivo de la opcion activa
void renderTerminalDashboard() {
    // Secuencia ANSI: limpia pantalla y coloca cursor en (1,1)
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
            double x, y; std::cout << "  tx ty: "; std::cin >> x >> y;
            chain.push_back({"T(" + fmtNum(x, 1) + "," + fmtNum(y, 1) + ")", cg::translationMatrix(x, y)});
        } else if (op == 2) {
            double d; std::cout << "  grados: "; std::cin >> d;
            chain.push_back({"R(" + fmtNum(d, 1) + ")", cg::rotationMatrix(d)});
        } else if (op == 3) {
            double sx, sy; std::cout << "  sx sy: "; std::cin >> sx >> sy;
            chain.push_back({"S(" + fmtNum(sx) + "," + fmtNum(sy) + ")", cg::scalingMatrix(sx, sy)});
        } else if (op == 4) {
            double k; std::cout << "  k: "; std::cin >> k;
            chain.push_back({"Hx(" + fmtNum(k) + ")", cg::shearXMatrix(k)});
        } else if (op == 5) {
            double k; std::cout << "  k: "; std::cin >> k;
            chain.push_back({"Hy(" + fmtNum(k) + ")", cg::shearYMatrix(k)});
        } else if (op == 6) {
            chain.push_back({"Ref_x", cg::reflectionXMatrix()});
        } else if (op == 7) {
            chain.push_back({"Ref_y", cg::reflectionYMatrix()});
        } else if (op == 8) {
            double phi; std::cout << "  phi (grados): "; std::cin >> phi;
            chain.push_back({"Ref(" + fmtNum(phi, 1) + ")", cg::reflectionLineMatrix(phi)});
        }
    }

    gCustomChain = std::move(chain);
    gMode = 9;
    renderTerminalDashboard();
}

// Rejilla de pixeles del curso (drawGrid), activable con G
void drawPixelGridOpenGL() {
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

// Dibuja exclusivamente las celdas logicas pintadas por putPoint() con margen m = 0.08
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
        if (!canvas.savePPM(filePath, true, 25)) {
            std::cerr << "Error guardando " << filePath << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Exportado raster real 40x30 (con rejilla): " << filePath << "\n";
    }
    return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char** argv) {
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
