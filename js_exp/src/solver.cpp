#include "solver.hpp"


float Solver::randomFloat(float min, float max)
{
    float r3 = min + (float) (rand()) /((float)(RAND_MAX/(max-min)));
    return r3;
}

// 2D Hash function for value noise
float Solver::hash2D(int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

// Interpolated 2D value noise
float Solver::noise2D(float x, float y) {
    int ix = (int)floor(x);
    int iy = (int)floor(y);
    float fx = x - ix;
    float fy = y - iy;

    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);

    float v00 = hash2D(ix, iy);
    float v10 = hash2D(ix + 1, iy);
    float v01 = hash2D(ix, iy + 1);
    float v11 = hash2D(ix + 1, iy + 1);

    float a = v00 * (1.f - ux) + v10 * ux;
    float b = v01 * (1.f - ux) + v11 * ux;

    return a * (1.f - uy) + b * uy;
}

// Fractal Brownian Motion (fBm)
float Solver::fBm2D(float x, float y, int octaves, float lacunarity, float persistence) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        total += noise2D(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return total / maxValue;
}


Solver::Solver():
    ga(settings)
{

    palette=(unsigned int)randomFloat(0,4);
}


void Solver::texturesInit()
{

    palette=(unsigned int)randomFloat(0,4);
    function_image.id=function_image.texture_new();
    function_image.height=settings.resolution;
    function_image.width=settings.resolution;

    for(int i=0; i<lineSize*lineSize; i++)
    {
        Texture tempTexture;
        tempTexture.id=function_image.texture_new();
        tempTexture.height=settings.resolution;
        tempTexture.width=settings.resolution;
        textureList.push_back(tempTexture);
        textureIndexList.push_back(tempTexture.id);
    }
    fillAllTextures();
}


bool Solver::generate_image(int genMode)
{

    auto res=ga.generate_texture(colors, stat, (unsigned int)randomFloat(0,100000), palette, genMode);

    if(res==false)
        return false;

    GLsizei const w = static_cast<GLsizei>(function_image.width);
    GLsizei const h = static_cast<GLsizei>(function_image.height);

    glBindTexture(GL_TEXTURE_2D,   function_image.id);
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB, w, h, 0, GL_RGB,  GL_UNSIGNED_BYTE, colors.data());
    glBindTexture(GL_TEXTURE_2D, 0u);
    CHECKGLERROR();

    {
        GLuint tempIndex=textureIndexList[lineSize*lineSize-1];
        GLuint tempIndexBegin=textureIndexList[lineSize*lineSize-1];

        glBindTexture(GL_TEXTURE_2D, tempIndex);
        glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB, w, h, 0, GL_RGB,  GL_UNSIGNED_BYTE, colors.data());
        glBindTexture(GL_TEXTURE_2D, 0u);
        CHECKGLERROR();

        auto it=textureIndexList.begin();
        textureIndexList.insert(it,tempIndex);
        {
            auto it=textureIndexList.begin();
            textureIndexList.erase(it+ lineSize*lineSize);
        }
    }

    return true;

}


void Solver::fillAllTextures(int genMode)
{
    GLsizei const w = static_cast<GLsizei>(function_image.width);
    GLsizei const h = static_cast<GLsizei>(function_image.height);

    // Update the separate preview texture (ID 2) so it stays in sync
    ga.generate_texture(colors, stat, (unsigned int)randomFloat(0, 100000), palette, genMode);
    glBindTexture(GL_TEXTURE_2D, function_image.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, colors.data());

    for (size_t i = 0; i < textureIndexList.size(); ++i)
    {
        ga.generate_texture(colors, stat, (unsigned int)randomFloat(0, 100000), palette, genMode);
        glBindTexture(GL_TEXTURE_2D, textureIndexList[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, colors.data());
    }
    glBindTexture(GL_TEXTURE_2D, 0u);
    CHECKGLERROR();
}



float Solver::sign(float _val){
    if(_val<0)
        return -1.0;
    else
        return 1.0;
}

void Solver::pointCircleMove(glm::vec3& _out, float _angle){
   float sa = cosf(_angle);
   float ca = sinf(_angle);
   _out.x = sa;
   _out.y= ca;
}


void Solver::squircle(glm::vec3& _out,  float _angle){
   float sa =sinf(_angle);
   float ca =cosf(_angle);
   _out.x = sqrt(abs(sa) ) * sign(sa);
   _out.y = sqrt(abs(ca) ) * sign(ca);
}


void Solver::initParticles(int numParticles, glm::vec2 worldSize_, SimDynamicParameters& simDynParams)
{
    worldSize=worldSize_;
    // Initial particle positions
    particleList.clear();
    pointList.clear();

    glm::vec2 center{worldSize.x/2,worldSize.y/2};
    float rnd_range=simDynParams.pointsMoveRange;
    float step=simDynParams.initConstraintDist;
    float radiusStep=step;

    if (simDynParams.pointPlacementMode == 0) // Radial
    {
        float targetMaxRadius = std::min(worldSize.x, worldSize.y) * 0.40f;
        float autoStep = targetMaxRadius * std::sqrt(M_PI / (float)numParticles);
        step = autoStep;
        radiusStep = step;
        simDynParams.initConstraintDist = autoStep * 1.15f;

        int numberCircles=75;
        for (auto i = 1; i < numberCircles ; i++) {
            const unsigned int numberSegments= M_PI * 2.0f*(radiusStep*i)/step;
            const float stepA = M_PI * 2.0f / (float)numberSegments;

            for (unsigned int  ii = 1; ii <=numberSegments; ++ii)
            {
                if ((int)pointList.size() >= numParticles)
                    break;

                const float angle = stepA * ii;

                glm::vec3 tempPoint(0);
                pointCircleMove(tempPoint, angle);

                PointData tempPoint1;
                tempPoint1.coord.x = center.x+tempPoint.x*(radiusStep*i);
                tempPoint1.coord.y = center.y+tempPoint.y*(radiusStep*i);

                if (simDynParams.useNoiseDisplacement) {
                    float dx = fBm2D(tempPoint1.coord.x * simDynParams.noiseScale, tempPoint1.coord.y * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                    float dy = fBm2D((tempPoint1.coord.x + 1000.f) * simDynParams.noiseScale, (tempPoint1.coord.y + 1000.f) * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                    tempPoint1.coord += glm::vec2(dx, dy) * rnd_range * 6.0f;
                } else {
                    tempPoint1.coord+=glm::vec2(randomFloat(-rnd_range,rnd_range),randomFloat(-rnd_range,rnd_range));
                }

                tempPoint1.radius =particleRadius_init;
                tempPoint1.constructSize=0;
                tempPoint1.constructSizeMax=simDynParams.maxConstraintsPerPoint;

                pointList.push_back(tempPoint1);
            }

            if ((int)pointList.size() >= numParticles)
                break;

            if((radiusStep*i)>center.x*1.5)
                break;
        }

        // Fill up to numParticles if the radial constraints were too tight
        while ((int)pointList.size() < numParticles) {
            float angle = randomFloat(0, M_PI * 2.0f);
            float radius = randomFloat(0, center.x * 0.9f);
            PointData tempPoint1;
            tempPoint1.coord.x = center.x + cosf(angle) * radius;
            tempPoint1.coord.y = center.y + sinf(angle) * radius;

            if (simDynParams.useNoiseDisplacement) {
                float dx = fBm2D(tempPoint1.coord.x * simDynParams.noiseScale, tempPoint1.coord.y * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                float dy = fBm2D((tempPoint1.coord.x + 1000.f) * simDynParams.noiseScale, (tempPoint1.coord.y + 1000.f) * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                tempPoint1.coord += glm::vec2(dx, dy) * rnd_range * 6.0f;
            } else {
                tempPoint1.coord += glm::vec2(randomFloat(-rnd_range,rnd_range),randomFloat(-rnd_range,rnd_range));
            }

            tempPoint1.radius = particleRadius_init;
            tempPoint1.constructSize = 0;
            tempPoint1.constructSizeMax = simDynParams.maxConstraintsPerPoint;
            pointList.push_back(tempPoint1);
        }
    }
    else if (simDynParams.pointPlacementMode == 1) // Rectangular (Grid)
    {
        int cols = sqrt(numParticles);
        if (cols < 1) cols = 1;
        int rows = (numParticles + cols - 1) / cols;
        float margin_x = worldSize.x * 0.05f;
        float margin_y = worldSize.y * 0.05f;
        float step_x = (worldSize.x - 2 * margin_x) / (cols > 1 ? cols - 1 : 1);
        float step_y = (worldSize.y - 2 * margin_y) / (rows > 1 ? rows - 1 : 1);

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if ((int)pointList.size() >= numParticles)
                    break;
                PointData tempPoint;
                tempPoint.coord.x = margin_x + c * step_x;
                tempPoint.coord.y = margin_y + r * step_y;

                if (simDynParams.useNoiseDisplacement) {
                    float dx = fBm2D(tempPoint.coord.x * simDynParams.noiseScale, tempPoint.coord.y * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                    float dy = fBm2D((tempPoint.coord.x + 1000.f) * simDynParams.noiseScale, (tempPoint.coord.y + 1000.f) * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                    tempPoint.coord += glm::vec2(dx, dy) * rnd_range * 6.0f;
                } else {
                    tempPoint.coord += glm::vec2(randomFloat(-rnd_range, rnd_range), randomFloat(-rnd_range, rnd_range));
                }

                tempPoint.radius = particleRadius_init;
                tempPoint.constructSize = 0;
                tempPoint.constructSizeMax = simDynParams.maxConstraintsPerPoint;
                pointList.push_back(tempPoint);
            }
            if ((int)pointList.size() >= numParticles)
                break;
        }
    }
    else if (simDynParams.pointPlacementMode == 2) // Noise-based (Organic)
    {
        int cols = sqrt(numParticles * 3);
        if (cols < 1) cols = 1;
        int rows = cols;
        float margin_x = worldSize.x * 0.05f;
        float margin_y = worldSize.y * 0.05f;
        float step_x = (worldSize.x - 2 * margin_x) / (cols > 1 ? cols - 1 : 1);
        float step_y = (worldSize.y - 2 * margin_y) / (rows > 1 ? rows - 1 : 1);

        std::vector<PointData> candidates;

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                glm::vec2 p(margin_x + c * step_x, margin_y + r * step_y);
                float n = glm::fract(sin(glm::dot(p * 0.015f, glm::vec2(12.9898f, 78.233f))) * 43758.5453f);
                if (n > 0.45f) {
                    PointData tempPoint;
                    tempPoint.coord = p;

                    if (simDynParams.useNoiseDisplacement) {
                        float dx = fBm2D(tempPoint.coord.x * simDynParams.noiseScale, tempPoint.coord.y * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                        float dy = fBm2D((tempPoint.coord.x + 1000.f) * simDynParams.noiseScale, (tempPoint.coord.y + 1000.f) * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                        tempPoint.coord += glm::vec2(dx, dy) * rnd_range * 6.0f;
                    } else {
                        tempPoint.coord += glm::vec2(randomFloat(-rnd_range, rnd_range), randomFloat(-rnd_range, rnd_range));
                    }

                    tempPoint.radius = particleRadius_init;
                    tempPoint.constructSize = 0;
                    tempPoint.constructSizeMax = simDynParams.maxConstraintsPerPoint;
                    candidates.push_back(tempPoint);
                }
            }
        }

        // Shuffle candidates to distribute them uniformly across the entire screen
        std::random_shuffle(candidates.begin(), candidates.end());

        // Copy up to numParticles candidates
        int toCopy = std::min((int)candidates.size(), numParticles);
        for (int idx = 0; idx < toCopy; ++idx) {
            pointList.push_back(candidates[idx]);
        }

        // Fill remaining if needed
        while ((int)pointList.size() < numParticles) {
            PointData tempPoint;
            tempPoint.coord.x = randomFloat(margin_x, worldSize.x - margin_x);
            tempPoint.coord.y = randomFloat(margin_y, worldSize.y - margin_y);

            if (simDynParams.useNoiseDisplacement) {
                float dx = fBm2D(tempPoint.coord.x * simDynParams.noiseScale, tempPoint.coord.y * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                float dy = fBm2D((tempPoint.coord.x + 1000.f) * simDynParams.noiseScale, (tempPoint.coord.y + 1000.f) * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                tempPoint.coord += glm::vec2(dx, dy) * rnd_range * 6.0f;
            } else {
                tempPoint.coord += glm::vec2(randomFloat(-rnd_range, rnd_range), randomFloat(-rnd_range, rnd_range));
            }

            tempPoint.radius = particleRadius_init;
            tempPoint.constructSize = 0;
            tempPoint.constructSizeMax = simDynParams.maxConstraintsPerPoint;
            pointList.push_back(tempPoint);
        }
    }
    else if (simDynParams.pointPlacementMode == 3) // Spiral
    {
        float c_coeff = worldSize.x * 0.45f / sqrt((float)numParticles);
        float goldenAngle = 137.507764f * (M_PI / 180.0f);
        for (int i = 1; i <= numParticles; ++i) {
            float r = c_coeff * sqrt((float)i);
            float theta = i * goldenAngle;
            PointData tempPoint;
            tempPoint.coord.x = center.x + r * cosf(theta);
            tempPoint.coord.y = center.y + r * sinf(theta);

            if (simDynParams.useNoiseDisplacement) {
                float dx = fBm2D(tempPoint.coord.x * simDynParams.noiseScale, tempPoint.coord.y * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                float dy = fBm2D((tempPoint.coord.x + 1000.f) * simDynParams.noiseScale, (tempPoint.coord.y + 1000.f) * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                tempPoint.coord += glm::vec2(dx, dy) * rnd_range * 6.0f;
            } else {
                tempPoint.coord += glm::vec2(randomFloat(-rnd_range, rnd_range), randomFloat(-rnd_range, rnd_range));
            }

            tempPoint.radius = particleRadius_init;
            tempPoint.constructSize = 0;
            tempPoint.constructSizeMax = simDynParams.maxConstraintsPerPoint;
            pointList.push_back(tempPoint);
        }
    }
    else if (simDynParams.pointPlacementMode == 4) // Perspective Grid
    {
        int cols = sqrt(numParticles);
        if (cols < 1) cols = 1;
        int rows = (numParticles + cols - 1) / cols;

        for (int r = 0; r < rows; ++r) {
            float t = (rows > 1) ? (float)r / (rows - 1) : 0.0f;
            // Exponential spacing for rows simulating 3D depth
            float factor_y = 0.05f + 0.95f * powf(t, 2.0f);
            float y = worldSize.y * factor_y;

            // Expand X range at the bottom to cover the entire screen
            float x_min = 0.0f - t * (worldSize.x * 0.4f);
            float x_max = worldSize.x + t * (worldSize.x * 0.4f);

            for (int c = 0; c < cols; ++c) {
                if ((int)pointList.size() >= numParticles)
                    break;
                float u = (cols > 1) ? (float)c / (cols - 1) : 0.5f;
                float x = x_min + u * (x_max - x_min);

                PointData tempPoint;
                tempPoint.coord.x = x;
                tempPoint.coord.y = y;

                if (simDynParams.useNoiseDisplacement) {
                    float dx = fBm2D(tempPoint.coord.x * simDynParams.noiseScale, tempPoint.coord.y * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                    float dy = fBm2D((tempPoint.coord.x + 1000.f) * simDynParams.noiseScale, (tempPoint.coord.y + 1000.f) * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                    tempPoint.coord += glm::vec2(dx, dy) * rnd_range * 6.0f;
                } else {
                    tempPoint.coord += glm::vec2(randomFloat(-rnd_range, rnd_range), randomFloat(-rnd_range, rnd_range));
                }

                tempPoint.radius = particleRadius_init;
                tempPoint.constructSize = 0;
                tempPoint.constructSizeMax = simDynParams.maxConstraintsPerPoint;
                pointList.push_back(tempPoint);
            }
            if ((int)pointList.size() >= numParticles)
                break;
        }
    }
    else if (simDynParams.pointPlacementMode == 5) // Nested Squares
    {
        int numFrames = 15;
        float stepSize = worldSize.x * 0.45f / numFrames;
        int ptsPerFrame = numParticles / numFrames;
        if (ptsPerFrame < 4) ptsPerFrame = 4;

        for (int k = 1; k <= numFrames; ++k) {
            float size = stepSize * k;
            int pts = ptsPerFrame;
            for (int i = 0; i < pts; ++i) {
                if ((int)pointList.size() >= numParticles)
                    break;
                float t = (float)i / pts;
                glm::vec2 p;
                if (t < 0.25f) { // Top edge
                    float u = t / 0.25f;
                    p = glm::vec2(center.x - size + u * 2 * size, center.y - size);
                } else if (t < 0.5f) { // Right edge
                    float u = (t - 0.25f) / 0.25f;
                    p = glm::vec2(center.x + size, center.y - size + u * 2 * size);
                } else if (t < 0.75f) { // Bottom edge
                    float u = (t - 0.5f) / 0.25f;
                    p = glm::vec2(center.x + size - u * 2 * size, center.y + size);
                } else { // Left edge
                    float u = (t - 0.75f) / 0.25f;
                    p = glm::vec2(center.x - size, center.y + size - u * 2 * size);
                }

                PointData tempPoint;
                tempPoint.coord = p;

                if (simDynParams.useNoiseDisplacement) {
                    float dx = fBm2D(tempPoint.coord.x * simDynParams.noiseScale, tempPoint.coord.y * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                    float dy = fBm2D((tempPoint.coord.x + 1000.f) * simDynParams.noiseScale, (tempPoint.coord.y + 1000.f) * simDynParams.noiseScale, simDynParams.octaves, simDynParams.lacunarity, simDynParams.persistence);
                    tempPoint.coord += glm::vec2(dx, dy) * rnd_range * 6.0f;
                } else {
                    tempPoint.coord += glm::vec2(randomFloat(-rnd_range, rnd_range), randomFloat(-rnd_range, rnd_range));
                }

                tempPoint.radius = particleRadius_init;
                tempPoint.constructSize = 0;
                tempPoint.constructSizeMax = simDynParams.maxConstraintsPerPoint;
                pointList.push_back(tempPoint);
            }
            if ((int)pointList.size() >= numParticles)
                break;
        }

        // Fallback to fill exactly numParticles
        while ((int)pointList.size() < numParticles) {
            PointData tempPoint;
            tempPoint.coord.x = randomFloat(center.x - worldSize.x * 0.45f, center.x + worldSize.x * 0.45f);
            tempPoint.coord.y = randomFloat(center.y - worldSize.y * 0.45f, center.y + worldSize.y * 0.45f);
            tempPoint.radius = particleRadius_init;
            tempPoint.coord += glm::vec2(randomFloat(-rnd_range, rnd_range), randomFloat(-rnd_range, rnd_range));
            tempPoint.constructSize = 0;
            tempPoint.constructSizeMax = simDynParams.maxConstraintsPerPoint;
            pointList.push_back(tempPoint);
        }
    }

    particleList.resize(pointList.size());
    for (auto i = 0; i < pointList.size() ; i++) {
        particleList[i].instancePos =  pointList[i].coord;
        particleList[i].instanceScale =pointList[i].radius;
        particleList[i].instanceRot =0;
    }
}



void Solver::initParticles2(int numParticles, glm::vec2 worldSize_, SimDynamicParameters& simDynParams)
{
    worldSize=worldSize_;
    // Initial particle positions
    particleList.clear();
    particleList.resize(numParticles);

    pointList.clear();
    pointList.resize(numParticles);

    int lineWidth=sqrt(numParticles);
    float step=0.95*worldSize.x/lineWidth;
   // glm::vec2 initCoord{-step*lineWidth/2.0f, -step*lineWidth/2.0f };
    glm::vec2 initCoord{worldSize.x/2-step*lineWidth/2.0f,worldSize.y/2-step*lineWidth/2.0f};

    glm::vec2 center{worldSize.x/2,worldSize.y/2};

    float x=0,y=0;
    int counter=0;

    // Distribute rocks randomly on two different rings
    float rnd_range=simDynParams.pointsMoveRange;

    float denzity=randomFloat(8,17);
    for (auto i = 0; i < numParticles ; i++) {

        float resRnd2=randomFloat(0,100);
        if((resRnd2)>denzity)
{
        auto tempCoord=initCoord+glm::vec2(x,y);

        auto deltaT=center-tempCoord;
        float distFromcenterT=glm::length(deltaT);

       // if(int((int)(y/300))%2==0)//and int(y/50)%2==0)
        if(distFromcenterT>worldSize.x/4)//and int(y/50)%2==0)
        {
            pointList[i].coord=tempCoord+glm::vec2(randomFloat(-rnd_range,rnd_range),randomFloat(-rnd_range,rnd_range));
        }
        else
        {
            pointList[i].coord=tempCoord+glm::vec2(randomFloat(-rnd_range/2.0,rnd_range/2.0),randomFloat(-rnd_range/2.0,rnd_range/2.0));
        }
        pointList[i].radius =particleRadius_init;
        pointList[i].constructSize=0;


        if(pointList[i].coord.x<worldSize.x/2.0)
        {
            if(i%250==0)
                pointList[i].constructSizeMax=simDynParams.createConstrParam01;
            else if(i%10==0)
                pointList[i].constructSizeMax=simDynParams.createConstrParam02;
            else
                pointList[i].constructSizeMax=simDynParams.createConstrParam03;

        }
        else
        {
            if(i%330==0)
                pointList[i].constructSizeMax=simDynParams.createConstrParam01;
            else if(i%10==0)
                pointList[i].constructSizeMax=simDynParams.createConstrParam02;
            else
                pointList[i].constructSizeMax=simDynParams.createConstrParam03;

         }


        auto delta=center-pointList[i].coord;
        float distFromcenter=glm::length(delta);

       // if(int((int)(y/300))%2==0)//and int(y/50)%2==0)
        if(distFromcenter<worldSize.x/4 and distFromcenter<worldSize.x>6)//and int(y/50)%2==0)
        {
            float resRnd2=randomFloat(0,100);
            if((resRnd2)<25)
               pointList[i].constructSizeMax=simDynParams.createConstrParam04;
            else
               pointList[i].constructSizeMax=simDynParams.createConstrParam05;
        }
        else{
            float resRnd3=randomFloat(0,100);
            if(resRnd3<30.0)
               pointList[i].constructSizeMax+=simDynParams.createConstrParam06;
            else
               pointList[i].constructSizeMax+=simDynParams.createConstrParam07;
        }
}
        x+=step;
        counter++;
        if(counter>lineWidth-1){
          x=0;
          y+=step;
          counter=0;
        }
    }

    //random_shuffle(pointList.begin(),pointList.end());
    particleList.resize(pointList.size());
    for (auto i = 0; i < pointList.size() ; i++) {
        particleList[i].instancePos =  pointList[i].coord;
        particleList[i].instanceScale =pointList[i].radius;
        particleList[i].instanceRot =0;

    }

}



void Solver::initParticlesTest(int numParticles, glm::vec2 worldSize_, SimDynamicParameters& simDynParams)
{

    worldSize=worldSize_;
    // Initial particle positions
    particleList.clear();
    particleList.resize(numParticles);

    pointList.clear();
    pointList.resize(numParticles);

    int lineWidth=sqrt(numParticles);
    float step=100;
   // glm::vec2 initCoord{-step*lineWidth/2.0f, -step*lineWidth/2.0f };
    glm::vec2 initCoord{200,200};

    glm::vec2 center{worldSize.x/2,worldSize.y/2};

    float x=0,y=0;
    int counter=0;

    // Distribute rocks randomly on two different rings
    float rnd_range=simDynParams.pointsMoveRange;
    for (auto i = 0; i < numParticles ; i++) {

        pointList[i].coord=initCoord+glm::vec2(x,y)+glm::vec2(randomFloat(-rnd_range,rnd_range),randomFloat(-rnd_range,rnd_range));
        pointList[i].radius =particleRadius_init;
        pointList[i].constructSize=0;
/*
        if(i%50==0)
            pointList[i].constructSizeMax=4;
        else if(i%2==0)
            pointList[i].constructSizeMax=1;
        else
            pointList[i].constructSizeMax=2;
*/

        if(int(x/200)%2==0 && int(y/200)%2==0){
            if((rand()%30+1)<3)
               pointList[i].constructSizeMax=9;
            else
               pointList[i].constructSizeMax=9;
        }
        else{
            if((rand()%30+1)<3)
               pointList[i].constructSizeMax=9;
            else
               pointList[i].constructSizeMax=9;
        }

        x+=step;
        counter++;
        if(counter>lineWidth-1){
          x=0;
          y+=step;
          counter=0;
        }
    }

     //std::random_shuffle(pointList.begin(),pointList.end());

     {
         float x=0,y=0;
         int counter=0;

         for (auto i = 0; i < numParticles ; i++) {
             particleList[i].instancePos =  pointList[i].coord;
             particleList[i].instanceScale =pointList[i].radius;
             particleList[i].instanceRot =0;


             x+=step;
             counter++;
             if(counter>lineWidth-1){
               x=0;
               y+=step;
               counter=0;
             }
         }
     }

}


void Solver::Step(SimDynamicParameters& simParam)
{
    #ifdef FXPUBLISH
    std::cout<<"Tick :"<<tick<<std::endl;
    std::cout<<"init dist :"<<simParam.initConstraintDist<<std::endl;
    #endif

    mapPointsUpdate();
    constraintList.clear();
    //constraintListDraw.resize(0);
    constraintListDraw.clear();
    constraintsLookUp.clear();

    for (auto i = 0; i < particleList.size() ; i++) {
       pointList[i].constructSize=0;
    }

    ConstraintUpdate(simParam);
}

void Solver::ParticleMove(SimDynamicParameters &simParam)
{
    for(GLuint  i=0; i<pointList.size();++i)
    {
        PointData& pointA=pointList[i];

        int pX = int(pointList[i].coord.x/float(mapCellSize.x));
        int pY = int(pointList[i].coord.y/float(mapCellSize.y));

        if (pX < 0) pX =0;
        else if (pX > mapSize.x - 1) pX = mapSize.x - 1;

        if (pY < 0) pY = 0;
        else if (pY > mapSize.y - 1)
            pY = mapSize.y - 1;

        int mapLookUp=3;

        for (int mapX = pX -  mapLookUp; mapX <=pX + mapLookUp; mapX++){
            for (int mapY = pY -  mapLookUp; mapY <= pY +  mapLookUp; mapY++){

                if(mapX<0 || mapY<0 || mapX>mapSize.x-1 || mapY>mapSize.y-1)
                     continue;
                int cellAmount=gridObject.at(mapX,mapY).pointsMapList.size();

                if(cellAmount==0)// or cellAmount==NULL)
                    continue;

                for(GLuint  ip=0; ip<cellAmount;ip++)
                {
                    GLuint  ipReal=gridObject.at(mapX,mapY).pointsMapList[ip];
                    GLuint  iReal=i;
                    //std::random_shuffle(gridObject.at(mapX,mapY).pointsMapList.begin(),gridObject.at(mapX,mapY).pointsMapList.end());

                    if(ipReal==i)
                        continue;







                }
            }
        }
    }
}

float Solver::AngleBetweenVectors(glm::vec2 pointAcoord,glm::vec2 pointBcoord)
{
    float dot = pointAcoord.x*pointBcoord.x + pointAcoord.y*pointBcoord.y;    // # dot product between [x1, y1] and [x2, y2]
    float det = pointAcoord.x*pointBcoord.y - pointAcoord.y*pointBcoord.x;    // # determinant
    float angle = atan2(det, dot);
    return angle;

}

void Solver::ConstraintUpdate(SimDynamicParameters& simParam)
{
    constraintList.clear();
    constraintListDraw.clear();
    constraintsLookUp.clear();

    for (size_t i = 0; i < pointList.size(); ++i)
    {
        pointList[i].constructSize = 0;
    }

    std::vector<std::pair<int, int>> candidatePairs;

    for (int i = 0; i < (int)pointList.size(); ++i)
    {
        PointData& pointA = pointList[i];

        int pX = int(pointA.coord.x / float(mapCellSize.x));
        int pY = int(pointA.coord.y / float(mapCellSize.y));

        if (pX < 0) pX = 0;
        else if (pX > mapSize.x - 1) pX = mapSize.x - 1;

        if (pY < 0) pY = 0;
        else if (pY > mapSize.y - 1) pY = mapSize.y - 1;

        int mapLookUp = 3;

        for (int mapX = pX - mapLookUp; mapX <= pX + mapLookUp; mapX++) {
            for (int mapY = pY - mapLookUp; mapY <= pY + mapLookUp; mapY++) {

                if (mapX < 0 || mapY < 0 || mapX > mapSize.x - 1 || mapY > mapSize.y - 1)
                    continue;

                int cellAmount = gridObject.at(mapX, mapY).pointsMapList.size();
                if (cellAmount == 0)
                    continue;

                for (GLuint ip = 0; ip < (GLuint)cellAmount; ip++)
                {
                    GLuint ipReal = gridObject.at(mapX, mapY).pointsMapList[ip];
                    if (ipReal == (GLuint)i)
                        continue;

                    GLuint resultIndex = indexConstraint(i, ipReal);
                    auto findKey = constraintsLookUp.find(resultIndex);

                    if (findKey != constraintsLookUp.end())
                        continue;
                    else
                        constraintsLookUp.insert({resultIndex, true});

                    candidatePairs.push_back({i, (int)ipReal});
                }
            }
        }
    }

    // Shuffle candidate pairs to generate constraints in random order
    std::random_shuffle(candidatePairs.begin(), candidatePairs.end());

    // Construct constraints from shuffled pairs
    for (auto& pair : candidatePairs)
    {
        int id1 = pair.first;
        int id2 = pair.second;

        PointData& pointA = pointList[id1];
        PointData& pointB = pointList[id2];

        if (pointA.constructSize < pointA.constructSizeMax && pointB.constructSize < pointB.constructSizeMax)
        {
            float dist = glm::distance(pointA.coord, pointB.coord);
            if (dist >= simParam.minConstraintDist && dist < simParam.initConstraintDist)
            {
                Constraint constraintTemp;
                constraintTemp.id1 = id1;
                constraintTemp.id2 = id2;
                constraintTemp.clength = dist;
                constraintTemp.cactive = true;
                constraintList.push_back(constraintTemp);

                pointA.constructSize++;
                pointB.constructSize++;

                constraintListDraw.push_back(pointA.coord);
                constraintListDraw.push_back(pointB.coord);
            }
        }
    }
}

//border conditions
void Solver::mapPointsUpdate()
{
    gridObject.clearCells();
    for(int i=0; i<pointList.size();++i)
    {
        int x = int(pointList[i].coord.x/mapCellSize.x);
        int y = int(pointList[i].coord.y/mapCellSize.y);

        if (x <0.0)
         x = 0.0;
        else
          if (x > mapSize.x - 1.0) x = mapSize.x - 1.0;

        if (y < 0.0)
         y =0.0;
        else
          if (y > mapSize.y - 1.0)
            y =mapSize.y - 1.0;

        gridObject.at(x,y).pointsMapList.push_back(i);
    }
}


void Solver::RemoveSimpleConstraint(SimDynamicParameters& simParam, bool& resume)
{
    bool findOne=true;

    std::vector<Constraint> constraintListTemp;//=constraintList;

    //while(findOne)
    {
        findOne=false;

        for(int i=0;i<constraintList.size();++i)
        {
            if(constraintList[i].cactive==true)
            {
                if(pointList[constraintList[i].id1].constructSize==1 or pointList[constraintList[i].id2].constructSize==1)
                {
                    constraintList[i].cactive=false;

                    pointList[constraintList[i].id1].constructSize--;

                    if(pointList[constraintList[i].id1].constructSize<0)
                        pointList[constraintList[i].id1].constructSize=0;
                    pointList[constraintList[i].id2].constructSize--;
                    if(pointList[constraintList[i].id2].constructSize<0)
                        pointList[constraintList[i].id2].constructSize=0;

                     findOne=true;

                }
                /*
                if(pointList[constraintList[i].id2].constructSize==1)
                {
                    constraintList[i].cactive=false;

                    pointList[constraintList[i].id2].constructSize--;
                    findOne=true;
                    if(pointList[constraintList[i].id1].constructSize<0)
                        pointList[constraintList[i].id1].constructSize=0;
                    pointList[constraintList[i].id2].constructSize--;
                    if(pointList[constraintList[i].id2].constructSize<0)
                        pointList[constraintList[i].id2].constructSize=0;
                }
                */
            }
        }

    }

    if(findOne)
    {
        resume=true;
    }

    for(int i=0;i<constraintList.size();++i)
    {
        if(constraintList[i].cactive==true)
        {
            constraintListTemp.push_back(constraintList[i]);
        }
    }

    constraintList=constraintListTemp;

    constraintListDraw.clear();

    for(int i=0;i<constraintList.size();++i)
    {
        constraintListDraw.push_back(pointList[constraintList[i].id1].coord);
        constraintListDraw.push_back(pointList[constraintList[i].id2].coord);
    }


}
