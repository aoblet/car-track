# include "glUtils.hpp"

////////////////////////////
///////// PROGRAM //////////
///////////////////////////

bool Program::link() {
    glLinkProgram(m_nGLId);
    GLint status;
    glGetProgramiv(m_nGLId, GL_LINK_STATUS, &status);
    return status == GL_TRUE;
}

const std::string Program::getInfoLog() const {
    GLint length;
    glGetProgramiv(m_nGLId, GL_INFO_LOG_LENGTH, &length);
    char* log = new char[length];
    glGetProgramInfoLog(m_nGLId, length, 0, log);
    std::string logString(log);
    delete [] log;
    return logString;
}

// Build a GLSL program from source code
Program buildProgram(const GLchar* vsSrc, const GLchar* fsSrc) {
    Shader vs(GL_VERTEX_SHADER);
    vs.setSource(vsSrc);

    if(!vs.compile()) {
        throw std::runtime_error("Compilation error for vertex shader: " + vs.getInfoLog());
    }

    Shader fs(GL_FRAGMENT_SHADER);
    fs.setSource(fsSrc);

    if(!fs.compile()) {
        throw std::runtime_error("Compilation error for fragment shader: " + fs.getInfoLog());
    }

    Program program;
    program.attachShader(vs);
    program.attachShader(fs);

    if(!program.link()) {
        throw std::runtime_error("Link error: " + program.getInfoLog());
    }

    return program;
}

// Load source code from files and build a GLSL program
Program loadProgram(const std::__cxx11::string &vsFile, const std::__cxx11::string &fsFile) {
    Shader vs = loadShader(GL_VERTEX_SHADER, vsFile);
    Shader fs = loadShader(GL_FRAGMENT_SHADER, fsFile);

    if(!vs.compile()) {
        throw std::runtime_error("Compilation error for vertex shader (from file " + std::string(vsFile) + "): " + vs.getInfoLog());
    }

    if(!fs.compile()) {
        throw std::runtime_error("Compilation error for fragment shader (from file " + std::string(fsFile) + "): " + fs.getInfoLog());
    }

    Program program;
    program.attachShader(vs);
    program.attachShader(fs);

    if(!program.link()) {
        throw std::runtime_error("Link error (for files " + vsFile + " and " + fsFile + "): " + program.getInfoLog());
    }

    return program;
}


//////////////////////////////
/////////// SHADER ///////////
//////////////////////////////


bool Shader::compile() {
    glCompileShader(m_nGLId);
    GLint status;
    glGetShaderiv(m_nGLId, GL_COMPILE_STATUS, &status);
    return status == GL_TRUE;
}

const std::string Shader::getInfoLog() const {
    GLint length;
    glGetShaderiv(m_nGLId, GL_INFO_LOG_LENGTH, &length);
    char* log = new char[length];
    glGetShaderInfoLog(m_nGLId, length, 0, log);
    std::string logString(log);
    delete [] log;
    return logString;
}

Shader loadShader(GLenum type, const std::string &filepath) {
    std::ifstream input(filepath.c_str());
    if(!input.is_open()) {
        throw std::runtime_error("Unable to load the file " + filepath);
    }

    std::stringstream buffer;
    buffer << input.rdbuf();

    Shader shader(type);
    shader.setSource(buffer.str().c_str());

    return shader;
}
