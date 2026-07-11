#ifdef USE_JS
#include <emscripten.h>
#endif

#include "graphmodule.hpp"


#ifdef USE_JS
EM_JS(float, fxrand_f, (), {
  return fxrand();
});
#endif

GraphModule::GraphModule()
{
   // auto res=emscripten_run_script_string("fxrand()");
   // std::cout<<"Fx function re: "<<res<<std::endl;
   // std::string fs(res);

    fxrand=1000;//(unsigned int)(solver.randomFloat(0,100000));
#ifdef USE_JS
    fxrand=static_cast<unsigned int>(1000000000*fxrand_f());
    cout.precision(17);
    std::cout<<"Rand function init: "<<fixed<<fxrand<<std::endl;
    srand(fxrand);
#endif
   // float fxrand2=fxrand_f();
   //std::cout<<"Fx function re: "<<fixed<<fxrand_f()<<std::endl;


    solver.settings.seed=fxrand;

    screen = NULL;

    simDynParam.initConstraintDist=32.0;
    simDynParam.pointsMoveRange=3.75f;
    simDynParam.particlesNum=2350;

    width=worldX;
    height=worldY;

    solver.mapSize=glm::ivec2(mapDivider,mapDivider);
    solver.mapCellSize=glm::ivec2((float)(width)/mapDivider, (float)(height)/mapDivider);

    solver.gridObject.set(solver.mapSize.x,solver.mapSize.y);

    auto resultSDLInit=InitSDL();
#ifdef FXPUBLISH
    if(resultSDLInit)
        std::cout<<"init Graphic System Successfull"<<std::endl;
    else
        std::cout<<"Error with Graphic System init"<<std::endl;
#endif


#ifdef FXPUBLISH
     auto resultImguiInit=InitImGui();
    if(resultImguiInit)
        std::cout<<"init ImGui Successfull"<<std::endl;
    else
        std::cout<<"Error with ImGui init"<<std::endl;
#endif
    auto resultInitShaders=InitShaders();
#ifdef FXPUBLISH
    if(resultInitShaders)
        std::cout<<"load and init Shaders Successfull"<<std::endl;
    else
        std::cout<<"Error with Shaders init"<<std::endl;
#endif
    auto resultFrameBufferInit=InitframeBuffers();
#ifdef FXPUBLISH
    if(resultFrameBufferInit)
        std::cout<<"init FrameBuffers Successfull"<<std::endl;
    else
        std::cout<<"Error with frameBuffers init"<<std::endl;
#endif
    //FramebufferBox_create();

    //--------------------Start Texture init-------------
    solver.texturesInit();
    int numTex=64;
    bool rungenerator=true;
    int counterTex=0;
    while(rungenerator) {
       bool res=solver.generate_image();
       if(res)
           counterTex++;
       if(numTex<counterTex)
           rungenerator=false;
    }
    //--------------------Texture init-------------


    FramebufferBoxElement();

    CircleCreateSegment(graphicProcessor.circle, 1.0, 32);
    solver.initParticles(particlesInitSize, glm::vec2(width,height), simDynParam);
    CircleCreateInstance(graphicProcessor.circleInstance, 1.0, 32, solver.particleList);

    solver.mapPointsUpdate();
    solver.Step(simDynParam);

    Constraint_create();

    Box_create();
    polygonCreate();
    UniformBlocksCreate();

    createPolygonGA();

    simRunParam.runSimulation=true;

    UniformBlocksUpdate();

}

bool GraphModule::InitSDL()
{
   #ifdef USE_JS

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    //const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL ); //| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    screen = SDL_CreateWindow("JS CPP experiments", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mainScreenWidth, mainScreenHeight, window_flags);
    gContext = SDL_GL_CreateContext(screen);
    if (!gContext)
    {
        fprintf(stderr, "Failed to initialize WebGL context!\n");
        return 1;
    }
    SDL_GL_SetSwapInterval(1); // Enable vsync


#else

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }


    // Initialize rendering context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    screen = SDL_CreateWindow("JS exp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mainScreenWidth, mainScreenHeight, window_flags);
    gContext = SDL_GL_CreateContext(screen);

    SDL_GL_MakeCurrent(screen, gContext);
    SDL_GL_SetSwapInterval(1);

    GLenum err;
    glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
    err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to init GLEW" << std::endl;
        SDL_GL_DeleteContext(gContext);
        SDL_DestroyWindow(screen);
        SDL_Quit();
        return true;
    }


    //Main loop flag
    bool quit = false;
    SDL_Event e;
    SDL_StartTextInput();

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    std::cout << "OpenGL version       | " << major << "." << minor << std::endl;
    std::cout << "GLEW version         | " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "---------------------+-------" << std::endl;

#endif
    return true;
}

bool GraphModule::InitImGui()
{
/*
    // Setup Dear ImGui context
       IMGUI_CHECKVERSION();
       ImGui::CreateContext();
       //ImGui::LoadIniSettingsFromDisk("tempImgui.ini");
       ImGuiIO& io = ImGui::GetIO(); (void)io;
       static ImGuiStyle* style = &ImGui::GetStyle();
       style->Alpha = 1.00f; //0.75f

       io.WantCaptureMouse=true;
       //io.WantCaptureKeyboard=false;
       //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
       //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

       // Setup Dear ImGui style
       ImGui::StyleColorsDark();
       //ImGui::StyleColorsClassic();

       // Setup Platform/Renderer backends
       ImGui_ImplSDL2_InitForOpenGL(screen, gContext);
       ImGui_ImplOpenGL3_Init(glsl_version);

       return true;
       */
#ifdef FXPUBLISH
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = NULL;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.3f;
    style.FrameRounding = 2.3f;
    style.ScrollbarRounding = 0;

    style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
    //style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.1f, 0.1f, 0.1f, 0.99f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
   // style.Colors[ImGuiCol_Column]                = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
   // style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
  //  style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
  //  style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.50f, 0.50f, 0.90f, 0.50f);
  //  style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.70f, 0.70f, 0.90f, 0.60f);
  //  style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
  //  style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);




    style.WindowPadding                     = ImVec2(8.00f, 8.00f);
    style.FramePadding                      = ImVec2(5.00f, 2.00f);
    style.CellPadding                       = ImVec2(6.00f, 6.00f);
    style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
    style.IndentSpacing                     = 25;
    style.ScrollbarSize                     = 15;
    style.GrabMinSize                       = 10;
    style.WindowBorderSize                  = 1;
    style.ChildBorderSize                   = 1;
    style.PopupBorderSize                   = 1;
    style.FrameBorderSize                   = 1;
    style.TabBorderSize                     = 1;
    style.WindowRounding                    = 7;
    style.ChildRounding                     = 4;
    style.FrameRounding                     = 3;
    style.PopupRounding                     = 4;
    style.ScrollbarRounding                 = 9;
    style.GrabRounding                      = 3;
    style.LogSliderDeadzone                 = 4;
    style.TabRounding                       = 4;
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(screen, gContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Emscripten allows preloading a file or folder to be accessible at runtime. See Makefile for details.
    //io.Fonts->AddFontDefault();
#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
#endif


#endif

     return true;
}


bool GraphModule::InitShaders()
{
     char *src_buffer = new char[MAX_SHADER_BUFFERSIZE]();
    /* Setup programs */

        graphicProcessor.box.prog= shaderutils.CreateRenderProgram((path_string+"assets/"+ "vs_basic.glsl").data(),
                                         (path_string+"assets/"+ "fs_basic.glsl").data(),
                                         src_buffer);

        graphicProcessor.circleInCircle.prog= shaderutils.CreateRenderProgram((path_string+"assets/"+ "vs_basic_color.glsl").data(),
                                         (path_string+"assets/"+ "fs_basic_color.glsl").data(),
                                         src_buffer);

        graphicProcessor.circleInstance.prog= shaderutils.CreateRenderProgram((path_string+"assets/"+ "vs_instance.glsl").data(),
                                         (path_string+"assets/"+ "fs_instance.glsl").data(),
                                         src_buffer);

        graphicProcessor.constraints.prog= shaderutils.CreateRenderProgram((path_string+"assets/"+ "vs_constraints.glsl").data(),
                                          (path_string+"assets/"+ "fs_constraints.glsl").data(),
                                          src_buffer);

        graphicProcessor.polygon.prog= shaderutils.CreateRenderProgram((path_string+"assets/"+ "vs_polygon.glsl").data(),
                                          (path_string+"assets/"+ "fs_polygon.glsl").data(),
                                          src_buffer);

        graphicProcessor.framebufferBox.prog= shaderutils.CreateRenderProgram((path_string+"assets/"+ "vs_framebuffer_tex.glsl").data(),
                                          (path_string+"assets/"+ "fs_framebuffer_tex.glsl").data(),
                                          src_buffer);

        graphicProcessor.framebufferBox.uniform.ubo = shaderutils.GetUniformLocation(graphicProcessor.framebufferBox.prog, "s_texture");

        graphicProcessor.polygonGA.prog= shaderutils.CreateRenderProgram((path_string+"assets/"+ "vs_poly_tex.glsl").data(),
                                          (path_string+"assets/"+ "fs_poly_tex.glsl").data(),src_buffer);
        graphicProcessor.polygonGA.uniform.ubo = shaderutils.GetUniformLocation(graphicProcessor.polygonGA.prog, "s_texture");

     delete [] src_buffer;

   return true;

}


void GraphModule::UniformBlocksUpdate()
{
    {
        uBlock *uDataTemp =new uBlock(
        {
             glm::vec2(width,height),
             glm::vec2(mainScreenWidth/4,mainScreenHeight/2),
             (float)(mainScreenWidth/10.0)
        });
        glBindBuffer ( GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo );
        glBufferSubData ( GL_UNIFORM_BUFFER, 0, graphicProcessor.box.uniform.blockSize, uDataTemp);
        delete uDataTemp;
    }

    {
        uBlock *uDataTemp =new uBlock(
        {
             glm::vec2(width,height),
             glm::vec2(width,0),
             (float)(width/2.0)
        });
        glBindBuffer(GL_UNIFORM_BUFFER, graphicProcessor.circleInCircle.uniform.ubo );
        glBufferSubData(GL_UNIFORM_BUFFER, 0, graphicProcessor.circleInCircle.uniform.blockSize, uDataTemp);
        delete uDataTemp;
    }

    {
        uBlock *uDataTemp =new uBlock(
        {
             glm::vec2(width,height),
             glm::vec2(mainScreenWidth,0),
             (float)(mainScreenWidth/2.0)
        });
        glBindBuffer(GL_UNIFORM_BUFFER, graphicProcessor.circleInCircle.uniform.ubo );
        glBufferSubData(GL_UNIFORM_BUFFER, 0, graphicProcessor.circleInCircle.uniform.blockSize, uDataTemp);
        delete uDataTemp;
    }

    {
        uBlock *uDataTemp =new uBlock(
        {
             glm::vec2(width,height),
             glm::vec2(0,0),
             1.0
        });
        glBindBuffer( GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo );
        glBufferSubData(GL_UNIFORM_BUFFER, 0, graphicProcessor.box.uniform.blockSize, uDataTemp);
        delete uDataTemp;
    }




}

void GraphModule::UniformBlocksCreate()
{
    {  //circleInCircle prog
        uBlock *uDataTemp =new uBlock(
        {
            glm::vec2(mainScreenWidth,mainScreenHeight),
            glm::vec2(0,0),
            230.0
        });

        // Retrieve the uniform block index
        graphicProcessor.circleInCircle.uniform.bindingPoint=0;
        std::string name="uBlock";
        graphicProcessor.circleInCircle.uniform.blockId = glGetUniformBlockIndex ( graphicProcessor.circleInCircle.prog, name.data());
        // Associate the uniform block index with a binding point
        glUniformBlockBinding (graphicProcessor.circleInCircle.prog, graphicProcessor.circleInCircle.uniform.blockId, graphicProcessor.circleInCircle.uniform.bindingPoint);
        glGetActiveUniformBlockiv (graphicProcessor.circleInCircle.prog, graphicProcessor.circleInCircle.uniform.blockId, GL_UNIFORM_BLOCK_DATA_SIZE, &graphicProcessor.circleInCircle.uniform.blockSize );
        //graphicProcessor.circleInCircle.uniform.blockSize=sizeof(uBlock);
        // Create and fill a buffer object
        glGenBuffers(1, &graphicProcessor.circleInCircle.uniform.ubo );
        glBindBuffer(GL_UNIFORM_BUFFER, graphicProcessor.circleInCircle.uniform.ubo );

        glBufferData(GL_UNIFORM_BUFFER, graphicProcessor.circleInCircle.uniform.blockSize, uDataTemp, GL_DYNAMIC_DRAW);
        // Bind the buffer object to the uniform block binding point
        glBindBufferBase(GL_UNIFORM_BUFFER, graphicProcessor.circleInCircle.uniform.bindingPoint, graphicProcessor.circleInCircle.uniform.ubo );

        delete uDataTemp;
    }

    {  //box prog
        uBlock *uDataTemp =new uBlock(
        {
            glm::vec2(mainScreenWidth,mainScreenHeight),
            glm::vec2(0,0),
            230.0
        });

        // Retrieve the uniform block index
        graphicProcessor.box.uniform.bindingPoint=1;
        std::string name="uBlock";
        graphicProcessor.box.uniform.blockId = glGetUniformBlockIndex ( graphicProcessor.box.prog, name.data());
        // Associate the uniform block index with a binding point
        glUniformBlockBinding (graphicProcessor.box.prog, graphicProcessor.box.uniform.blockId, graphicProcessor.box.uniform.bindingPoint);
        glGetActiveUniformBlockiv (graphicProcessor.box.prog, graphicProcessor.box.uniform.blockId, GL_UNIFORM_BLOCK_DATA_SIZE, &graphicProcessor.box.uniform.blockSize );
        //graphicProcessor.box.uniform.blockSize=sizeof(uBlock);
        // Create and fill a buffer object
        glGenBuffers(1, &graphicProcessor.box.uniform.ubo );
        glBindBuffer(GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.ubo );

        glBufferData(GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.blockSize, uDataTemp, GL_DYNAMIC_DRAW);
        // Bind the buffer object to the uniform block binding point
        glBindBufferBase(GL_UNIFORM_BUFFER, graphicProcessor.box.uniform.bindingPoint, graphicProcessor.box.uniform.ubo );

        delete uDataTemp;
    }

}


bool GraphModule::InitframeBuffers()
{
    // framebuffer configuration Stipples
    glGenFramebuffers(1, &graphicProcessor.renderscreenQuad.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, graphicProcessor.renderscreenQuad.framebuffer);
    // create a color attachment texture

    glGenTextures(1, &graphicProcessor.renderscreenQuad.textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, graphicProcessor.renderscreenQuad.textureColorbuffer);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    //glTexImage2D(GL_TEXTURE_2D, 0,   GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0,   GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, graphicProcessor.renderscreenQuad.textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &graphicProcessor.renderscreenQuad.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, graphicProcessor.renderscreenQuad.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,  GL_DEPTH_COMPONENT16, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, graphicProcessor.renderscreenQuad.rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    CHECKGLERROR();

    return true;
}

void GraphModule::main_loop(void* arg)
{
    #ifdef USE_JS
    GraphModule* graph_module=(GraphModule *)arg;

    graph_module->solver.tick++;

    SDL_GL_MakeCurrent(screen, gContext);

    glViewport ( 0, 0, graph_module->mainScreenWidth, graph_module->mainScreenHeight );
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClear(GL_COLOR_BUFFER_BIT);

    double curTime = emscripten_get_now();
    graph_module->timeSincePrevious = (graph_module->prevTime >= 0) ? (curTime - graph_module->prevTime) : (1000.0/60.0);


    //-------------------------------------------
    graph_module->FrameBufferDrawStep();
    //-------------------------------------------

    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

    //ImGuiIO& io = ImGui::GetIO();
    SDL_Event event;
    while (SDL_PollEvent(&event)){
       #ifdef FXPUBLISH
        ImGui_ImplSDL2_ProcessEvent(&event);
        // Capture events here, based on io.WantCaptureMouse and io.WantCaptureKeyboard
        #endif
    }

    graph_module->ProgressStep();
 #ifdef FXPUBLISH
    graph_module->draw_ImGui();
 #endif
    SDL_GL_SwapWindow(screen);

    #endif
}

bool GraphModule::RunLoop()
{
    #ifdef USE_JS
     emscripten_set_main_loop_arg(main_loop, this, 0, 1);
     int ret = emscripten_set_main_loop_timing(EM_TIMING_RAF, 4);
    #else
    bool done = false;
       while (!done){
           solver.tick++;

           SDL_GL_MakeCurrent(screen, gContext);

           glViewport ( 0, 0, mainScreenWidth, mainScreenHeight );
           glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
           glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
           //glClear(GL_COLOR_BUFFER_BIT);

           double curTime =0;// emscripten_get_now();
           timeSincePrevious = (prevTime >= 0) ? (curTime - prevTime) : (1000.0/60.0);

           //-------------------------------------------
           FrameBufferDrawStep();

           //-------------------------------------------

           // Poll and handle events (inputs, window resize, etc.)
           // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
           // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
           // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
           // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

           //ImGuiIO& io = ImGui::GetIO();

           while (SDL_PollEvent(&event)){
              #ifdef FXPUBLISH
              ImGui_ImplSDL2_ProcessEvent(&event);
              ImGuiIO& io = ImGui::GetIO();
              #endif
              if (event.type == SDL_QUIT)
              {
                  done = true;
                  //ImGui::SaveIniSettingsToDisk("tempImgui.ini");
              }
              if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(screen))
                  done = true;
              else
              {
                  //HandleEvents(event, camera_event);
              }
           }

            ProgressStep();
#ifdef FXPUBLISH
           draw_ImGui();
#endif
           SDL_GL_SwapWindow(screen);


       }

#endif


     return true;
}

bool GraphModule::ProgressStep()
{

    if(initRun)
    {
        initRun=false;

        solver.initParticles(simDynParam.particlesNum, glm::vec2(width,height), simDynParam);
        simRunParam.runSimulation=true;

        simRunParam.runPolygonPipeline=true;
        simRunParam.drawTriangulatePolygons=true;
        simRunParam.drawTriangulatePolygonsOffset=true;
        simRunParam.drawPolyConstraints=true;
        simRunParam.drawPolygons=false;
        simRunParam.drawConstraints=false;
        simRunParam.drawPoints=false;
        simRunParam.drawTriangulatePolygonsOffsetGA=false;


    }

    return true;
}

bool GraphModule::FrameBufferDrawStep()
{

    glBindFramebuffer(GL_FRAMEBUFFER, graphicProcessor.renderscreenQuad.framebuffer);
   // glEnable(GL_DEPTH_TEST);
    // make sure we clear the framebuffer's content
    glClearColor ( simDynParam.backgroundColor.r, simDynParam.backgroundColor.g, simDynParam.backgroundColor.b, 1.0f );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, width, height);

    Step();

    CHECKGLERROR();
    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    //clear all relevant buffers
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    //glClear(GL_COLOR_BUFFER_BIT);
    shaderutils.teardown(viewport);

    glViewport(0, 0, mainScreenWidth, mainScreenHeight);
    glUseProgram(graphicProcessor.framebufferBox.prog);
    {
          glBindVertexArray(graphicProcessor.framebufferBox.vao);
          glDisable(GL_DEPTH_TEST);
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D,graphicProcessor.renderscreenQuad.textureColorbuffer);
          // Set the sampler texture unit to 0
          glUniform1i(graphicProcessor.framebufferBox.uniform.ubo, 0);
          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
          glBindVertexArray(0);
    }
    glUseProgram(0u);

CHECKGLERROR();

    return true;
}


bool GraphModule::Step()
{


    //graph_module->draw_FrameBuffer();
    //graph_module->draw_Canvas();
    if(simRunParam.runSimulation){
         solver.Step(simDynParam);
         simRunParam.runSimulation=false;
    }

    if(simRunParam.runRemoveSimpleConstraint){
         simRunParam.runRemoveSimpleConstraint=false;
         solver.RemoveSimpleConstraint(simDynParam, simRunParam.runRemoveSimpleConstraint);

    }

    if(simRunParam.generatePolygons){
        simRunParam.generatePolygons=false;

        //polygonsList=solver.polyLib.testForPolygons(solver.pointList, solver.constraintList);
        polygonsList=solver.polyLib.testForPolygons(solver.pointList, solver.constraintList, simDynParam);
      //  if(polygonsList.size()>0)
      //      simRunParam.drawPolygons=true;
    }

    if(simRunParam.generateSweepLine) {
        simRunParam.generateSweepLine=false;
        solver.RemoveSimpleConstraint(simDynParam, simRunParam.runRemoveSimpleConstraint);
        //polygonsList=solver.polyLib.testForPolygons(solver.pointList, solver.constraintList);
        solver.polyLib.FindIntersections(solver.pointList, solver.constraintList);
      //  if(polygonsList.size()>0)
      //      simRunParam.drawPolygons=true;
    }

    if(simRunParam.generateOffset){
        simRunParam.generateOffset=false;
        polygonsListOffset=solver.polyLib.offsetPolygons(simRunParam.polygonOffest );
    }

    if(simRunParam.triangulatePolygons){
        simRunParam.triangulatePolygons=false;
        //solver.polyLib.Triangulation(polygonsList);
        solver.polyLib.triangles_draw_vertex=solver.polyLib.TriangulationPolygon(polygonsList, false, solver.palette, simDynParam.enablePolygonMerging);
        solver.polyLib.trianglesOffset_draw_vertex=solver.polyLib.TriangulationPolygon(polygonsListOffset, true, solver.palette, simDynParam.enablePolygonMerging);

    }

    if(simRunParam.recolorPolygons){
        simRunParam.recolorPolygons=false;
        solver.polyLib.RecolorPolygons(solver.palette);
    }

    if(simRunParam.runPolygonPipeline){
        simRunParam.runPolygonPipeline=false;

        // Step 1: Clean and segment the initial physics network
        solver.RemoveSimpleConstraint(simDynParam, simRunParam.runRemoveSimpleConstraint);
        solver.RemoveSimpleConstraint(simDynParam, simRunParam.runRemoveSimpleConstraint);
        solver.polyLib.FindIntersections(solver.pointList, solver.constraintList);
        solver.polyLib.FindIntersections(solver.pointList, solver.constraintList);

        // Step 2: If fractal subdivision is enabled, execute it recursively
        if (simDynParam.enableFractalSubdivision && simDynParam.fractalSubdivisionDepth > 0)
        {
            for (int depth = 0; depth < simDynParam.fractalSubdivisionDepth; ++depth)
            {
                // Detect polygons on current temporary lines
                polygonsList = solver.polyLib.testForPolygons(solver.pointList, solver.constraintList, simDynParam);

                std::vector<std::vector<glm::vec2>> newPolygonsToAdd;

                // Process each polygon and find large ones to scale, rotate, shift
                for (const auto& poly : polygonsList)
                {
                    int n = poly.size();
                    int nUnique = n;
                    if (n >= 2 && glm::distance(poly.front(), poly.back()) < 1e-4f) {
                        nUnique = n - 1;
                    }
                    if (nUnique < 3) continue;

                    // Calculate area using Shoelace formula
                    double area = 0.0;
                    for (int i = 0; i < nUnique; ++i) {
                        glm::vec2 p1 = poly[i];
                        glm::vec2 p2 = poly[(i + 1) % nUnique];
                        area += (p1.x * p2.y - p2.x * p1.y);
                    }
                    area = std::abs(area) * 0.5;

                    if (area > simDynParam.fractalSubdivisionMinArea)
                    {
                        // Centroid
                        glm::vec2 centroid(0.0f);
                        for (int i = 0; i < nUnique; ++i) {
                            centroid += poly[i];
                        }
                        centroid /= (float)nUnique;

                        // Shift center slightly
                        float angle_shift = solver.randomFloat(0.0f, M_PI * 2.0f);
                        glm::vec2 shiftedCenter = centroid + glm::vec2(cos(angle_shift), sin(angle_shift)) * simDynParam.fractalSubdivisionShift;

                        // Scale and rotate vertices
                        float rad = simDynParam.fractalSubdivisionRotateRange * (M_PI / 180.0f);
                        float theta = solver.randomFloat(-rad, rad);
                        float scale = simDynParam.fractalSubdivisionScale;

                        std::vector<glm::vec2> subPoly;
                        for (int i = 0; i < nUnique; ++i)
                        {
                            glm::vec2 relative = poly[i] - shiftedCenter;
                            glm::vec2 rotated = glm::vec2(
                                relative.x * cos(theta) - relative.y * sin(theta),
                                relative.x * sin(theta) + relative.y * cos(theta)
                            );
                            subPoly.push_back(shiftedCenter + rotated * scale);
                        }
                        subPoly.push_back(subPoly.front()); // Close the loop
                        newPolygonsToAdd.push_back(subPoly);
                    }
                }

                if (newPolygonsToAdd.empty())
                    break; // No more polygons to subdivide

                // Insert the new boundary lines into polyLib's temporary lists
                for (const auto& poly : newPolygonsToAdd)
                {
                    std::vector<int> pointIndices;
                    for (int i = 0; i < (int)poly.size() - 1; ++i)
                    {
                        PointData tempPt;
                        tempPt.coord = poly[i];
                        tempPt.radius = particleRadius_init;
                        tempPt.constructSize = 0;
                        tempPt.constructSizeMax = simDynParam.maxConstraintsPerPoint;
                        solver.polyLib.pointListTemp.push_back(tempPt);
                        pointIndices.push_back(solver.polyLib.pointListTemp.size() - 1);
                    }

                    // Create constraints to form the closed loop
                    for (size_t i = 0; i < pointIndices.size(); ++i)
                    {
                        int id1 = pointIndices[i];
                        int id2 = pointIndices[(i + 1) % pointIndices.size()];
                        float dist = glm::distance(solver.polyLib.pointListTemp[id1].coord, solver.polyLib.pointListTemp[id2].coord);

                        Constraint constraintTemp;
                        constraintTemp.id1 = id1;
                        constraintTemp.id2 = id2;
                        constraintTemp.clength = dist;
                        constraintTemp.cactive = true;
                        solver.polyLib.constraintListTemp.push_back(constraintTemp);
                    }
                }

                // Copy to local vectors to pass to FindIntersections safely (avoiding reference-clearing aliasing)
                std::vector<PointData> inputPoints = solver.polyLib.pointListTemp;
                std::vector<Constraint> inputConstraints = solver.polyLib.constraintListTemp;

                // Re-intersect the entire compound network
                solver.polyLib.FindIntersections(inputPoints, inputConstraints);
                solver.polyLib.FindIntersections(inputPoints, inputConstraints);
            }
        }

        // Step 3: Run final polygon detection and triangulation
        polygonsList=solver.polyLib.testForPolygons(solver.pointList, solver.constraintList, simDynParam);
        solver.polyLib.Triangulation(polygonsList, solver.palette);

        polygonsListOffset=solver.polyLib.offsetPolygons(simRunParam.polygonOffest );

        solver.polyLib.triangles_draw_vertex=solver.polyLib.TriangulationPolygon(polygonsList, false, solver.palette, simDynParam.enablePolygonMerging);
        solver.polyLib.trianglesOffset_draw_vertex=solver.polyLib.TriangulationPolygon(polygonsListOffset, true, solver.palette, simDynParam.enablePolygonMerging);
    }

    if(simRunParam.drawConstraints)
        draw_Constraints();
#ifdef FXPUBLISH
    if(simRunParam.drawPoints)
        draw_Points(solver.particleList);
#endif

    if(simRunParam.drawTriangulatePolygons)
        draw_PolygonsSort(solver.polyLib.triangles_draw_vertex,solver.polyLib.triangles_draw_index);
    if(simRunParam.drawTriangulatePolygonsOffset)
        draw_PolygonsSort(solver.polyLib.trianglesOffset_draw_vertex,solver.polyLib.triangles_draw_index);

    if(simRunParam.drawPolyConstraints)
        draw_Constraints(solver.polyLib.constraintListDraw);

    if(simRunParam.drawPolygons)
        draw_Polygons();
#ifdef FXPUBLISH
    if(simRunParam.drawNewPoints)
        draw_Points(solver.polyLib.particleListTemp);
#endif

    if(simRunParam.drawTriangulatePolygonsGA)
         draw_PolygonsUV(solver.polyLib.arr.triangles_draw_vertexUV,solver.polyLib.triangles_draw_index);

    if(simRunParam.drawTriangulatePolygonsOffsetGA)
         draw_PolygonsUV(solver.polyLib.arr.triangles_draw_vertexOffsetUV,solver.polyLib.triangles_draw_index);

    return true;
}

