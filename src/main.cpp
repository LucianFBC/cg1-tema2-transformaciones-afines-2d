#include <GLFW/glfw3.h>

#include "experiments.hpp"
#include "math_affine.hpp"
#include "raster.hpp"

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
// UNSAAC - Computacion Grafica I (IF455AIN) - Semestre 2026-02 - Tema 2
// Integrantes:
//   - CHOQUENAIRA GARCIA, Ronal Franklin (174941)
//   - FERNANDEZ BACA CASTRO, Lucian Neptali (184197)
//
// Reglas estrictas cumplidas:
//   - Matrices 3x3, concatenacion y transformacion implementadas en math_affine.hpp
//   - Rasterizacion propia con putPoint(), Bresenham y drawPolygon() en raster.hpp
//   - Prohibido glTranslatef / glRotatef / glScalef / glMultMatrix / GL_POLYGON
// ============================================================================

namespace {

constexpr int WINDOW_WIDTH  = 1120;
constexpr int WINDOW_HEIGHT = 840;

cg::BaseScene gScene;
cg::RasterCanvas gCanvas(cg::GRID_WIDTH, cg::GRID_HEIGHT);
std::vector<cg::ExperimentConfig> gPresets;

bool gShowOriginal = true;
bool gShowGrid = true;
bool gInteractiveMode = false;
int gPresetIndex = 0;

// Estado interactivo editable (concatena >= 3 transformaciones en vivo)
struct InteractiveParams {
    double shearX = 0.7;
    double shearY = 0.0;
    double scaleX = 1.2;
    double scaleY = 0.9;
    double rotDeg = 30.0;
    double tx = 1.0;
    double ty = -0.5;
    bool reflectY = false;
    bool reverseOrder = false;
} gParams;

// Cadena personalizada construida desde consola (tecla C)
bool gUseCustomChain = false;
std::vector<cg::NamedTransform> gCustomChain;

std::string fmtNum(double v, int prec = 2) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(prec) << cg::cleanZero(v);
    return oss.str();
}

cg::ExperimentConfig buildInteractiveConfig() {
    cg::ExperimentConfig cfg;
    if (gUseCustomChain) {
        cfg.id = "CUSTOM";
        cfg.title = "Cadena personalizada (tecla C)";
        cfg.description = "Transformacion compuesta definida interactivamente por el usuario.";
        cfg.steps = gCustomChain;
        return cfg;
    }

    cfg.id = "LIVE";
    cfg.title = gParams.reverseOrder
                    ? "Modo interactivo en vivo (ORDEN INVERTIDO)"
                    : "Modo interactivo en vivo (Hx -> Hy -> S -> Ref? -> R -> T)";
    cfg.description = "Concatena transformaciones afines en tiempo real con teclado.";

    std::vector<cg::NamedTransform> steps;
    steps.push_back({"Hx(" + fmtNum(gParams.shearX) + ")", cg::shearXMatrix(gParams.shearX)});
    if (std::abs(gParams.shearY) > 1e-6) {
        steps.push_back({"Hy(" + fmtNum(gParams.shearY) + ")", cg::shearYMatrix(gParams.shearY)});
    }
    steps.push_back({"S(" + fmtNum(gParams.scaleX) + "," + fmtNum(gParams.scaleY) + ")",
                     cg::scalingMatrix(gParams.scaleX, gParams.scaleY)});
    if (gParams.reflectY) {
        steps.push_back({"Ref_y", cg::reflectionYMatrix()});
    }
    steps.push_back({"R(" + fmtNum(gParams.rotDeg, 1) + ")", cg::rotationMatrix(gParams.rotDeg)});
    steps.push_back({"T(" + fmtNum(gParams.tx) + "," + fmtNum(gParams.ty) + ")",
                     cg::translationMatrix(gParams.tx, gParams.ty)});

    if (gParams.reverseOrder) {
        std::reverse(steps.begin(), steps.end());
    }
    cfg.steps = std::move(steps);
    return cfg;
}

const cg::ExperimentConfig activeConfig() {
    if (gInteractiveMode) return buildInteractiveConfig();
    return gPresets[static_cast<std::size_t>(gPresetIndex)];
}

void printHelp() {
    std::cout << "\n================ CONTROLES DEL LABORATORIO (TEMA 2) ================\n"
              << "Presets obligatorios:\n"
              << "  0 : Escena base (Identidad)\n"
              << "  1 : E1 - Cizallamiento horizontal Hx(0.7)\n"
              << "  2 : E2 - Reflexion eje vertical Ref_y\n"
              << "  3 : E3a - Orden 1: R(45) * Hx(0.7)\n"
              << "  4 : E3b - Orden 2: Hx(0.7) * R(45)\n"
              << "  5 : E3 - Comparacion simultanea R(45)*Hx(0.7) [azul] vs Hx(0.7)*R(45) [naranja]\n"
              << "  6 : E4 - Cambio de area (4 transformaciones, |det(A)| = 1.2)\n"
              << "  7 : E5 - Caso degenerado singular S(1.0, 0.0) [det(A) = 0]\n"
              << "  8 : Auxiliar - Reflexion sobre recta con angulo phi = 30 grados\n"
              << "Modo interactivo (>= 3 transformaciones en vivo):\n"
              << "  I : Activar/volver a modo interactivo en vivo\n"
              << "  A/D : Cizallamiento Hx (-/+ 0.1)     |  J/L : Cizallamiento Hy (-/+ 0.1)\n"
              << "  Z/X : Escala sx (-/+ 0.1)            |  W/S : Escala sy (-/+ 0.1, llega a 0!)\n"
              << "  Q/E : Rotacion theta (-/+ 5 deg)     |  Flechas : Traslacion (tx, ty)\n"
              << "  F : Alternar Reflexion Ref_y         |  O : Invertir orden de composicion\n"
              << "  C : Constructor en consola (concatenar N >= 3 transformaciones con parametros exactos)\n"
              << "Visualizacion y reporte:\n"
              << "  G : Mostrar/ocultar cuadricula       |  H : Mostrar/ocultar figura original\n"
              << "  P : Imprimir reporte numerico        |  R : Reiniciar parametros interactivos\n"
              << "  ESC : Salir\n"
              << "====================================================================\n";
}

void printCurrentReport() {
    const auto cfg = activeConfig();
    const cg::Matrix3 M1 = cg::composeInApplicationOrder(cfg.steps);
    const auto rep1 = cg::evaluateTransform(gScene, M1);
    cg::printReportToStream(std::cout, gScene, cfg.title, cfg.steps, rep1);

    if (cfg.hasSecondBranch) {
        const cg::Matrix3 M2 = cg::composeInApplicationOrder(cfg.secondSteps);
        const auto rep2 = cg::evaluateTransform(gScene, M2);
        cg::printReportToStream(std::cout, gScene,
                                "Rama 2 (Naranja): Hx(0.7) * R(45)",
                                cfg.secondSteps, rep2);

        const std::size_t k = gScene.highlightVertexIndex;
        const cg::Point v1 = rep1.transPoly[k];
        const cg::Point v2 = rep2.transPoly[k];
        const double diff = cg::distanceBetween(v1, v2);
        std::cout << "\n>>> CUANTIFICACION E3 en vertice " << gScene.vertexNames[k]
                  << " = " << cg::formatPoint(gScene.polygon[k], 2) << ":\n"
                  << "    R(45)*Hx(0.7) -> " << cg::formatPoint(v1, 4) << "\n"
                  << "    Hx(0.7)*R(45) -> " << cg::formatPoint(v2, 4) << "\n"
                  << "    Distancia ||p1 - p2|| = " << std::fixed << std::setprecision(6) << diff << "\n";
    }
}

void runConsoleChainBuilder() {
    std::cout << "\n--- CONSTRUCTOR DE CADENA DE TRANSFORMACIONES (en orden de aplicacion) ---\n"
              << "Ingrese el numero de transformaciones a concatenar (ej. 3 o mas, 0 para cancelar): ";
    int count = 0;
    if (!(std::cin >> count) || count <= 0) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Operacion cancelada.\n";
        return;
    }

    std::vector<cg::NamedTransform> chain;
    for (int i = 0; i < count; ++i) {
        std::cout << "\nTransformacion #" << (i + 1) << ":\n"
                  << "  1: Traslacion T(tx, ty)\n"
                  << "  2: Rotacion R(grados)\n"
                  << "  3: Escalamiento S(sx, sy)\n"
                  << "  4: Cizallamiento horizontal Hx(k)\n"
                  << "  5: Cizallamiento vertical Hy(k)\n"
                  << "  6: Reflexion eje X (Ref_x)\n"
                  << "  7: Reflexion eje Y (Ref_y)\n"
                  << "  8: Reflexion recta por origen con angulo phi (Ref_phi)\n"
                  << "Opcion: ";
        int op = 0;
        if (!(std::cin >> op)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Entrada invalida, cancelado.\n";
            return;
        }
        if (op == 1) {
            double tx, ty;
            std::cout << "  tx ty: ";
            std::cin >> tx >> ty;
            chain.push_back({"T(" + fmtNum(tx) + "," + fmtNum(ty) + ")", cg::translationMatrix(tx, ty)});
        } else if (op == 2) {
            double deg;
            std::cout << "  grados: ";
            std::cin >> deg;
            chain.push_back({"R(" + fmtNum(deg, 1) + ")", cg::rotationMatrix(deg)});
        } else if (op == 3) {
            double sx, sy;
            std::cout << "  sx sy: ";
            std::cin >> sx >> sy;
            chain.push_back({"S(" + fmtNum(sx) + "," + fmtNum(sy) + ")", cg::scalingMatrix(sx, sy)});
        } else if (op == 4) {
            double k;
            std::cout << "  k: ";
            std::cin >> k;
            chain.push_back({"Hx(" + fmtNum(k) + ")", cg::shearXMatrix(k)});
        } else if (op == 5) {
            double k;
            std::cout << "  k: ";
            std::cin >> k;
            chain.push_back({"Hy(" + fmtNum(k) + ")", cg::shearYMatrix(k)});
        } else if (op == 6) {
            chain.push_back({"Ref_x", cg::reflectionXMatrix()});
        } else if (op == 7) {
            chain.push_back({"Ref_y", cg::reflectionYMatrix()});
        } else if (op == 8) {
            double phi;
            std::cout << "  phi (grados): ";
            std::cin >> phi;
            chain.push_back({"Ref(" + fmtNum(phi, 1) + ")", cg::reflectionLineMatrix(phi)});
        } else {
            std::cout << "  Opcion desconocida, se usa Identidad.\n";
            chain.push_back({"I", cg::identityMatrix()});
        }
    }

    gCustomChain = std::move(chain);
    gUseCustomChain = true;
    gInteractiveMode = true;
    printCurrentReport();
}

// Vuelca las celdas del RasterCanvas a la ventana OpenGL 2.1 usando
// exclusivamente celdas logicas individuales (equivalente a putPixel del curso).
void flushCanvasToOpenGL(const cg::RasterCanvas& canvas) {
    glClear(GL_COLOR_BUFFER_BIT);
    const auto& buf = canvas.pixels();
    const int w = canvas.width();
    const int h = canvas.height();

    glBegin(GL_QUADS);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const cg::Color& c = buf[static_cast<std::size_t>(y * w + x)];
            // Saltar celdas exactamente iguales al fondo para acelerar el refresco
            if (c.r == cg::COL_BG.r && c.g == cg::COL_BG.g && c.b == cg::COL_BG.b) continue;
            glColor3f(c.r, c.g, c.b);
            glVertex2f(static_cast<float>(x),     static_cast<float>(y));
            glVertex2f(static_cast<float>(x + 1), static_cast<float>(y));
            glVertex2f(static_cast<float>(x + 1), static_cast<float>(y + 1));
            glVertex2f(static_cast<float>(x),     static_cast<float>(y + 1));
        }
    }
    glEnd();
}

void updateWindowTitle(GLFWwindow* window) {
    const auto cfg = activeConfig();
    std::string title = "UNSAAC CG1 - Tema 2 | " + cfg.title;
    glfwSetWindowTitle(window, title.c_str());
}

void renderFrame() {
    const auto cfg = activeConfig();
    cg::renderExperimentToCanvas(gCanvas, gScene, cfg, gShowOriginal, gShowGrid);
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
        const int idx = key - GLFW_KEY_0;
        if (idx < static_cast<int>(gPresets.size())) {
            gPresetIndex = idx;
            gInteractiveMode = false;
            gUseCustomChain = false;
            changed = true;
        }
    } else if (action == GLFW_PRESS && key == GLFW_KEY_I) {
        gInteractiveMode = true;
        gUseCustomChain = false;
        changed = true;
    } else if (action == GLFW_PRESS && key == GLFW_KEY_G) {
        gShowGrid = !gShowGrid;
        std::cout << "Cuadricula: " << (gShowGrid ? "ON" : "OFF") << "\n";
    } else if (action == GLFW_PRESS && key == GLFW_KEY_H) {
        gShowOriginal = !gShowOriginal;
        std::cout << "Figura original: " << (gShowOriginal ? "ON" : "OFF") << "\n";
    } else if (action == GLFW_PRESS && key == GLFW_KEY_P) {
        printCurrentReport();
    } else if (action == GLFW_PRESS && key == GLFW_KEY_C) {
        runConsoleChainBuilder();
        updateWindowTitle(window);
        return;
    } else if (action == GLFW_PRESS && key == GLFW_KEY_R) {
        gParams = InteractiveParams{};
        gUseCustomChain = false;
        gInteractiveMode = true;
        changed = true;
    } else {
        // Ajustes rapidos en modo interactivo
        auto enterLive = [&]() {
            gInteractiveMode = true;
            gUseCustomChain = false;
            changed = true;
        };
        if (key == GLFW_KEY_A) { gParams.shearX -= 0.1; enterLive(); }
        else if (key == GLFW_KEY_D) { gParams.shearX += 0.1; enterLive(); }
        else if (key == GLFW_KEY_J) { gParams.shearY -= 0.1; enterLive(); }
        else if (key == GLFW_KEY_L) { gParams.shearY += 0.1; enterLive(); }
        else if (key == GLFW_KEY_Z) { gParams.scaleX -= 0.1; enterLive(); }
        else if (key == GLFW_KEY_X) { gParams.scaleX += 0.1; enterLive(); }
        else if (key == GLFW_KEY_S) { gParams.scaleY -= 0.1; enterLive(); }
        else if (key == GLFW_KEY_W) { gParams.scaleY += 0.1; enterLive(); }
        else if (key == GLFW_KEY_Q) { gParams.rotDeg += 5.0; enterLive(); }
        else if (key == GLFW_KEY_E) { gParams.rotDeg -= 5.0; enterLive(); }
        else if (key == GLFW_KEY_LEFT)  { gParams.tx -= 0.5; enterLive(); }
        else if (key == GLFW_KEY_RIGHT) { gParams.tx += 0.5; enterLive(); }
        else if (key == GLFW_KEY_UP)    { gParams.ty += 0.5; enterLive(); }
        else if (key == GLFW_KEY_DOWN)  { gParams.ty -= 0.5; enterLive(); }
        else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
            gParams.reflectY = !gParams.reflectY;
            enterLive();
        } else if (action == GLFW_PRESS && key == GLFW_KEY_O) {
            gParams.reverseOrder = !gParams.reverseOrder;
            enterLive();
        }
    }

    if (changed) {
        updateWindowTitle(window);
        printCurrentReport();
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
    cg::RasterCanvas canvas(cg::GRID_WIDTH, cg::GRID_HEIGHT);
    for (const auto& exp : gPresets) {
        cg::renderExperimentToCanvas(canvas, gScene, exp, true, true);
        const std::string filePath = outDir + "/" + exp.id + ".ppm";
        if (!canvas.savePPM(filePath, 4)) {
            std::cerr << "Error guardando " << filePath << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Exportado raster real C++: " << filePath << "\n";
    }
    return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char** argv) {
    gPresets = cg::buildPresetExperiments();

    // Modo headless opcional para exportar capturas exactas del rasterizador C++
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
        WINDOW_WIDTH, WINDOW_HEIGHT,
        "UNSAAC CG1 - Tema 2 | Laboratorio de Transformaciones Afines 2D",
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
    glClearColor(cg::COL_BG.r, cg::COL_BG.g, cg::COL_BG.b, 1.0f);

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    framebufferSizeCallback(window, fbW, fbH);

    printHelp();
    updateWindowTitle(window);
    printCurrentReport();

    while (!glfwWindowShouldClose(window)) {
        renderFrame();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
