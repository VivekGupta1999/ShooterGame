#ifndef GAME_H_
#define GAME_H_

#include "GLSH.h"

#include <vector>

class Game : public glsh::App {

    GLuint                  mUColorProgram;
    GLuint                  mUColorDirLightProgram;
    GLuint                  mVColorProgram;

    std::vector<GLuint>     mPrograms;

    glsh::Mesh* mPlane;
    glsh::Mesh* mWorldAxes;

    std::vector<glsh::Mesh*> mMeshes;       // list of viewable meshes
    unsigned                 mMeshIndex;    // index of the currently displayed mesh

    glm::mat4               mMeshRotMatrix;    // transform of the currently displayed mesh

    bool                    mShowAxes;

    glsh::FreeLookCamera* mCamera;

    static std::vector<std::string> LoadAssetList(const std::string& fname);

public:
    Game();
    ~Game();

    bool                    initialize(int w, int h)    override;
    void                    shutdown()                  override;
    void                    resize(int w, int h)        override;
    void                    draw()                      override;
    void                    update(float dt)            override;
};

#endif
