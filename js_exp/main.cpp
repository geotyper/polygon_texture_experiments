// Dear ImGui: standalone example application for Emscripten, using SDL2 + OpenGL3
// (Emscripten is a C++-to-javascript compiler, used to publish executables for the web. See https://emscripten.org/)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// This is mostly the same code as the SDL2 + OpenGL3 example, simply with the modifications needed to run on Emscripten.
// It is possible to combine both code into a single source file that will compile properly on Desktop and using Emscripten.
// See https://github.com/ocornut/imgui/pull/2492 as an example on how to do just that.

#include "src/graphmodule.hpp"


int main(int, char**)
{


    GraphModule grapmodule;
    grapmodule.RunLoop();

}
