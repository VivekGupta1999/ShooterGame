#include "Game.h"
#include "Wavefront.h"

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

Game::Game()
    : mUColorProgram(0)
    , mVColorProgram(0)
    , mUColorDirLightProgram(0)
    , mPlane(NULL)
    , mWorldAxes(NULL)
    , mMeshIndex(0)
    , mShowAxes(true)
    , mCamera(NULL)
{
}

Game::~Game()
{
}

std::vector<std::string> Game::LoadAssetList(const std::string& fname)
{
    std::vector<std::string> names;

    // open the file
    std::ifstream f(fname);

    // check if file opened ok
    if (!f) {
        std::cerr << "Error: Failed to open " << fname << std::endl;
        std::exit(1);
    }

    // read lines until the end of file
    std::string line;
    while (std::getline(f, line)) {
        // ignore blank lines and comments starting with '#'
        if (!line.empty() && line[0] != '#') {
            names.push_back(line);
        }
    }

    return names;
}

bool Game::initialize(int w, int h)
{
    // set screen clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);    // !!!!!!111!!1!!!11!^&#(!@^(!!!!!!

    glEnable(GL_CULL_FACE);

    // load all meshes listed in the asset file
    // - comment out the meshes that you cannot load yet!
    std::vector<std::string> meshNames = LoadAssetList("meshes/meshes.txt");
    for (unsigned i = 0; i < meshNames.size(); i++) {
        mMeshes.push_back(LoadWavefrontOBJ("meshes/" + meshNames[i]));
    }

    mPlane = glsh::CreateWireframePlane(100, 100, 100, 100);
    mWorldAxes = glsh::CreateFullAxes(50);

    mUColorProgram = glsh::BuildShaderProgram("shaders/ucolor-vs.glsl", "shaders/ucolor-fs.glsl");
    mVColorProgram = glsh::BuildShaderProgram("shaders/vcolor-vs.glsl", "shaders/vcolor-fs.glsl");
    mUColorDirLightProgram = glsh::BuildShaderProgram("shaders/ucolor-DirLight-vs.glsl", "shaders/ucolor-DirLight-fs.glsl");

    mPrograms.push_back(mUColorProgram);
    mPrograms.push_back(mVColorProgram);
    mPrograms.push_back(mUColorDirLightProgram);

    mCamera = new glsh::FreeLookCamera(this);
    mCamera->setPosition(0, 3, 12);
    mCamera->lookAt(0, 0, -12);

    return true;
}

void Game::shutdown()
{
    // FIXME: cleanup
}

void Game::resize(int w, int h)
{
    // set viewport (subrect of screen to draw on)
    glViewport(0, 0, w, h);

    mCamera->setViewportSize(w, h);         // !!!!111!!!@22(*#*&@!!
}

void Game::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // !!!!!!111!!1!!!11!^&#(!@^(!!!!!!

    glm::mat4 projMatrix = mCamera->getProjectionMatrix();
    glm::mat4 viewMatrix = mCamera->getViewMatrix();

    // send projection matrix to ALL programs
    for (unsigned i = 0; i < mPrograms.size(); i++) {
        glUseProgram(mPrograms[i]);
        glsh::SetShaderUniform("u_ProjectionMatrix", projMatrix);
    }

    if (mShowAxes) {
        // draw ground plane
        glUseProgram(mUColorProgram);
        glsh::SetShaderUniform("u_ModelViewMatrix", viewMatrix);
        glsh::SetShaderUniform("u_Color", glm::vec4(0.54f, 0.8f, 0.9f, 1.0f));
        mPlane->draw();

        // draw world axes
        glUseProgram(mVColorProgram);
        glDisable(GL_DEPTH_TEST);       // temporarily disable depth test
        glsh::SetShaderUniform("u_ModelViewMatrix", viewMatrix);
        mWorldAxes->draw();
        glEnable(GL_DEPTH_TEST);        // restore depth test
    }



    //
    // draw the active mesh
    //

    glsh::Mesh* mesh = NULL;
    if (mMeshIndex < mMeshes.size()) {
        mesh = mMeshes[mMeshIndex];
    }

    if (mesh) {
        glUseProgram(mUColorDirLightProgram);

        // set lighting parameters for the directional light shader
        glm::vec3 lightDir(1.5f, 2.0f, 3.0f);           // direction to light in world space
        lightDir = glm::mat3(viewMatrix) * lightDir;    // direction to light in camera space
        lightDir = glm::normalize(lightDir);            // normalized for sanity
        glsh::SetShaderUniform("u_LightDir", lightDir);
        glsh::SetShaderUniform("u_LightColor", glm::vec3(1.0f, 1.0f, 1.0f));

        // set transform and material properties
        glm::mat4 MV = viewMatrix * mMeshRotMatrix;
        glsh::SetShaderUniform("u_ModelViewMatrix", MV);
        glsh::SetShaderUniform("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(MV))));

        // set material properties
        glsh::SetShaderUniform("u_Color", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

        // issue drawing call
        mesh->draw();
    }

    GLSH_CHECK_GL_ERRORS("drawing");
}


void Game::update(float dt)
{
    const glsh::Keyboard* kb = getKeyboard();

    if (kb->keyPressed(glsh::KC_ESCAPE)) {
        quit();  // request to exit
        return;
    }

    if (kb->keyPressed(glsh::KC_O)) {
        mCamera->toggleOrthographic();
    }

    if (kb->keyPressed(glsh::KC_V)) {
        mShowAxes ^= true;
    }

    const float rotSpeed = glsh::PI;

    //
    // Pitch and yaw in local space.
    // Hold CTRL to pitch and yaw in world space.
    //

    float yaw = 0;
    float pitch = 0;
    if (kb->isKeyDown(glsh::KC_LEFT)) {
        yaw -= dt * rotSpeed;
    }
    if (kb->isKeyDown(glsh::KC_RIGHT)) {
        yaw += dt * rotSpeed;
    }
    if (kb->isKeyDown(glsh::KC_UP)) {
        pitch += dt * rotSpeed;
    }
    if (kb->isKeyDown(glsh::KC_DOWN)) {
        pitch -= dt * rotSpeed;
    }

    // rotate the mesh
    if (kb->isKeyDown(glsh::KC_CTRL)) {
        // apply rotations about the world axes
        glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::quat yawQuat = glsh::CreateQuaternion(yaw, yAxis);
        glm::quat pitchQuat = glsh::CreateQuaternion(pitch, xAxis);
        glm::quat Q = pitchQuat * yawQuat;
        mMeshRotMatrix = glm::toMat4(Q) * mMeshRotMatrix;
    }
    else {
        // apply rotations about the model's local axes
        glm::vec3 xAxis = glm::vec3(mMeshRotMatrix[0]);
        glm::vec3 yAxis = glm::vec3(mMeshRotMatrix[1]);
        glm::quat yawQuat = glsh::CreateQuaternion(yaw, yAxis);
        glm::quat pitchQuat = glsh::CreateQuaternion(pitch, yawQuat * xAxis);
        glm::quat Q = pitchQuat * yawQuat;
        mMeshRotMatrix = glm::toMat4(Q) * mMeshRotMatrix;
    }

    // reset mesh orientation
    if (kb->keyPressed(glsh::KC_R)) {
        mMeshRotMatrix = glm::mat4(1.0f);
    }

    // cycle through the meshes
    // cycle through the meshes
    if (kb->keyPressed(glsh::KC_X)) {
        if (mMeshIndex < mMeshes.size() - 1) {
            ++mMeshIndex;
        }
        else {
            mMeshIndex = 0;
        }
    }
    if (kb->keyPressed(glsh::KC_Z)) {
        if (mMeshIndex > 0) {
            --mMeshIndex;
        }
        else {
            mMeshIndex = mMeshes.size() - 1;
        }
    }

    mCamera->update(dt);
}
