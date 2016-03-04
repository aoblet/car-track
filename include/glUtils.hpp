#pragma once

//glm
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/geometric.hpp>
//glew
#include <GL/glew.h>
//std
#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include <stdexcept>
#include <fstream>
#include <sstream>

GLuint createGlProgram(const std::string& vertexSource, const std::string& fragmentSource);


//////////////////////////////////////////////
/////////////////// GLOBAL ///////////////////
//////////////////////////////////////////////

static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

struct Vertex{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uvCoord;

    inline Vertex(glm::vec3 pos = glm::vec3(0,0,0), glm::vec3 col = glm::vec3(1,1,1), glm::vec2 uv = glm::vec2(0,0) ) : position(pos), color(col), uvCoord(uv)
    {

    }

};

///////////////////////////////////
///////////// SHADER //////////////
///////////////////////////////////

class Shader {
public:
    Shader(GLenum type): m_nGLId(glCreateShader(type)) {
    }

    ~Shader() {
        glDeleteShader(m_nGLId);
    }

    Shader(Shader&& rvalue): m_nGLId(rvalue.m_nGLId) {
        rvalue.m_nGLId = 0;
    }

    Shader& operator =(Shader&& rvalue) {
        m_nGLId = rvalue.m_nGLId;
        rvalue.m_nGLId = 0;
        return *this;
    }

    GLuint getGLId() const {
        return m_nGLId;
    }

    void setSource(const char* src) {
        glShaderSource(m_nGLId, 1, &src, 0);
    }

    bool compile();

    const std::string getInfoLog() const;

private:
    Shader(const Shader&);
    Shader& operator =(const Shader&);

    GLuint m_nGLId;
};

// Load a shader (but does not compile it)
Shader loadShader(GLenum type, const std::string& filepath);


////////////////////////////
///////// PROGRAM //////////
////////////////////////////
class Program {
public:
    Program(): m_nGLId(glCreateProgram()) {
    }

    ~Program() {
        glDeleteProgram(m_nGLId);
    }

    Program(Program&& rvalue): m_nGLId(rvalue.m_nGLId) {
        rvalue.m_nGLId = 0;
    }

    Program& operator =(Program&& rvalue) {
        m_nGLId = rvalue.m_nGLId;
        rvalue.m_nGLId = 0;
        return *this;
    }

    GLuint getGLId() const {
        return m_nGLId;
    }

    void attachShader(const Shader& shader) {
        glAttachShader(m_nGLId, shader.getGLId());
    }

    bool link();

    const std::string getInfoLog() const;

    void use() const {
        glUseProgram(m_nGLId);
    }

private:
    Program(const Program&);
    Program& operator =(const Program&);

    GLuint m_nGLId;
};

// Build a GLSL program from source code
Program buildProgram(const GLchar* vsSrc, const GLchar* fsSrc);

// Load source code from files and build a GLSL program
Program loadProgram(const std::string& vsFile, const std::string& fsFile);


//////////////////////////////////////////////

