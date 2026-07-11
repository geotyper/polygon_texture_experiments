#ifdef USE_JS
#include <emscripten.h>
#endif

#include "graphmodule.hpp"

#define shortMenu false

void GraphModule::draw_ImGui() {

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if(shortMenu)
    {
         ImGui::Begin("Exp JS");

            if(ImGui::Button("generate some picture"))
            {
                solver.initParticles(simDynParam.particlesNum, glm::vec2(width,height), simDynParam);
                simRunParam.runSimulation=true;

                simRunParam.runPolygonPipeline=true;
                simRunParam.drawTriangulatePolygons=true;
                simRunParam.drawTriangulatePolygonsOffset=true;
                simRunParam.drawPolyConstraints=false;
                simRunParam.drawPolygons=false;
                simRunParam.drawConstraints=false;
                simRunParam.drawPoints=false;
            }

         ImGui::End();
    }
     else
    {
        float f = 0.0f;
        int counter = 0;

        ImGui::Begin("Exp JS");                                // Create a window called "Hello, world!" and append into it.
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Application step %.3f ms/frame", timeSincePrevious);
        ImGui::Text("Particle number %i", solver.particleList.size());
        ImGui::Text("Constraint number %i", solver.constraintList.size());
        ImGui::Text("World Size width %i height %i", width, height);

        if(ImGui::Button("remove Simple Constraint"))
        {
            simRunParam.runRemoveSimpleConstraint=true;
        }

        if(ImGui::Button("Draw source"))
        {
            simRunParam.drawConstraints=true;
            simRunParam.drawPoints=true;
            simRunParam.drawPolyConstraints=false;
            simRunParam.drawNewPoints=false;

            simRunParam.drawPolygons=false;
            simRunParam.drawTriangulatePolygons=false;
        }

        if(ImGui::Button("SweepLine run"))
        {
            simRunParam.generateSweepLine=true;
            simRunParam.drawConstraints=false;
            simRunParam.drawPoints=false;
            simRunParam.drawPolyConstraints=true;
            simRunParam.drawNewPoints=true;
        }

        if(ImGui::Button("Polygons create"))
        {
            simRunParam.generatePolygons=true;
            simRunParam.drawConstraints=true;
            simRunParam.drawPoints=true;
            simRunParam.drawPolyConstraints=false;
            simRunParam.drawNewPoints=false;
        }

        ImGui::Checkbox("Draw points", &simRunParam.drawPoints);
        ImGui::Checkbox("Draw constraints", &simRunParam.drawConstraints);
        ImGui::Checkbox("Draw Polygons", &simRunParam.drawPolygons );
        ImGui::Checkbox("Draw New Point", &simRunParam.drawNewPoints );
        ImGui::Checkbox("Draw Polyconstraints", &simRunParam.drawPolyConstraints);
        ImGui::Checkbox("Draw TriangulatePolygons", &simRunParam.drawTriangulatePolygons);
        ImGui::Checkbox("Draw TriangulatePolygonsOffset", &simRunParam.drawTriangulatePolygonsOffset);

        ImGui::Checkbox("Draw TriangulatePolygonsUV", &simRunParam.drawTriangulatePolygonsGA);
        ImGui::Checkbox("Draw TriangulatePolygonsOffsetUV", &simRunParam.drawTriangulatePolygonsOffsetGA);

        if(ImGui::SliderInt("Particles num", &simDynParam.particlesNum ,1, max_particlesLimit))
        {
            solver.initParticles(simDynParam.particlesNum, glm::vec2(width,height), simDynParam);
            simRunParam.runSimulation=true;

            simRunParam.drawConstraints=true;
            simRunParam.drawPoints=true;
            simRunParam.drawPolyConstraints=false;
            simRunParam.drawNewPoints=false;
        }

        const char* placementModes[] = { "Radial", "Rectangular", "Noise-based", "Spiral", "Perspective Grid", "Nested Squares" };
        int currentMode = simDynParam.pointPlacementMode;
        if (ImGui::Combo("Placement Mode", &currentMode, placementModes, IM_ARRAYSIZE(placementModes)))
        {
            simDynParam.pointPlacementMode = currentMode;
            solver.initParticles(simDynParam.particlesNum, glm::vec2(width,height), simDynParam);
            simRunParam.runSimulation=true;

            simRunParam.drawConstraints=true;
            simRunParam.drawPoints=true;
            simRunParam.drawPolyConstraints=false;
            simRunParam.drawNewPoints=false;
        }

        if(ImGui::SliderFloat("Create dist constraint", &simDynParam.initConstraintDist ,0.0,170))
        {
            simRunParam.runSimulation=true;
        }
        if(ImGui::SliderFloat("Min dist constraint", &simDynParam.minConstraintDist ,0.0,170))
        {
            simRunParam.runSimulation=true;
        }
        if(ImGui::SliderInt("Max constraints / point", &simDynParam.maxConstraintsPerPoint, 1, 10))
        {
            solver.initParticles(simDynParam.particlesNum, glm::vec2(width,height), simDynParam);
            simRunParam.runSimulation=true;
        }
        if(ImGui::SliderFloat("Point move dist", &simDynParam.pointsMoveRange ,0.0,25))
        {
            solver.initParticles(simDynParam.particlesNum, glm::vec2(width,height), simDynParam);
            simRunParam.runSimulation=true;

            simRunParam.drawConstraints=true;
            simRunParam.drawPoints=true;
            simRunParam.drawPolyConstraints=false;
            simRunParam.drawNewPoints=false;
        }

        ImGui::SliderInt("Max Polygon Point", &simRunParam.maxPolygonPoints ,1,125);

        ImGui::SliderInt("minus Smallest Polygons", &simRunParam.minusSmallestPolygons ,0, 125);
        ImGui::SliderInt("minus Biggest Polygons", &simRunParam.minusBiggestPolygons   ,0, 125);


        if(ImGui::Button("Polygons Offset"))
        {
            simRunParam.generateOffset=true;
        }
        if(ImGui::SliderFloat("Poligon Offset", &simRunParam.polygonOffest ,-15.0,-0.01))
        {
            simRunParam.generateOffset=true;
        }

        if(ImGui::SliderFloat("Min Polygon Area", &solver.polyLib.arr.minPolygonArea, 1.0f, 200.0f))
        {
            simRunParam.generateOffset=true;
            simRunParam.triangulatePolygons=true;
        }

        ImGui::Separator();
        ImGui::Text("Polygon Spline Smoothing");
        if(ImGui::Checkbox("Enable Smoothing", &simDynParam.enableSmoothing))
        {
            simRunParam.generatePolygons=true;
            simRunParam.generateOffset=true;
            simRunParam.triangulatePolygons=true;
        }
        if (simDynParam.enableSmoothing)
        {
            if(ImGui::SliderFloat("Smoothing Tension", &simDynParam.smoothingTension, 0.0f, 1.0f, "%.2f"))
            {
                simRunParam.generatePolygons=true;
                simRunParam.generateOffset=true;
                simRunParam.triangulatePolygons=true;
            }
            if(ImGui::SliderFloat("Points / unit length", &simDynParam.smoothingPointsPerUnit, 0.02f, 1.0f, "%.2f"))
            {
                simRunParam.generatePolygons=true;
                simRunParam.generateOffset=true;
                simRunParam.triangulatePolygons=true;
            }
            if(ImGui::SliderFloat("Interpolation Zoom", &simDynParam.smoothingZoom, 0.7f, 1.3f, "%.2f"))
            {
                simRunParam.generatePolygons=true;
                simRunParam.generateOffset=true;
                simRunParam.triangulatePolygons=true;
            }
        }

        ImGui::Separator();
        ImGui::Text("Recursive Cell Subdivision");
        if(ImGui::SliderInt("Subdivision Depth", &simDynParam.subdivisionDepth, 0, 4))
        {
            simRunParam.generatePolygons=true;
            simRunParam.generateOffset=true;
            simRunParam.triangulatePolygons=true;
        }
        const char* subModes[] = { "Centroid", "Skeleton" };
        if (ImGui::Combo("Subdivision Mode", &simDynParam.subdivisionMode, subModes, IM_ARRAYSIZE(subModes)))
        {
            simRunParam.generatePolygons=true;
            simRunParam.generateOffset=true;
            simRunParam.triangulatePolygons=true;
        }
        if (simDynParam.subdivisionDepth > 0)
        {
            if(ImGui::SliderFloat("Subdivision Min Area", &simDynParam.subdivisionMinArea, 50.0f, 2000.0f, "%.1f"))
            {
                simRunParam.generatePolygons=true;
                simRunParam.generateOffset=true;
                simRunParam.triangulatePolygons=true;
            }
        }

        ImGui::Separator();
        ImGui::Text("Fractal Feedback Loop Subdivision");
        ImGui::Checkbox("Enable Fractal Loop", &simDynParam.enableFractalSubdivision);
        if (simDynParam.enableFractalSubdivision)
        {
            ImGui::SliderInt("Fractal Depth", &simDynParam.fractalSubdivisionDepth, 1, 3);
            ImGui::SliderFloat("Fractal Min Area", &simDynParam.fractalSubdivisionMinArea, 50.0f, 3000.0f, "%.1f");
            ImGui::SliderFloat("Fractal Scale Factor", &simDynParam.fractalSubdivisionScale, 0.3f, 0.95f, "%.2f");
            ImGui::SliderFloat("Fractal Shift (offset)", &simDynParam.fractalSubdivisionShift, 0.0f, 50.0f, "%.1f");
            ImGui::SliderFloat("Fractal Max Rotate (deg)", &simDynParam.fractalSubdivisionRotateRange, 0.0f, 90.0f, "%.1f");
        }
        ImGui::Separator();

        if(ImGui::Button("triangulate polygons"))
        {
            simRunParam.triangulatePolygons=true;
            simRunParam.drawTriangulatePolygons=true;
        }

        const char* palettes[] = { "Kintsugi (Ink & Gold)", "Blueprint (Cobalt)", "Malachite & Rust", "Crimson & Steel", "Sage & Sand", "Bauhaus", "Noir", "Ochre & Charcoal", "Cyberpunk Minimal", "Muted Rose & Slate", "Ash & Amber" };
        int currentPalette = (int)solver.palette;
        if (ImGui::Combo("Palette Selection", &currentPalette, palettes, IM_ARRAYSIZE(palettes)))
        {
            solver.palette = (unsigned int)currentPalette;
            simRunParam.recolorPolygons = true;
        }
        if (ImGui::Checkbox("Merge Same-Color Neighbors", &simDynParam.enablePolygonMerging))
        {
            simRunParam.triangulatePolygons = true;
        }
        if (ImGui::Checkbox("Remove Overlapping Polygons", &simDynParam.removeOverlappingPolygons))
        {
            simRunParam.triangulatePolygons = true;
        }
        ImGui::SliderFloat("General Zoom", &simDynParam.generalZoom, 0.2f, 3.0f, "%.2f");

        const char* bgPresets[] = { "Original Dark Blue", "Dark Gray", "Cinematic Black", "Charcoal", "Cream / Sepia", "Pure White", "Custom Color" };
        static int selectedBg = 0; // Default: Original Dark Blue
        if (ImGui::Combo("Background Style", &selectedBg, bgPresets, IM_ARRAYSIZE(bgPresets)))
        {
            if (selectedBg == 0) simDynParam.backgroundColor = glm::vec3(0.15f, 0.15f, 0.33f);
            else if (selectedBg == 1) simDynParam.backgroundColor = glm::vec3(0.11f, 0.11f, 0.11f);
            else if (selectedBg == 2) simDynParam.backgroundColor = glm::vec3(0.02f, 0.02f, 0.02f);
            else if (selectedBg == 3) simDynParam.backgroundColor = glm::vec3(0.15f, 0.15f, 0.15f);
            else if (selectedBg == 4) simDynParam.backgroundColor = glm::vec3(0.92f, 0.90f, 0.85f);
            else if (selectedBg == 5) simDynParam.backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
        }

        if (selectedBg == 6) // Custom Color
        {
            float col[3] = { simDynParam.backgroundColor.r, simDynParam.backgroundColor.g, simDynParam.backgroundColor.b };
            if (ImGui::ColorEdit3("Custom BG Color", col))
            {
                simDynParam.backgroundColor = glm::vec3(col[0], col[1], col[2]);
            }
        }

        const char* polyConstPresets[] = { "Original Light Green", "Sophisticated White", "Deep Gray", "Electric Orange", "Pastel Cyan", "Custom Color" };
        static int selectedPolyConst = 0; // Default: Original Light Green
        if (ImGui::Combo("Polyconstraint Line Style", &selectedPolyConst, polyConstPresets, IM_ARRAYSIZE(polyConstPresets)))
        {
            if (selectedPolyConst == 0) simDynParam.constraintColor = glm::vec3(0.75f, 0.90f, 0.75f);
            else if (selectedPolyConst == 1) simDynParam.constraintColor = glm::vec3(0.95f, 0.95f, 0.95f);
            else if (selectedPolyConst == 2) simDynParam.constraintColor = glm::vec3(0.35f, 0.35f, 0.35f);
            else if (selectedPolyConst == 3) simDynParam.constraintColor = glm::vec3(0.95f, 0.50f, 0.15f);
            else if (selectedPolyConst == 4) simDynParam.constraintColor = glm::vec3(0.60f, 0.85f, 0.90f);
        }

        if (selectedPolyConst == 5) // Custom Color
        {
            float col[3] = { simDynParam.constraintColor.r, simDynParam.constraintColor.g, simDynParam.constraintColor.b };
            if (ImGui::ColorEdit3("Custom Constraint Color", col))
            {
                simDynParam.constraintColor = glm::vec3(col[0], col[1], col[2]);
            }
        }

        if(ImGui::Button("PolygonPipeline"))
        {
            simRunParam.runPolygonPipeline=true;
            simRunParam.drawTriangulatePolygons=true;
            simRunParam.drawPolyConstraints=true;
            simRunParam.drawPolygons=true;
            simRunParam.drawConstraints=false;
            simRunParam.drawPoints=false;
        }

        bool flag1=false;
        if(ImGui::SliderInt("Constr param1", &simDynParam.createConstrParam01 ,0,9))
            flag1=true;
        bool flag2=false;
        flag2=ImGui::SliderInt("Constr param2", &simDynParam.createConstrParam02 ,0,9);
        bool flag3=false;
        flag3=ImGui::SliderInt("Constr param3", &simDynParam.createConstrParam03 ,0,9);
        bool flag4=false;
        flag4=ImGui::SliderInt("Constr param4", &simDynParam.createConstrParam04 ,0,9);
        bool flag5=false;
        flag5=ImGui::SliderInt("Constr param5", &simDynParam.createConstrParam05 ,0,9);
        bool flag6=false;
        flag6=ImGui::SliderInt("Constr param6", &simDynParam.createConstrParam06 ,0,9);
        bool flag7=false;
        flag7=ImGui::SliderInt("Constr param7", &simDynParam.createConstrParam07 ,0,9);


        if(flag1 or flag2 or flag3 or flag4 or flag5 or flag6 or flag7)
        {
            solver.initParticles(simDynParam.particlesNum, glm::vec2(width,height), simDynParam);
            simRunParam.runSimulation=true;

            simRunParam.drawConstraints=true;
            simRunParam.drawPoints=true;

            simRunParam.drawPolyConstraints=false;
            simRunParam.drawNewPoints=false;
            simRunParam.drawPolygons=false;
            simRunParam.drawTriangulatePolygons=false;
        }

        ImGui::End();
    }

    ImGui::Begin("Noise Displacement Preview");
    {
        static GLuint noisePreviewTexture = 0;
        static unsigned char noiseData[128 * 128];
        static bool firstTime = true;

        if (noisePreviewTexture == 0) {
            glGenTextures(1, &noisePreviewTexture);
            glBindTexture(GL_TEXTURE_2D, noisePreviewTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        bool changed = false;

        changed |= ImGui::Checkbox("Use Noise Displacement", &simDynParam.useNoiseDisplacement);
        changed |= ImGui::SliderFloat("Noise Scale", &simDynParam.noiseScale, 0.001f, 0.1f, "%.4f");
        changed |= ImGui::SliderInt("Noise Octaves", &simDynParam.octaves, 1, 8);
        changed |= ImGui::SliderFloat("Noise Persistence", &simDynParam.persistence, 0.05f, 1.0f);
        changed |= ImGui::SliderFloat("Noise Lacunarity", &simDynParam.lacunarity, 1.0f, 4.0f);

        if (changed || firstTime) {
            firstTime = false;
            // Regenerate preview
            for (int y = 0; y < 128; ++y) {
                for (int x = 0; x < 128; ++x) {
                    float val = Solver::fBm2D(x * simDynParam.noiseScale * 10.f, y * simDynParam.noiseScale * 10.f, simDynParam.octaves, simDynParam.lacunarity, simDynParam.persistence);
                    unsigned char c = (unsigned char)((val * 0.5f + 0.5f) * 255.0f);
                    noiseData[y * 128 + x] = c;
                }
            }
            glBindTexture(GL_TEXTURE_2D, noisePreviewTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 128, 128, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, noiseData);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        ImGui::Image((void*)(intptr_t)noisePreviewTexture, ImVec2(128, 128));
        ImGui::SameLine();
        ImGui::Text("Live\nNoise\nPreview");

        if (ImGui::Button("Re-initialize layout") || (changed && simDynParam.useNoiseDisplacement)) {
            solver.initParticles(simDynParam.particlesNum, glm::vec2(width,height), simDynParam);
            simRunParam.runSimulation=true;
            simRunParam.drawConstraints=true;
            simRunParam.drawPoints=true;
            simRunParam.drawPolyConstraints=false;
            simRunParam.drawNewPoints=false;
        }
    }
    ImGui::End();


    ImGui::Begin("Texture 2 viewer");
        ImGui::Text("pointer = %p", solver.function_image.id);
        ImGui::Text("size = %d x %d", solver.function_image.width,  solver.function_image.height);
        ImGui::Image((void*)solver.function_image.id, ImVec2(solver.function_image.width, solver.function_image.height));
    ImGui::End();


    ImGui::Begin("Texture preview");

    ImGui::SliderFloat("Texture UV Zoom", &simDynParam.textureUVZoom, 0.001f, 5.0f, "%.4f");
    ImGui::SliderFloat("Texture UV Rotation", &simDynParam.textureUVAngle, 0.0f, 360.0f, "%.1f");
    ImGui::Checkbox("Auto-Regen on Triangulate", &simDynParam.autoRegenerateTextures);

    int minDepth = (int)solver.settings.function_depth.min;
    int maxDepth = (int)solver.settings.function_depth.max;
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::SliderInt("Min Depth", &minDepth, 1, 5)) {
        solver.settings.function_depth.min = minDepth;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::SliderInt("Max Depth", &maxDepth, 1, 5)) {
        solver.settings.function_depth.max = maxDepth;
    }

    if (ImGui::Button("Regenerate All Textures"))
    {
        solver.fillAllTextures();
    }
    ImGui::Separator();

    int texSize=solver.function_image.height/6;
    int textureIndex=0;
    for(int xs=0; xs<6; xs++)
    {
        for(int ys=0; ys<5; ys++)
        {
             ImGui::Image((void*)solver.textureIndexList[textureIndex], ImVec2(texSize, texSize));
             ImGui::SameLine();
             textureIndex++;
        }
        ImGui::NewLine();
    }

    ImGui::End();


/*
    ImGui::Begin("Particles viewer");
       ImGui::Text("pointer = %p",   graphicProcessor.renderscreenQuad.textureColorbuffer);
       ImGui::Text("size = %d x %d", width/2, height/2);
       ImGui::Image((void*)(intptr_t)graphicProcessor.renderscreenQuad.textureColorbuffer,
                    ImVec2(width/2, height/2),ImVec2(0, 0),ImVec2(1, -1));
    ImGui::End();
*/
    // Rendering
    ImGui::Render();
   // glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifdef USE_JS
    {
        size_t size = 0;
        const char* data = ImGui::SaveIniSettingsToMemory(&size);
        if (data && size > 0) {
            EM_ASM({
                var dataStr = UTF8ToString($0);
                try {
                    localStorage.setItem("imgui_ini", dataStr);
                } catch (e) {
                    console.error("Failed to save imgui settings to localStorage:", e);
                }
            }, data);
        }
    }
#endif


}


void GraphModule::draw_Points(std::vector<InstanceParticle> &pointList) {

    if(pointList.size()>0)
    {
        glUseProgram(graphicProcessor.circleInstance.prog);
        {
            GLint zoomLoc = glGetUniformLocation(graphicProcessor.circleInstance.prog, "uZoom");
            if (zoomLoc != -1) glUniform1f(zoomLoc, simDynParam.generalZoom);

            glBindBuffer(GL_UNIFORM_BUFFER, graphicProcessor.circleInCircle.uniform.ubo );

            glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.circleInstance.vbo_inst);
            size_t bytes = pointList.size()*sizeof(InstanceParticle);
            glBufferData(GL_ARRAY_BUFFER, bytes, pointList.data(), GL_DYNAMIC_DRAW);

            glBindVertexArray(graphicProcessor.circleInstance.vao);
            glDrawElementsInstanced(GL_TRIANGLES, graphicProcessor.circleInstance.nindices, GL_UNSIGNED_INT, (const void *) NULL, pointList.size());
            glBindVertexArray(0);
        }
        glUseProgram(0u);

   CHECKGLERROR();
  }

}

void GraphModule::draw_Constraints() {
    //glViewport(0, 0, mainScreenWidth,mainScreenHeight);
    glUseProgram(graphicProcessor.constraints.prog);
    {
          GLint zoomLoc = glGetUniformLocation(graphicProcessor.constraints.prog, "uZoom");
          if (zoomLoc != -1) glUniform1f(zoomLoc, simDynParam.generalZoom);

          GLint colorLoc = glGetUniformLocation(graphicProcessor.constraints.prog, "uConstraintColor");
          if (colorLoc != -1) {
              glUniform3f(colorLoc, simDynParam.constraintColor.r, simDynParam.constraintColor.g, simDynParam.constraintColor.b);
          }

          glBindBuffer ( GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo);

          glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.constraints.vbo);
          glBufferData(GL_ARRAY_BUFFER, solver.constraintListDraw.size()*sizeof(glm::vec2), solver.constraintListDraw.data(), GL_DYNAMIC_DRAW);

          glBindVertexArray(graphicProcessor.constraints.vao);
          glLineWidth(1.0);
          glDrawArrays(GL_LINES, 0,  solver.constraintListDraw.size());
          glBindVertexArray(0);
    }
    glUseProgram(0u);
    CHECKGLERROR();
}


void GraphModule::draw_Constraints(std::vector<glm::vec2>&  constraintListDraw) {

    //glViewport(0, 0, mainScreenWidth,mainScreenHeight);
    glUseProgram(graphicProcessor.constraints.prog);
    {
          GLint zoomLoc = glGetUniformLocation(graphicProcessor.constraints.prog, "uZoom");
          if (zoomLoc != -1) glUniform1f(zoomLoc, simDynParam.generalZoom);

          GLint colorLoc = glGetUniformLocation(graphicProcessor.constraints.prog, "uConstraintColor");
          if (colorLoc != -1) {
              glUniform3f(colorLoc, simDynParam.constraintColor.r, simDynParam.constraintColor.g, simDynParam.constraintColor.b);
          }

          //glBindVertexArray(graphicProcessor.box.vao);
          glBindBuffer (GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo);

          glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.constraints.vbo);
          //glBufferData(GL_ARRAY_BUFFER, 0, solver.constraintListDraw.size()*sizeof(glm::vec2), solver.constraintListDraw.data());
          glBufferData(GL_ARRAY_BUFFER, constraintListDraw.size()*sizeof(glm::vec2), constraintListDraw.data(), GL_DYNAMIC_DRAW);

          glBindVertexArray(graphicProcessor.constraints.vao);
          glLineWidth(2.7);
          glDrawArrays(GL_LINES, 0,  constraintListDraw.size());
          glBindVertexArray(0);
    }
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0u);

    CHECKGLERROR();

}

void GraphModule::draw_Polygons(const vector<vector<TrianglesDrawStruct>>& triangulatePolygons) {

    glUseProgram(graphicProcessor.polygon.prog);
    {
          GLint zoomLoc = glGetUniformLocation(graphicProcessor.polygon.prog, "uZoom");
          if (zoomLoc != -1) glUniform1f(zoomLoc, simDynParam.generalZoom);

          glBindBuffer ( GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo);

          glBindVertexArray(graphicProcessor.polygon.vao);
          for(auto polygon:triangulatePolygons)
          {
              glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.polygon.vbo);
              glBufferData(GL_ARRAY_BUFFER, polygon.size()*sizeof(TrianglesDrawStruct), polygon.data(), GL_DYNAMIC_DRAW);

              glDrawArrays(GL_TRIANGLES, 0,  polygon.size());
          }
          glBindVertexArray(0);

    }
    glUseProgram(0u);
    CHECKGLERROR();
}


void GraphModule::draw_Polygons(vector<vector<TrianglesDrawStruct>>& triangulatePolygons_in, vector<vector<unsigned int>>& triangulatePolygonsIndex_in) {
    glUseProgram(graphicProcessor.polygon.prog);
    {
          GLint zoomLoc = glGetUniformLocation(graphicProcessor.polygon.prog, "uZoom");
          if (zoomLoc != -1) glUniform1f(zoomLoc, simDynParam.generalZoom);

          glBindBuffer ( GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo);
          glBindVertexArray(graphicProcessor.polygon.vao);
          for(int i=0+simRunParam.minusSmallestPolygons; i<triangulatePolygons_in.size()-simRunParam.minusBiggestPolygons;i++)
          {
              glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.polygon.vbo);
              glBufferData(GL_ARRAY_BUFFER, triangulatePolygons_in[i].size()*sizeof(TrianglesDrawStruct), triangulatePolygons_in[i].data(), GL_DYNAMIC_DRAW);

              //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphicProcessor.polygon.ibo);
              //glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangulatePolygonsIndex[i].size()*sizeof(unsigned int), triangulatePolygonsIndex[i].data(), GL_DYNAMIC_DRAW);
               glDrawArrays(GL_TRIANGLES, 0,  triangulatePolygons_in[i].size());
              //glDrawElements(GL_TRIANGLES, triangulatePolygonsIndex_in[i].size(), GL_UNSIGNED_INT,0);
          }
          glBindVertexArray(0);
    }
    glUseProgram(0u);
    CHECKGLERROR();
}

void GraphModule::draw_PolygonsSort(vector<vector<TrianglesDrawStruct>>& triangulatePolygons_in, vector<vector<unsigned int>>& triangulatePolygonsIndex_in) {
    glUseProgram(graphicProcessor.polygon.prog);
    {
          GLint zoomLoc = glGetUniformLocation(graphicProcessor.polygon.prog, "uZoom");
          if (zoomLoc != -1) glUniform1f(zoomLoc, simDynParam.generalZoom);

          glBindBuffer ( GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo);
          glBindVertexArray(graphicProcessor.polygon.vao);
          bool isOffset = (&triangulatePolygons_in == &solver.polyLib.trianglesOffset_draw_vertex);
          for(int i=0+simRunParam.minusSmallestPolygons; i<triangulatePolygons_in.size()-simRunParam.minusBiggestPolygons;i++)
          {
              int ip = isOffset ? solver.polyLib.arr.areaListOffset[i].first : solver.polyLib.arr.areaList[i].first;
              glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.polygon.vbo);
              glBufferData(GL_ARRAY_BUFFER, triangulatePolygons_in[ip].size()*sizeof(TrianglesDrawStruct), triangulatePolygons_in[ip].data(), GL_DYNAMIC_DRAW);

              //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphicProcessor.polygon.ibo);
              //glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangulatePolygonsIndex[i].size()*sizeof(unsigned int), triangulatePolygonsIndex[i].data(), GL_DYNAMIC_DRAW);
               glDrawArrays(GL_TRIANGLES, 0,  triangulatePolygons_in[ip].size());
              //glDrawElements(GL_TRIANGLES, triangulatePolygonsIndex_in[i].size(), GL_UNSIGNED_INT,0);
          }
          glBindVertexArray(0);
    }
    glUseProgram(0u);
    CHECKGLERROR();
}


void GraphModule::draw_PolygonsUV(vector<vector<VertexPosUV>>& triangulatePolygons_in, vector<vector<unsigned int>>& triangulatePolygonsIndex_in) {

        //glViewport(0, 0, mainScreenWidth,mainScreenHeight);
        glUseProgram(graphicProcessor.polygonGA.prog);
        {
              GLint zoomLoc = glGetUniformLocation(graphicProcessor.polygonGA.prog, "uZoom");
              if (zoomLoc != -1) glUniform1f(zoomLoc, simDynParam.generalZoom);
              GLint uvZoomLoc = glGetUniformLocation(graphicProcessor.polygonGA.prog, "uUVZoom");
              if (uvZoomLoc != -1) glUniform1f(uvZoomLoc, simDynParam.textureUVZoom);
              GLint uvAngleLoc = glGetUniformLocation(graphicProcessor.polygonGA.prog, "uUVAngle");
              if (uvAngleLoc != -1) {
                  float angleRad = simDynParam.textureUVAngle * (3.14159265f / 180.0f);
                  glUniform1f(uvAngleLoc, angleRad);
              }
              bool isOffset = (&triangulatePolygons_in == &solver.polyLib.arr.triangles_draw_vertexOffsetUV);
              for(int i=0+simRunParam.minusSmallestPolygons; i<triangulatePolygons_in.size()-simRunParam.minusBiggestPolygons;i++)
              {

                  if(triangulatePolygons_in[i].size()==0)
                      continue;

                  int ip = isOffset ? solver.polyLib.arr.areaListOffset[i].first : solver.polyLib.arr.areaList[i].first;

                  glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.polygonGA.vbo);
                  glBufferData(GL_ARRAY_BUFFER, triangulatePolygons_in[ip].size()*sizeof(VertexPosUV), triangulatePolygons_in[ip].data(), GL_DYNAMIC_DRAW);
                  glBindBuffer(GL_ARRAY_BUFFER, 0);

                  glBindVertexArray(graphicProcessor.polygonGA.vao);
                  glActiveTexture(GL_TEXTURE0);
                  int textureIndex = 0;
                  if (isOffset) {
                      if (ip >= 0 && ip < (int)solver.polyLib.arr.trianglesOffset_parent_polygon_id.size()) {
                          textureIndex = solver.polyLib.arr.trianglesOffset_parent_polygon_id[ip] % solver.textureIndexList.size();
                      }
                  } else {
                      if (ip >= 0 && ip < (int)solver.polyLib.arr.triangles_parent_polygon_id.size()) {
                          textureIndex = solver.polyLib.arr.triangles_parent_polygon_id[ip] % solver.textureIndexList.size();
                      }
                  }
                  glBindTexture(GL_TEXTURE_2D, solver.textureIndexList[textureIndex]);
                  // Set the sampler texture unit to 0
                  glUniform1i(graphicProcessor.polygonGA.uniform.ubo, 0);

                  // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                  glDrawArrays(GL_TRIANGLES, 0, triangulatePolygons_in[ip].size());
                  glBindVertexArray(0);
                  //glDrawElements(GL_TRIANGLES, triangulatePolygonsIndex_in[i].size(), GL_UNSIGNED_INT,0);
              }

                CHECKGLERROR();
        }

        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        glUseProgram(0u);

       CHECKGLERROR();

}


void GraphModule::draw_Polygons() {
    if(polygonsList.size()>0)
    {
        glBindBuffer ( GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo);

        glUseProgram(graphicProcessor.constraints.prog);
        {
            GLint zoomLoc = glGetUniformLocation(graphicProcessor.constraints.prog, "uZoom");
            if (zoomLoc != -1) glUniform1f(zoomLoc, simDynParam.generalZoom);
            glBindVertexArray(graphicProcessor.constraints.vao);
            for(int i=0; i<polygonsListOffset.size();++i)
            {
                if(polygonsListOffset[i].size()>0 and polygonsListOffset[i].size()<simRunParam.maxPolygonPoints)
                {
                      auto polygon=polygonsListOffset[i];
                      glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.constraints.vbo);
                      glBufferData(GL_ARRAY_BUFFER, polygon.size()*sizeof(glm::vec2), polygon.data(), GL_DYNAMIC_DRAW);
                      glLineWidth(1.0);
                      glDrawArrays(GL_LINE_STRIP, 0,  polygon.size());
                }
            }
            glBindVertexArray(0);
        }
        glUseProgram(0u);
        CHECKGLERROR();
    }
}

