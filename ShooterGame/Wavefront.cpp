#include "Wavefront.h"

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <limits>

// OBJ vertex format flags
enum {
    OBJ_VFF_POSITION = 1,
    OBJ_VFF_NORMAL = 2,
    OBJ_VFF_TEXCOORD = 4
};

struct OBJVertex {
    int v, vn, vt;
    OBJVertex();
    OBJVertex(const std::string& str);
    int getFormat() const;
};

struct OBJTriangle {
    OBJVertex verts[3];
    OBJTriangle();
    OBJTriangle(const OBJVertex& a, const OBJVertex& b, const OBJVertex& c);
};

struct IndexTriangle {
    unsigned index[3];
    IndexTriangle();
};

typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;
typedef glm::vec2 TexCoord;

class OBJMesh {

public:
    // vertex and index buffer ids
    GLuint mVAO;
    GLuint mVBO;
    GLuint mIBO;

    // number of components in each vertex attribute
    // (needed by glEnableVertexArray and glVertexAttribPointer)
    GLint mPositionSize;
    GLint mNormalSize;
    GLint mTangentSize;
    GLint mTexCoordSize;

    // vertex attribute offsets in buffer
    // (needed by glVertexAttribPointer)
    GLvoid* mPositionOffset;
    GLvoid* mNormalOffset;
    GLvoid* mTangentlOffset;
    GLvoid* mTexCoordOffset;

    // vertex size in bytes
    // (needed by glVertexAttribPointer)
    GLsizei mStride;

    // number of vertices
    GLsizei mNumVertices;

    // number of indices
    // (needed by glDrawElements)
    GLsizei mNumIndices;

    // zerofy all variables
    void clear();

    // Reindex positions
    static void Reindex(std::vector<Vec3>& positions,
        const std::vector<OBJTriangle>& faces,
        std::vector<IndexTriangle>& newFaces);

    // Reindex positions and normals
    static void Reindex(std::vector<Vec3>& positions,
        std::vector<Vec3>& normals,
        const std::vector<OBJTriangle>& faces,
        std::vector<IndexTriangle>& newFaces);

    // Reindex positions and texcoords
    static void Reindex(std::vector<Vec3>& positions,
        std::vector<TexCoord>& texcoords,
        const std::vector<OBJTriangle>& faces,
        std::vector<IndexTriangle>& newFaces);

    // Reindex positions, normals, and texcoords
    static void Reindex(std::vector<Vec3>& positions,
        std::vector<Vec3>& normals,
        std::vector<TexCoord>& texcoords,
        const std::vector<OBJTriangle>& faces,
        std::vector<IndexTriangle>& newFaces);

    // compute tangents for normal mapping
    static void ComputeTangents(const std::vector<Vec3>& positions,
        const std::vector<Vec3>& normals,
        const std::vector<TexCoord>& texcoords,
        const std::vector<IndexTriangle>& triangles,
        std::vector<Vec4>& tangents);

public:

    OBJMesh();
    OBJMesh(const std::string& path, bool shouldComputeTangents = false);
    ~OBJMesh();

    bool isLoaded() const;

    bool load(const std::string& path, bool shouldComputeTangents = false);
};


inline OBJVertex::OBJVertex()
    : v(-1), vn(-1), vt(-1)
{
}

OBJVertex::OBJVertex(const std::string& str)
    : v(-1), vn(-1), vt(-1)
{
    //std::cout << str << " -->";

    std::string::size_type p = str.find_first_of('/', 0);
    if (p != std::string::npos) {

        if (p > 0) {
            std::string vstr = str.substr(0, p);
            v = glsh::FromString<int>(vstr);
            //std::cout << " v = " << vstr << " (" << v << ")";
        }

        std::string::size_type q = str.find_first_of('/', p + 1);
        if (q != std::string::npos) {
            // have two slashes (v/vt/vn)

            if (q > p + 1) {
                std::string vtstr = str.substr(p + 1, q - p - 1);
                vt = glsh::FromString<int>(vtstr);
                //std::cout << " vt = " << vtstr << " (" << vt << ")";
            }

            if (q < str.size() - 1) {
                std::string vnstr = str.substr(q + 1, str.size() - q - 1);
                vn = glsh::FromString<int>(vnstr);
                //std::cout << " vn = " << vnstr << " (" << vn << ")";
            }

        }
        else {
            // have one slash (v/vt)
            if (p < str.size() - 1) {
                std::string vtstr = str.substr(p + 1, str.size() - p - 1);
                vt = glsh::FromString<int>(vtstr);
                //std::cout << " vt = " << vtstr << " (" << vt << ")";
            }
        }

    }
    else {
        // no slash found, have a single token only
        v = glsh::FromString<int>(str);
        //std::cout << " v = " << str << " (" << v << ")";
    }

    //std::cout << std::endl;
}

inline int OBJVertex::getFormat() const
{
    int fmt = 0;

    if (v > 0)
        fmt |= OBJ_VFF_POSITION;
    if (vn > 0)
        fmt |= OBJ_VFF_NORMAL;
    if (vt > 0)
        fmt |= OBJ_VFF_TEXCOORD;

    return fmt;
}

inline OBJTriangle::OBJTriangle()
{
}

inline OBJTriangle::OBJTriangle(const OBJVertex& a, const OBJVertex& b, const OBJVertex& c)
{
    verts[0] = a;
    verts[1] = b;
    verts[2] = c;
}

inline IndexTriangle::IndexTriangle()
{
    index[0] = index[1] = index[2] = -1;
}


std::vector<OBJTriangle> Tesselate(const std::vector<OBJVertex>& v)
{
    std::vector<OBJTriangle> tris;

    if (v.size() < 3)
        return tris;

    tris.resize(v.size() - 2);

    for (unsigned i = 2; i < v.size(); i++) {
        tris[i - 2].verts[0] = v[0];
        tris[i - 2].verts[1] = v[i - 1];
        tris[i - 2].verts[2] = v[i];
    }

    return tris;
}



OBJMesh::OBJMesh()
    : mVAO(0)
    , mVBO(0)
    , mIBO(0)
{
    memset(this, 0, sizeof(*this));
}

OBJMesh::OBJMesh(const std::string& path, bool shouldComputeTangents)
    : mVAO(0)
    , mVBO(0)
    , mIBO(0)
{
    memset(this, 0, sizeof(*this));

    load(path, shouldComputeTangents);
}

OBJMesh::~OBJMesh()
{
}


//
//
// Load
//
//


bool OBJMesh::load(const std::string& path, bool shouldComputeTangents)
{
    std::cout << "Loading '" << path << "'" << std::endl;

    std::ifstream file(path.c_str());

    if (!file) {
        std::cerr << "ERROR: Failed to open " << path << std::endl;
        return false;
    }

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<TexCoord> texcoords;
    std::vector<OBJTriangle> faces;

    std::string line;
    int lineno = 0;

    int vertexFormat = 0;

    unsigned numFaces = 0;

    float xmin = std::numeric_limits<float>::infinity();
    float xmax = -std::numeric_limits<float>::infinity();
    float ymin = std::numeric_limits<float>::infinity();
    float ymax = -std::numeric_limits<float>::infinity();
    float zmin = std::numeric_limits<float>::infinity();
    float zmax = -std::numeric_limits<float>::infinity();

    for (;;) {

        std::getline(file, line);

        if (!file) {
            if (!file.eof()) {
                std::cerr << "ERROR: Failed to read from " << path << std::endl;
                return false;
            }
            break;
        }

        ++lineno;

        std::vector<std::string> tokens = glsh::Tokenize(line);

        // make sure it's not an empty line
        if (tokens.size() > 0) {

            // check if it's a comment
            char firstChar = tokens[0][0];
            if (firstChar == '#')
                continue;  // move on to next line

            if (tokens[0] == "v") {
                if (tokens.size() < 4) {
                    std::cerr << "ERROR: Incorrect number of vertex position components on line " << lineno << std::endl;
                    return false;
                }

                GLfloat x = glsh::FromString<GLfloat>(tokens[1]);
                GLfloat y = glsh::FromString<GLfloat>(tokens[2]);
                GLfloat z = glsh::FromString<GLfloat>(tokens[3]);

                if (x < xmin) xmin = x;
                if (x > xmax) xmax = x;
                if (y < ymin) ymin = y;
                if (y > ymax) ymax = y;
                if (z < zmin) zmin = z;
                if (z > zmax) zmax = z;

                positions.push_back(Vec3(x, y, z));

            }
            else if (tokens[0] == "vn") {
                if (tokens.size() < 4) {
                    std::cerr << "ERROR: Incorrect number of vertex normal components on line " << lineno << std::endl;
                    return false;
                }

                GLfloat nx = glsh::FromString<GLfloat>(tokens[1]);
                GLfloat ny = glsh::FromString<GLfloat>(tokens[2]);
                GLfloat nz = glsh::FromString<GLfloat>(tokens[3]);

                normals.push_back(Vec3(nx, ny, nz));

            }
            else if (tokens[0] == "vt") {
                if (tokens.size() < 3) {
                    std::cerr << "ERROR: Incorrect number of texture coordinates on line " << lineno << std::endl;
                    return false;
                }

                GLfloat u = glsh::FromString<GLfloat>(tokens[1]);
                GLfloat v = glsh::FromString<GLfloat>(tokens[2]);

                texcoords.push_back(TexCoord(u, v));

            }
            else if (tokens[0] == "f") {

                // need at least 3 vertices per face
                if (tokens.size() < 4) {
                    std::cerr << "ERROR: Incorrect number of face elements" << lineno << std::endl;
                    return NULL;
                }

                std::vector<OBJVertex> verts(tokens.size() - 1);

                for (unsigned i = 1; i < tokens.size(); i++) {
                    verts[i - 1] = OBJVertex(tokens[i]);
                    // deal with negative indices
                    if (verts[i - 1].v < 0)
                        verts[i - 1].v = positions.size() + verts[i - 1].v + 1;
                    if (verts[i - 1].vn < 0)
                        verts[i - 1].vn = normals.size() + verts[i - 1].vn + 1;
                    if (verts[i - 1].vt < 0)
                        verts[i - 1].vt = texcoords.size() + verts[i - 1].vt + 1;
                }

                // format checking
                if (!vertexFormat) {
                    // this is the very first face
                    vertexFormat = verts[0].getFormat();
                    // make sure at least position is included
                    if ((vertexFormat & OBJ_VFF_POSITION) != OBJ_VFF_POSITION) {
                        std::cerr << "Invalid vertex format!" << std::endl;
                        return false;
                    }
                }
                // make sure all vertices have the same format as the very first one
                for (unsigned i = 0; i < verts.size(); i++) {
                    if (verts[i].getFormat() != vertexFormat) {
                        std::cerr << "Inconsistent vertex format!" << std::endl;
                        return false;
                    }
                }

                // triangulate this face
                const std::vector<OBJTriangle>& tris = Tesselate(verts);

                //std::cout << verts.size() << " verts -> " << tris.size() << " tris" << std::endl;

                for (unsigned i = 0; i < tris.size(); i++)
                    faces.push_back(tris[i]);

                ++numFaces;

#if 0
                // only allow 3 vertices per face (triangles only!)
                if (tokens.size() != 4) {
                    std::cerr << "ERROR: Incorrect number of face elements" << lineno << std::endl;
                    return false;
                }

                OBJTriangle tri(tokens[1], tokens[2], tokens[3]);

                // format checking
                if (!vertexFormat) {
                    // this is the first face
                    vertexFormat = tri.verts[0].getFormat();
                    // make sure at least position is included
                    if ((vertexFormat & OBJ_VFF_POSITION) != OBJ_VFF_POSITION) {
                        std::cerr << "Invalid vertex format!" << std::endl;
                        return false;
                    }
                }

                // make sure all vertices have the same format as the very first one
                for (int i = 0; i < 3; i++)
                    if (tri.verts[i].getFormat() != vertexFormat) {
                        std::cerr << "Inconsistent vertex format!" << std::endl;
                        return false;
                    }

                faces.push_back(tri);
#endif
            }
        }
    }

    std::cout << "  Loaded " << positions.size() << " positions" << std::endl;
    std::cout << "  Loaded " << normals.size() << " normals" << std::endl;
    std::cout << "  Loaded " << texcoords.size() << " texture coordinates" << std::endl;
    std::cout << "  Loaded " << numFaces << " faces (" << faces.size() << " triangles)" << std::endl;

    bool haveNormals = (vertexFormat & OBJ_VFF_NORMAL) == OBJ_VFF_NORMAL;
    if (!haveNormals && normals.size() == positions.size()) {
        // normals were not specified with vertex format in faces,
        // but the number of normals given matches the number of positions,
        // so assume a 1:1 correspondance between normals and positions
        for (unsigned i = 0; i < faces.size(); i++)
            for (int j = 0; j < 3; j++)
                faces[i].verts[j].vn = faces[i].verts[j].v;
        haveNormals = true;
    }

    bool haveTexCoords = (vertexFormat & OBJ_VFF_TEXCOORD) == OBJ_VFF_TEXCOORD;
    if (!haveTexCoords && texcoords.size() == positions.size()) {
        // texcoords were not specified with vertex format in faces,
        // but the number of texcoords given matches the number of positions,
        // so assume a 1:1 correspondance between texcoords and positions
        for (unsigned i = 0; i < faces.size(); i++)
            for (int j = 0; j < 3; j++)
                faces[i].verts[j].vt = faces[i].verts[j].v;
        haveTexCoords = true;
    }

    int floatsPerVertex = 0;

    mPositionSize = 3;
    mPositionOffset = (void*)(floatsPerVertex * sizeof(GLfloat));
    floatsPerVertex += mPositionSize;

    if (haveNormals) {
        mNormalSize = 3;
        mNormalOffset = (void*)(floatsPerVertex * sizeof(GLfloat));
        floatsPerVertex += mNormalSize;
    }

    if (haveTexCoords) {
        mTexCoordSize = 2;
        mTexCoordOffset = (void*)(floatsPerVertex * sizeof(GLfloat));
        floatsPerVertex += mTexCoordSize;
    }

    if (shouldComputeTangents) {
        if (haveNormals && haveTexCoords) {
            std::cout << "  Tangents will be computed" << std::endl;
            mTangentSize = 4;
            mTangentlOffset = (void*)(floatsPerVertex * sizeof(GLfloat));
            floatsPerVertex += mTangentSize;
        }
        else {
            std::cout << "  Warning: Tangents will not be computed because normals and/or texture coordinates are missing" << std::endl;
            shouldComputeTangents = false;
        }
    }
    else {
        std::cout << "  Tangents will not be computed" << std::endl;
    }

    // vertex size in bytes
    mStride = floatsPerVertex * sizeof(GLfloat);

    //
    // Reindex
    //

    std::vector<IndexTriangle> newFaces;

    if (haveNormals && haveTexCoords) {
        Reindex(positions, normals, texcoords, faces, newFaces);
    }
    else if (haveNormals && !haveTexCoords) {
        Reindex(positions, normals, faces, newFaces);
    }
    else if (!haveNormals && haveTexCoords) {
        Reindex(positions, texcoords, faces, newFaces);
    }
    else {
        Reindex(positions, faces, newFaces);
    }

    // compute tangents, if needed
    std::vector<Vec4> tangents;
    if (shouldComputeTangents)
        ComputeTangents(positions, normals, texcoords, newFaces, tangents);

    mNumVertices = positions.size();
    mNumIndices = 3 * newFaces.size();

    std::cout << "  Found " << mNumVertices << " unique vertices" << std::endl;
    std::cout << "  Using " << mNumIndices << " indices" << std::endl;

    unsigned indexSize = sizeof(newFaces[0].index[0]);
    unsigned vboSize = mNumVertices * mStride;
    unsigned iboSize = mNumIndices * indexSize;
    unsigned totalSize = vboSize + iboSize;

    std::cout << "  Vertex size: " << mStride << " bytes" << std::endl;
    std::cout << "  Index size:  " << indexSize << " bytes" << std::endl;
    std::cout << "  VBO size:    " << vboSize << " bytes" << std::endl;
    std::cout << "  IBO size:    " << iboSize << " bytes" << std::endl;
    std::cout << "  Total size:  " << totalSize << " bytes" << std::endl;

    unsigned naiveSize = 3 * faces.size() * mStride;
    std::cout << "  Naive size:  " << naiveSize << " bytes (without IBO)" << std::endl;

    std::cout << "  Bounding box:\n";
    std::cout << "    Width:    " << (xmax - xmin) << " [" << xmin << ", " << xmax << "]\n";
    std::cout << "    Height:   " << (ymax - ymin) << " [" << ymin << ", " << ymax << "]\n";
    std::cout << "    Depth:    " << (zmax - zmin) << " [" << zmin << ", " << zmax << "]\n";
    std::cout << std::endl;

    //
    // build the vertex buffer
    //
    std::vector<GLfloat> vertexData(mNumVertices * floatsPerVertex);
    std::vector<GLfloat>::iterator it = vertexData.begin();

    for (int i = 0; i < mNumVertices; i++) {
        // write position
        *it++ = positions[i].x;
        *it++ = positions[i].y;
        *it++ = positions[i].z;
        // write normal
        if (haveNormals) {
            *it++ = normals[i].x;
            *it++ = normals[i].y;
            *it++ = normals[i].z;
        }
        // write texcoord
        if (haveTexCoords) {
            *it++ = texcoords[i].s;
            *it++ = texcoords[i].t;
        }
        // write tangent
        if (shouldComputeTangents) {
            *it++ = tangents[i].x;
            *it++ = tangents[i].y;
            *it++ = tangents[i].z;
            *it++ = tangents[i].w;
        }
    }

    GLSH_CHECK_GL_ERRORS("poop");

    // create a vertex array object (VAO)
    glGenVertexArrays(1, &mVAO);
    if (!mVAO) {
        std::cerr << "*** Poop: Failed to create VAO" << std::endl;
        return false;
    }

    GLSH_CHECK_GL_ERRORS("poop");

    // bind the VAO (subsequent vertex attribute info will be stored in this VAO)
    glBindVertexArray(mVAO);

    // generate vertex buffer
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER,                           // the buffer to resize and fill
        vertexData.size() * sizeof(vertexData[0]), // total size in bytes
        &vertexData[0],                            // address of data in RAM
        GL_STATIC_DRAW);                           // buffer usage mode (GL_STATIC_DRAW == read-only == fast drawing)

    GLSH_CHECK_GL_ERRORS("poop");

    // describe vertex attributes
    if (mPositionSize > 0) {
        glVertexAttribPointer(glsh::VA_POSITION, mPositionSize, GL_FLOAT, GL_FALSE, mStride, mPositionOffset);
        glEnableVertexAttribArray(glsh::VA_POSITION);
    }
    if (mNormalSize > 0) {
        glVertexAttribPointer(glsh::VA_NORMAL, mNormalSize, GL_FLOAT, GL_FALSE, mStride, mNormalOffset);
        glEnableVertexAttribArray(glsh::VA_NORMAL);
    }
    if (mTexCoordSize > 0) {
        glVertexAttribPointer(glsh::VA_TEXCOORD, mTexCoordSize, GL_FLOAT, GL_FALSE, mStride, mTexCoordOffset);
        glEnableVertexAttribArray(glsh::VA_TEXCOORD);
    }
    if (mTangentSize > 0) {
        glVertexAttribPointer(glsh::VA_TANGENT, mTangentSize, GL_FLOAT, GL_FALSE, mStride, mTangentlOffset);
        glEnableVertexAttribArray(glsh::VA_TANGENT);
    }

    GLSH_CHECK_GL_ERRORS("poop");

    // generate index buffer
    glGenBuffers(1, &mIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,                   // the buffer to resize and fill
        newFaces.size() * sizeof(newFaces[0]),     // total size in bytes
        &newFaces[0],                              // address of data in RAM
        GL_STATIC_DRAW);                           // buffer usage mode (GL_STATIC_DRAW == read-only == fast drawing)

    GLSH_CHECK_GL_ERRORS("poop");

    // unbind stuff, for now
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLSH_CHECK_GL_ERRORS("poop");

    return true;
}

//
// Reindex positions
//
void OBJMesh::Reindex(std::vector<Vec3>& positions,
    const std::vector<OBJTriangle>& faces,
    std::vector<IndexTriangle>& newFaces)
{
    std::vector<Vec3> newPositions;

    newPositions.reserve(positions.size());

    newFaces.resize(faces.size());

    std::map<int, unsigned> indexTable;

    int index = 0;

    // for each face...
    for (unsigned i = 0; i < faces.size(); i++) {
        // for each vertex in the face...
        for (int j = 0; j < 3; j++) {

            int v = faces[i].verts[j].v;

            std::pair<std::map<int, unsigned>::iterator, bool> insertionResult;

            insertionResult = indexTable.insert(std::make_pair(v, index));

            if (insertionResult.second) {
                // vertex was not seen yet, new index inserted
                newPositions.push_back(positions[v - 1]);
                newFaces[i].index[j] = index++;
            }
            else {
                // vertex already seen, use existing index
                newFaces[i].index[j] = insertionResult.first->second;
            }
        }
    }

    // replace the old with the new
    positions.swap(newPositions);
}

//
// Reindex positions and normals
//
void OBJMesh::Reindex(std::vector<Vec3>& positions,
    std::vector<Vec3>& normals,
    const std::vector<OBJTriangle>& faces,
    std::vector<IndexTriangle>& newFaces)
{
    std::vector<Vec3> newPositions;
    std::vector<Vec3> newNormals;

    newPositions.reserve(positions.size());
    newNormals.reserve(normals.size());

    newFaces.resize(faces.size());

    std::map<int, std::map<int, unsigned> > indexTable;

    int index = 0;

    // for each face...
    for (unsigned i = 0; i < faces.size(); i++) {
        // for each vertex in the face...
        for (int j = 0; j < 3; j++) {

            int v = faces[i].verts[j].v;
            int vn = faces[i].verts[j].vn;

            std::pair<std::map<int, unsigned>::iterator, bool> insertionResult;

            insertionResult = indexTable[v].insert(std::make_pair(vn, index));

            if (insertionResult.second) {
                // vertex was not seen yet, new index inserted
                newPositions.push_back(positions[v - 1]);
                newNormals.push_back(normals[vn - 1]);
                newFaces[i].index[j] = index++;
            }
            else {
                // vertex already seen, use existing index
                newFaces[i].index[j] = insertionResult.first->second;
            }
        }
    }

    // replace the old with the new
    positions.swap(newPositions);
    normals.swap(newNormals);
}

//
// Reindex positions and texcoords
//
void OBJMesh::Reindex(std::vector<Vec3>& positions,
    std::vector<TexCoord>& texcoords,
    const std::vector<OBJTriangle>& faces,
    std::vector<IndexTriangle>& newFaces)
{
    std::vector<Vec3> newPositions;
    std::vector<TexCoord> newTexcoords;

    newPositions.reserve(positions.size());
    newTexcoords.reserve(texcoords.size());

    newFaces.resize(faces.size());

    std::map<int, std::map<int, unsigned> > indexTable;

    int index = 0;

    // for each face...
    for (unsigned i = 0; i < faces.size(); i++) {
        // for each vertex in the face...
        for (int j = 0; j < 3; j++) {

            int v = faces[i].verts[j].v;
            int vt = faces[i].verts[j].vt;

            std::pair<std::map<int, unsigned>::iterator, bool> insertionResult;

            insertionResult = indexTable[v].insert(std::make_pair(vt, index));

            if (insertionResult.second) {
                // vertex was not seen yet, new index inserted
                newPositions.push_back(positions[v - 1]);
                newTexcoords.push_back(texcoords[vt - 1]);
                newFaces[i].index[j] = index++;
            }
            else {
                // vertex already seen, use existing index
                newFaces[i].index[j] = insertionResult.first->second;
            }
        }
    }

    // replace the old with the new
    positions.swap(newPositions);
    texcoords.swap(newTexcoords);
}

//
// Reindex positions, normals, and texcoords
//
void OBJMesh::Reindex(std::vector<Vec3>& positions,
    std::vector<Vec3>& normals,
    std::vector<TexCoord>& texcoords,
    const std::vector<OBJTriangle>& faces,
    std::vector<IndexTriangle>& newFaces)
{
    std::vector<Vec3> newPositions;
    std::vector<Vec3> newNormals;
    std::vector<TexCoord> newTexcoords;

    newPositions.reserve(positions.size());
    newNormals.reserve(normals.size());
    newTexcoords.reserve(texcoords.size());

    newFaces.resize(faces.size());

    std::map<int, std::map<int, std::map<int, unsigned> > > indexTable;

    int index = 0;

    // for each face...
    for (unsigned i = 0; i < faces.size(); i++) {
        // for each vertex in the face...
        for (int j = 0; j < 3; j++) {

            int v = faces[i].verts[j].v;
            int vn = faces[i].verts[j].vn;
            int vt = faces[i].verts[j].vt;

            std::pair<std::map<int, unsigned>::iterator, bool> insertionResult;

            insertionResult = indexTable[v][vn].insert(std::make_pair(vt, index));

            if (insertionResult.second) {
                // vertex was not seen yet, new index inserted
                newPositions.push_back(positions[v - 1]);
                newNormals.push_back(normals[vn - 1]);
                newTexcoords.push_back(texcoords[vt - 1]);
                newFaces[i].index[j] = index++;
            }
            else {
                // vertex already seen, use existing index
                newFaces[i].index[j] = insertionResult.first->second;
            }
        }
    }

    // replace the old with the new
    positions.swap(newPositions);
    normals.swap(newNormals);
    texcoords.swap(newTexcoords);
}

//
//
// Adapted from code by Eric Lengyel (http://www.terathon.com/code/tangent.html)
//
//
void OBJMesh::ComputeTangents(const std::vector<Vec3>& positions,
    const std::vector<Vec3>& normals,
    const std::vector<TexCoord>& texcoords,
    const std::vector<IndexTriangle>& triangles,
    std::vector<Vec4>& tangents)
{
    unsigned numVertices = positions.size();
    unsigned numTriangles = triangles.size();

    // Vec3 default constructor zeroes out each element
    std::vector<Vec3> tan1(numVertices);
    std::vector<Vec3> tan2(numVertices);

    tangents.resize(numVertices);

    for (unsigned a = 0; a < numTriangles; a++) {

        unsigned i1 = triangles[a].index[0];
        unsigned i2 = triangles[a].index[1];
        unsigned i3 = triangles[a].index[2];

        const Vec3& v1 = positions[i1];
        const Vec3& v2 = positions[i2];
        const Vec3& v3 = positions[i3];

        const TexCoord& w1 = texcoords[i1];
        const TexCoord& w2 = texcoords[i2];
        const TexCoord& w3 = texcoords[i3];

        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;

        float s1 = w2.s - w1.s;
        float s2 = w3.s - w1.s;
        float t1 = w2.t - w1.t;
        float t2 = w3.t - w1.t;

        float r = 1.0f / (s1 * t2 - s2 * t1);
        Vec3 sdir((t2 * x1 - t1 * x2) * r,
            (t2 * y1 - t1 * y2) * r,
            (t2 * z1 - t1 * z2) * r);
        Vec3 tdir((s1 * x2 - s2 * x1) * r,
            (s1 * y2 - s2 * y1) * r,
            (s1 * z2 - s2 * z1) * r);

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    for (unsigned a = 0; a < numVertices; a++)
    {
        const Vec3& n = normals[a];
        const Vec3& t = tan1[a];

        // Gram-Schmidt orthogonalize
        Vec3 result = glm::normalize(t - n * glm::dot(n, t));
        tangents[a].x = result.x;
        tangents[a].y = result.y;
        tangents[a].z = result.z;

        // Calculate handedness
        tangents[a].w = (glm::dot(glm::cross(n, t), tan2[a]) < 0.0f) ? 1.0f : -1.0f;
    }
}



glsh::Mesh* LoadWavefrontOBJ(const std::string& path)
{
    OBJMesh mesh;

    if (mesh.load(path, false)) {
        return new glsh::IndexedMesh(mesh.mVBO, mesh.mIBO, mesh.mVAO, GL_TRIANGLES, GL_UNSIGNED_INT, mesh.mNumIndices);
    }

    return NULL;
}
