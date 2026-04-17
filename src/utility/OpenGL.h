#ifndef OPENGL_H
#define OPENGL_H

#include <glad/gl.h>

namespace OpenGL {
    /// Specify the OpenGL version to create a context for.
    /// With a switch so that Apple only uses 4.1, since that is all that is supported.
    const int VERSION_MAJOR = 4;
#ifdef __APPLE__
    const int VERSION_MINOR = 1;
#else
    const int VERSION_MINOR = 3;
#endif

    /// Load the OpenGL function pointers, requires a current OpenGL context
    void load_functions();

    /// Hook in the debug callback, or just print error to console if on APPLE
    void setup_debug_callback();

    /// A helper method to check for OpenGL errors and print them to the console.
    /// However do NOT use this directly, instead use the macro GL_CHECK_ERRORS() below,
    /// as that fills out the file, and line, parameters for you.
    void check_errors(const char* file, int line);
}

/// A helper macro to print out any OpenGL errors and also print the file and line it's called on
#define GL_CHECK_ERRORS() OpenGL::check_errors(__FILE__, __LINE__)

#endif //OPENGL_H
