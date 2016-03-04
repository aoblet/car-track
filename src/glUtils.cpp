# include "glUtils.hpp"



GLuint createGlProgram(const std::string& vertexSource, const std::string& fragmentSource)
{
    std::cout<<"vertex source = "<<vertexSource<<std::endl;
    std::cout<<"fragment source = "<<fragmentSource<<std::endl;

    //Create an empty vertex shader handle
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //Send the vertex shader source code to GL
    //Note that std::string's .c_str is NULL character terminated.
    const GLchar *source = (const GLchar *)vertexSource.c_str();
    glShaderSource(vertexShader, 1, &source, 0);

    //Compile the vertex shader
    glCompileShader(vertexShader);

    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

        //We don't need the shader anymore.
        glDeleteShader(vertexShader);

        //Use the infoLog as you see fit.

        //In this simple program, we'll just leave
        return 0;
    }

    //Create an empty fragment shader handle
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //Send the fragment shader source code to GL
    //Note that std::string's .c_str is NULL character terminated.
    source = (const GLchar *)fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &source, 0);

    //Compile the fragment shader
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

        //We don't need the shader anymore.
        glDeleteShader(fragmentShader);
        //Either of them. Don't leak shaders.
        glDeleteShader(vertexShader);

        //Use the infoLog as you see fit.

        //In this simple program, we'll just leave
        return 0;
    }

    //Vertex and fragment shaders are successfully compiled.
    //Now time to link them together into a program.
    //Get a program object.
    GLuint program = glCreateProgram();

    //Attach our shaders to our program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    //Link our program
    glLinkProgram(program);

    //Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        //We don't need the program anymore.
        glDeleteProgram(program);
        //Don't leak shaders either.
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        //Use the infoLog as you see fit.

        //In this simple program, we'll just leave
        return 0;
    }

    //Always detach shaders after a successful link.
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    return program;
}


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
Program loadProgram(const std::string &vsFile, const std::string &fsFile) {
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
