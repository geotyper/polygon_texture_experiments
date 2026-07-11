#include "graphmodule.hpp"


float GraphModule::sign(float _val){
    if(_val<0)
        return -1.0;
    else
        return 1.0;
}

void GraphModule::pointCircleMove(glm::vec3& _out, float _angle){
   float sa = cosf(_angle);
   float ca = sinf(_angle);
   _out.x = sa;
   _out.y= ca;
}


void GraphModule::squircle(glm::vec3& _out,  float _angle){
   float sa =sinf(_angle);
   float ca =cosf(_angle);
   _out.x = sqrt(abs(sa) ) * sign(sa);
   _out.y = sqrt(abs(ca) ) * sign(ca);
}


void GraphModule::Box_create()
{
    glGenVertexArrays(1u, &graphicProcessor.box.vao);
    glBindVertexArray(graphicProcessor.box.vao);

    {   // Make and bind a VBO that draws a simple circle

        glm::vec4 color=glm::vec4(0.1,0.5,0.9,1.0);
        VertexPosColor verts[] = {
            {glm::vec2{-0.5f, -0.5f}, color},
            {glm::vec2{ 0.5f, -0.5f}, color},
            {glm::vec2{ 0.5f,  0.5f}, color},
            {glm::vec2{ -0.5f, 0.5f}, color}
            };

        size_t bytes =sizeof(verts);

        glGenBuffers(1u, &graphicProcessor.box.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.box.vbo);
        glBufferData(GL_ARRAY_BUFFER, bytes, verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);

        CHECKGLERROR();
    }

    glBindVertexArray(0);
    CHECKGLERROR();
}

void GraphModule::FramebufferBox_create()
{
    glGenVertexArrays(1u, &graphicProcessor.box.vao);
    glBindVertexArray(graphicProcessor.framebufferBox.vao);

    {   // Make and bind a VBO that draws a simple circle

        VertexPosUV verts[] = {
            {glm::vec2{ -1.0f, -1.0f}, glm::vec2{0.0f, 0.0f}},
            {glm::vec2{  1.0f, -1.0f}, glm::vec2{1.0f, 0.0f}},
            {glm::vec2{  1.0f,  1.0f}, glm::vec2{1.0f, 1.0f}},
            {glm::vec2{ -1.0f,  1.0f}, glm::vec2{0.0f, 1.0f}}
            };

        size_t bytes =sizeof(verts);

        glGenBuffers(1u, &graphicProcessor.framebufferBox.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.framebufferBox.vbo);
        glBufferData(GL_ARRAY_BUFFER, bytes, verts, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0u);

        CHECKGLERROR();
    }

    glBindVertexArray(0);
    CHECKGLERROR();
}

void GraphModule::CircleInCirclrCreate(GraphicProcessor_::MeshBase_& meshIn, float radius, int numberSegments)
{
        float scale=1.0f;

        int num=64;
        float radius1=1.0;
        float radius2=0.25;
        const uint numVertices = numberSegments*2;
        const float step = M_PI * 2.0f / (float)numberSegments;

        std::vector<uint32_t> indexBuffer;
        std::vector<VertexPosColor> vertexBuffer;

        int indexVertex=0;

        for (uint ii = 1; ii <=numberSegments; ++ii)
        {
            const float angle = step * ii;

            glm::vec3 tempPoint(0);
            pointCircleMove(tempPoint, angle);
            glm::vec3 tempPointN1(0.0,1.0,0.0);

            VertexPosColor tempPoint1;
            tempPoint1.pos.x = tempPoint.x*radius1;
            tempPoint1.pos.y =  tempPoint.y*radius1;
            tempPoint1.color=glm::vec4(0.0,0.9,0.7,1.0);
            vertexBuffer.push_back(tempPoint1);
        }
        for (uint ii = 1; ii <=numberSegments; ++ii)
        {
            const float angle = step * ii;

            glm::vec3 tempPointRange(0.25,0.0,0.0);
            glm::vec3 tempPoint(0);
            pointCircleMove(tempPoint, angle);
            glm::vec3 tempPointN1(0.0,1.0,0.0);

            VertexPosColor tempPoint1;
            tempPoint1.pos.x = tempPoint.x*radius2+tempPointRange.x;
            tempPoint1.pos.y = tempPoint.y*radius2+tempPointRange.x;
            tempPoint1.color=glm::vec4(0.0,0.9,0.7,1.0);
            vertexBuffer.push_back(tempPoint1);
        }

        for(uint ii = 0; ii <numberSegments-1; ++ii)
        {
                indexBuffer.push_back(ii);
                indexBuffer.push_back(ii+numberSegments);
                indexBuffer.push_back(ii+numberSegments+1);

                indexBuffer.push_back(ii);
                indexBuffer.push_back(ii+numberSegments+1);
                indexBuffer.push_back(ii+1);
       }

        indexBuffer.push_back(numberSegments-1);
        indexBuffer.push_back(numberSegments+numberSegments-1);
        indexBuffer.push_back(numberSegments);

        indexBuffer.push_back(numberSegments-1);
        indexBuffer.push_back(numberSegments);
        indexBuffer.push_back(0);

   meshIn.nvertices=vertexBuffer.size();
   meshIn.nindices=indexBuffer.size();

   glGenVertexArrays(1u, &meshIn.vao);
   glBindVertexArray(meshIn.vao);

   {
       size_t bytes =vertexBuffer.size()*(sizeof(glm::vec2)+ sizeof(glm::vec4));
       glGenBuffers(1u, &meshIn.vbo);
       glBindBuffer(GL_ARRAY_BUFFER, meshIn.vbo);
       glBufferData(GL_ARRAY_BUFFER, bytes, vertexBuffer.data(), GL_STATIC_DRAW);
       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       glGenBuffers(1u, &meshIn.ibo);
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIn.ibo);
       glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size()*sizeof(uint), indexBuffer.data(), GL_STATIC_DRAW);
       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       CHECKGLERROR();
   }

   glBindVertexArray(0);
   CHECKGLERROR();
}




void GraphModule::FramebufferBoxElement()
{
    vector<Vertex3PosUV> vertexBuffer = {
            // positions          // texture coords
             {glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(   1.0f, 1.0f)}, // top right
             {glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(   1.0f, 0.0f)}, // bottom right
             {glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(   0.0f, 0.0f)}, // bottom left
             {glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(   0.0f, 1.0f)}  // top left
        };
     vector<unsigned int> indexBuffer = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };

   graphicProcessor.framebufferBox.nvertices=vertexBuffer.size();
   graphicProcessor.framebufferBox.nindices=indexBuffer.size();

   glGenVertexArrays(1u, &graphicProcessor.framebufferBox.vao);
   glBindVertexArray(graphicProcessor.framebufferBox.vao);
   {
       size_t bytes =vertexBuffer.size()*(sizeof(glm::vec3)+ sizeof(glm::vec2));
       glGenBuffers(1u, &graphicProcessor.framebufferBox.vbo);
       glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.framebufferBox.vbo);
       glBufferData(GL_ARRAY_BUFFER, bytes, vertexBuffer.data(), GL_STATIC_DRAW);

       glEnableVertexAttribArray(0);
       glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
       glEnableVertexAttribArray(1);
       glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (const void*) (3* sizeof(float)));

       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       CHECKGLERROR();

       glGenBuffers(1u, &graphicProcessor.framebufferBox.ibo);
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphicProcessor.framebufferBox.ibo);
       glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size()*sizeof(uint), indexBuffer.data(), GL_STATIC_DRAW);
       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphicProcessor.framebufferBox.ibo);

       CHECKGLERROR();
   }
   glBindVertexArray(0);
   CHECKGLERROR();
}


void GraphModule::CircleCreateSegment(GraphicProcessor_::MeshBase_& meshIn, float radius, int numberSegments)
{
        const float step = M_PI * 2.0f / (float)numberSegments;

        std::vector<uint32_t> indexBuffer;
        std::vector<VertexPosColor> vertexBuffer;

        int indexVertex=0;

        VertexPosColor tempPointCenter;
        tempPointCenter.pos=glm::vec2(0);
        tempPointCenter.color=glm::vec4(0.0,0.9,0.7,1.0);
        vertexBuffer.push_back(tempPointCenter);

        for (uint ii = 1; ii <=numberSegments; ++ii)
        {
            const float angle = step * ii;

            glm::vec3 tempPoint(0);
            pointCircleMove(tempPoint, angle);
            glm::vec3 tempPointN1(0.0,1.0,0.0);

            VertexPosColor tempPoint1;
            tempPoint1.pos.x = tempPoint.x*radius;
            tempPoint1.pos.y = tempPoint.y*radius;
            tempPoint1.color=glm::vec4(0.0,0.9,0.7,1.0);
            vertexBuffer.push_back(tempPoint1);
        }

        for(uint ii = 1; ii<numberSegments-5; ++ii)
        {
                indexBuffer.push_back(0);
                indexBuffer.push_back(ii);
                indexBuffer.push_back(ii+1);

       }

   meshIn.nvertices=vertexBuffer.size();
   meshIn.nindices=indexBuffer.size();

   glGenVertexArrays(1u, &meshIn.vao);
   glBindVertexArray(meshIn.vao);
   {
       size_t bytes =vertexBuffer.size()*sizeof(VertexPosColor);
       glGenBuffers(1u, &meshIn.vbo);
       glBindBuffer(GL_ARRAY_BUFFER, meshIn.vbo);
       glBufferData(GL_ARRAY_BUFFER, bytes, vertexBuffer.data(), GL_STATIC_DRAW);
       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       glGenBuffers(1u, &meshIn.ibo);
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIn.ibo);
       glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size()*sizeof(uint), indexBuffer.data(), GL_STATIC_DRAW);
       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       CHECKGLERROR();
   }

   glBindVertexArray(0);
   CHECKGLERROR();
}




void GraphModule::CircleCreateInstance(GraphicProcessor_::MeshBaseInstance_& meshIn, float radius, int numberSegments, std::vector<InstanceParticle>& particleList_)
{
        const float step = M_PI * 2.0f / (float)numberSegments;

        std::vector<uint32_t> indexBuffer;
        std::vector<VertexPos> vertexBuffer;

        VertexPos tempPointCenter;
        tempPointCenter.pos=glm::vec2(0);
        vertexBuffer.push_back(tempPointCenter);

        for (uint ii = 1; ii <=numberSegments; ++ii)
        {
            const float angle = step * ii;

            glm::vec3 tempPoint(0);
            pointCircleMove(tempPoint, angle);
            glm::vec3 tempPointN1(0.0,1.0,0.0);

            VertexPos tempPoint1;
            tempPoint1.pos.x = tempPoint.x*radius;
            tempPoint1.pos.y = tempPoint.y*radius;
            vertexBuffer.push_back(tempPoint1);
        }

        for(uint ii = 1; ii<numberSegments-5; ++ii)
        {
                indexBuffer.push_back(0);
                indexBuffer.push_back(ii);
                indexBuffer.push_back(ii+1);

       }

   meshIn.vertexBuffer=vertexBuffer;
   meshIn.indexBuffer=indexBuffer;
   meshIn.nvertices=vertexBuffer.size();
   meshIn.nindices=indexBuffer.size();

   meshIn.instanceNum=particleList_.size();

   glGenVertexArrays(1u, &meshIn.vao);
   glBindVertexArray(meshIn.vao);
   {
       size_t bytes =vertexBuffer.size()*sizeof(VertexPos);
       glGenBuffers(1u, &meshIn.vbo);
       glBindBuffer(GL_ARRAY_BUFFER, meshIn.vbo);
       glBufferData(GL_ARRAY_BUFFER, bytes, vertexBuffer.data(), GL_STATIC_DRAW);

       //  glEnableVertexAttribArray(0);
       //  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (2)*sizeof(float), 0);

       glEnableVertexAttribArray(0);
       glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (const void*)(0*sizeof(float)));
       //glVertexAttribDivisor(0,0);
       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       CHECKGLERROR();

       size_t bytesInstance =particleList_.size()*sizeof(InstanceParticle);
       glGenBuffers(1u, &meshIn.vbo_inst);
       glBindBuffer(GL_ARRAY_BUFFER, meshIn.vbo_inst);

       glEnableVertexAttribArray(1);
       glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (const void*) (0* sizeof(float)));
       glVertexAttribDivisor(1,1);

       glEnableVertexAttribArray(2);
       glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 4*sizeof(float), (const void*) (2* sizeof(float)));
       glVertexAttribDivisor(2,1);

       glEnableVertexAttribArray(3);
       glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 4*sizeof(float), (const void*) (3* sizeof(float)));
       glVertexAttribDivisor(3,1);

       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       CHECKGLERROR();

       glGenBuffers(1u, &meshIn.ibo);
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIn.ibo);
       glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size()*sizeof(uint), indexBuffer.data(), GL_STATIC_DRAW);
       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       CHECKGLERROR();

   }
   glBindVertexArray(0);


   CHECKGLERROR();
}


void GraphModule::update_mesh_geometry( GraphicProcessor_::MeshBase_& meshIn,  MeshData& mesh) {

    meshIn.nvertices=mesh.numberVertices();
    meshIn.nindices=mesh.numberIndices();
    // Vertices buffer.
    {
        glBindBuffer(GL_ARRAY_BUFFER, meshIn.vbo);
          size_t  bytesize = meshIn.nvertices * 3*sizeof(float);
          glBufferData(GL_ARRAY_BUFFER, bytesize, mesh.vertices.data(),  GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);
    }
    // Color buffer
    /*
    {
        glBindBuffer(GL_ARRAY_BUFFER, meshIn.vbo_n);
          size_t  bytesize = meshIn.nvertices * 4*sizeof(float);
          glBufferData(GL_ARRAY_BUFFER, bytesize, mesh.colors.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);
    }
    */
    // Indices buffer.
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIn.ibo);
          size_t bytesize = meshIn.nindices* sizeof(uint);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytesize, mesh.triangelVertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
    }
}

void GraphModule::polygonCreate()
{
    glGenVertexArrays(1u, &graphicProcessor.polygon.vao);
    glBindVertexArray(graphicProcessor.polygon.vao);
    {   // Make and bind a VBO that draws a simple circle
        //size_t bytes =sizeof(verts);
        vector<TrianglesDrawStruct> triangles_draw_vertex;
        vector<int> triangles_draw_index;
        for(int i=0; i<10; i++)
        {
            TrianglesDrawStruct temp;
            triangles_draw_vertex.push_back(temp);
            triangles_draw_index.push_back(i);
        }
        glGenBuffers(1u, &graphicProcessor.polygon.vbo);
        glBindBuffer(GL_ARRAY_BUFFER,graphicProcessor.polygon.vbo);
      //  glBufferData(GL_ARRAY_BUFFER,triangles_draw_vertex.size()*sizeof(TrianglesDrawStruct), triangles_draw_vertex.data(), GL_DYNAMIC_DRAW);
      //  glBindBuffer(GL_ARRAY_BUFFER, 0u);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3*sizeof(glm::vec2), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 3*sizeof(glm::vec2), (void*)(sizeof(glm::vec2)));

        glBindBuffer(GL_ARRAY_BUFFER, 0u);

        CHECKGLERROR();

        glGenBuffers(1u, &graphicProcessor.polygon.ibo);
        glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.polygon.ibo);
       // glBufferData(GL_ELEMENT_ARRAY_BUFFER,triangles_draw_index.size()*sizeof(int), triangles_draw_index.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);
    }
    glBindVertexArray(0);
    CHECKGLERROR();
}



void GraphModule::createPolygonGA()
{
    vector<VertexPosUV> vertexBuffer = {
            // positions          // texture coords
             {glm::vec2( 1.0f,  1.0f), glm::vec2(   1.0f, 1.0f)}, // top right
             {glm::vec2( 1.0f, -1.0f), glm::vec2(   1.0f, 0.0f)}, // bottom right
             {glm::vec2(-1.0f, -1.0f), glm::vec2(   0.0f, 0.0f)}, // bottom left
             {glm::vec2(-1.0f,  1.0f), glm::vec2(   0.0f, 1.0f)}  // top left
        };
     vector<unsigned int> indexBuffer = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };

   graphicProcessor.polygonGA.nvertices=vertexBuffer.size();
   graphicProcessor.polygonGA.nindices=indexBuffer.size();

   glGenVertexArrays(1u, &graphicProcessor.polygonGA.vao);
   glBindVertexArray(graphicProcessor.polygonGA.vao);

   {
       size_t bytes =vertexBuffer.size()*(sizeof(glm::vec2)+ sizeof(glm::vec2));
       glGenBuffers(1u, &graphicProcessor.polygonGA.vbo);
       glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.polygonGA.vbo);
       glBufferData(GL_ARRAY_BUFFER, bytes, vertexBuffer.data(), GL_DYNAMIC_DRAW);

       glEnableVertexAttribArray(0);
       glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
       glEnableVertexAttribArray(1);
       glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (const void*) (2* sizeof(float)));

       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       CHECKGLERROR();

       glGenBuffers(1u, &graphicProcessor.polygonGA.ibo);
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphicProcessor.polygonGA.ibo);
       glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size()*sizeof(uint), indexBuffer.data(), GL_DYNAMIC_DRAW);
       glBindBuffer(GL_ARRAY_BUFFER, 0u);

       // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphicProcessor.framebufferBox.ibo);

       CHECKGLERROR();
   }

   glBindVertexArray(0);
   CHECKGLERROR();
}


GLuint GraphModule::texture_new()
{
    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return tex;
}

GLuint GraphModule::texture32F_new()
{
    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return tex;
}
GLuint  GraphModule::setup_texture_emptyWHRGBA32F(int widths_, int heights_) {

    int widths, heights, nrChannels;
    width=widths_;
    height=heights_;

    GLuint id=texture_new();

    GLsizei const w = static_cast<GLsizei>(width);
    GLsizei const h = static_cast<GLsizei>(height);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0,   GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    //glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA32F, w, h);
    glBindTexture(GL_TEXTURE_2D, 0u);
    CHECKGLERROR();

    return id;
}


void GraphModule::Constraint_create()
{

    glGenVertexArrays(1u, &graphicProcessor.constraints.vao);
    glBindVertexArray(graphicProcessor.constraints.vao);
    {   // Make and bind a VBO that draws a simple circle

        CHECKGLERROR();

        size_t bytes =solver.constraintListDraw.size() *sizeof(glm::vec2);

        glGenBuffers(1u, &graphicProcessor.constraints.vbo);
        CHECKGLERROR();
        glBindBuffer(GL_ARRAY_BUFFER, graphicProcessor.constraints.vbo);
        glBufferData(GL_ARRAY_BUFFER, bytes,solver.constraintListDraw.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0u);

        CHECKGLERROR();
    }
    glBindVertexArray(0);
    CHECKGLERROR();
}
