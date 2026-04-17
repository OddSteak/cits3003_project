#include "OpenGL.h"

#include <iostream>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

void OpenGL::load_functions() {
    int status = gladLoadGL((GLADloadfunc) glfwGetProcAddress);
    if (!status) {
        std::cerr << "Failed to Load OpenGL functions, via GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Print some info about the OpenGL context
    std::cout << "" << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "" << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "" << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "" << "OpenGL Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "" << "------------------------------------------------------------" << std::endl;
    std::cout << "" << std::endl;
}

#ifndef __APPLE__

void GLAPIENTRY message_callback(GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

#endif

void OpenGL::setup_debug_callback() {
#ifdef __APPLE__
    std::cerr << "Insufficient OpenGL version to use debug callback." << std::endl;
    std::cerr << "\tThe minimum required version is 4.3" << std::endl;
    std::cerr << "\tHowever Apple has decided to deprecate OpenGL, so the maximum supported version by MacOS is 4.1" << std::endl;
    std::cerr << std::endl;
    std::cerr << "To do error checking, place calls to GL_CHECK_ERRORS() in key places." << std::endl;
#else
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, nullptr);
#endif
}

void OpenGL::check_errors(const char* file, int line) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char* error_name;
        switch (err) {
            case GL_INVALID_ENUM: {
                error_name = "GL_INVALID_ENUM";
                break;
            }
            case GL_INVALID_VALUE: {
                error_name = "GL_INVALID_VALUE";
                break;
            }
            case GL_INVALID_OPERATION: {
                error_name = "GL_INVALID_OPERATION";
                break;
            }
            case GL_STACK_OVERFLOW: {
                error_name = "GL_STACK_OVERFLOW";
                break;
            }
            case GL_STACK_UNDERFLOW: {
                error_name = "GL_STACK_UNDERFLOW";
                break;
            }
            case GL_OUT_OF_MEMORY: {
                error_name = "GL_OUT_OF_MEMORY";
                break;
            }
            case GL_INVALID_FRAMEBUFFER_OPERATION: {
                error_name = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            }
            case GL_CONTEXT_LOST: {
                error_name = "GL_CONTEXT_LOST";
                break;
            }
            default: {
                error_name = "[UNKNOWN]";
                break;
            }
        }

        std::cerr << "OpenGL ERROR: `" << error_name << "` (0x" << std::hex << err << std::dec << ") at: " << file << ":" << line << std::endl;
    }
}
