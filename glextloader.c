static PFNGLCREATESHADERPROC glCreateShader = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
static PFNGLATTACHSHADERPROC glAttachShader = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
static PFNGLDELETESHADERPROC glDeleteShader = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram = NULL;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
static PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
static PFNGLUNIFORM2FPROC glUniform2f = NULL;
static PFNGLUNIFORM2FVPROC glUniform2fv = NULL;
static PFNGLGENBUFFERSPROC glGenBuffers = NULL;
static PFNGLBINDBUFFERPROC glBindBuffer = NULL;
static PFNGLBUFFERDATAPROC glBufferData = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
static PFNGLUNIFORM1IPROC glUniform1i = NULL;
static PFNGLUNIFORM1FPROC glUniform1f = NULL;
static PFNGLUNIFORM3FPROC glUniform3f = NULL;
static PFNGLUNIFORM4FPROC glUniform4f = NULL;
static PFNGLUNIFORM3FVPROC glUniform3fv = NULL;
static PFNGLUNIFORM4FVPROC glUniform4fv = NULL;
static PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;
static PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = NULL;
static PFNGLCLEARBUFFERFVPROC glClearBufferfv = NULL;
static PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv = NULL;
static PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;
static PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv = NULL;
static PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = NULL;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;

// NOTE: All this does is find the location of the gl calls in memory.
// If you wanted to add another gl function, you can follow the pattern you
// see here:
//
// 1. make a new static variable above, with the name of the function you want to define
//	e.g. static PFNGL{NAME_OF_FUNCTION}PROC {NAME_OF_FUNCTION} = NULL;
// 2. assign the memory address to the function variable you created in step 1
//	e.g.  {NAME_OF_FUNCTION} = (PFNGL{NAME_OF_FUNCTION}PROC) glfwGetProcAddress("{NAME_OF_FUNCTION}");
//
// Hopefully that makese sense. If not, just look at the pattern above/below and copy it for 
// the function you want to.
//
// Additionally, there are libraries out there that do this for you, for instance, I think GLUT will
// do all of this for you. I did this because the only libarary I wanted to link against was GLFW for
// cross platform Window creation and mouse/keyboard input.
static void load_gl_extensions(void)
{
  glCreateShader            = (PFNGLCREATESHADERPROC) glfwGetProcAddress("glCreateShader");
  glShaderSource            = (PFNGLSHADERSOURCEPROC) glfwGetProcAddress("glShaderSource");
  glCompileShader           = (PFNGLCOMPILESHADERPROC) glfwGetProcAddress("glCompileShader");
  glGetShaderiv             = (PFNGLGETSHADERIVPROC) glfwGetProcAddress("glGetShaderiv");
  glGetShaderInfoLog        = (PFNGLGETSHADERINFOLOGPROC) glfwGetProcAddress("glGetShaderInfoLog");
  glAttachShader            = (PFNGLATTACHSHADERPROC) glfwGetProcAddress("glAttachShader");
  glCreateProgram           = (PFNGLCREATEPROGRAMPROC) glfwGetProcAddress("glCreateProgram");
  glLinkProgram             = (PFNGLLINKPROGRAMPROC) glfwGetProcAddress("glLinkProgram");
  glGetProgramiv            = (PFNGLGETPROGRAMIVPROC) glfwGetProcAddress("glGetProgramiv");
  glGetProgramInfoLog       = (PFNGLGETPROGRAMINFOLOGPROC) glfwGetProcAddress("glGetProgramInfoLog");
  glDeleteShader            = (PFNGLDELETESHADERPROC) glfwGetProcAddress("glDeleteShader");
  glUseProgram              = (PFNGLUSEPROGRAMPROC) glfwGetProcAddress("glUseProgram");
  glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC) glfwGetProcAddress("glGenVertexArrays");
  glBindVertexArray         = (PFNGLBINDVERTEXARRAYPROC) glfwGetProcAddress("glBindVertexArray");
  glDeleteProgram           = (PFNGLDELETEPROGRAMPROC) glfwGetProcAddress("glDeleteProgram");
  glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC) glfwGetProcAddress("glGetUniformLocation");
  glUniform2fv               = (PFNGLUNIFORM2FVPROC) glfwGetProcAddress("glUniform2fv");
  glUniform2f               = (PFNGLUNIFORM2FPROC) glfwGetProcAddress("glUniform2f");
  glUniform3f               = (PFNGLUNIFORM3FPROC) glfwGetProcAddress("glUniform3f");
  glUniform3fv               = (PFNGLUNIFORM3FVPROC) glfwGetProcAddress("glUniform3fv");
  glUniform4f               = (PFNGLUNIFORM4FPROC) glfwGetProcAddress("glUniform4f");
  glUniform4fv               = (PFNGLUNIFORM4FVPROC) glfwGetProcAddress("glUniform4fv");
  glGenBuffers              = (PFNGLGENBUFFERSPROC) glfwGetProcAddress("glGenBuffers");
  glBindBuffer              = (PFNGLBINDBUFFERPROC) glfwGetProcAddress("glBindBuffer");
  glBufferData              = (PFNGLBUFFERDATAPROC) glfwGetProcAddress("glBufferData");
  glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) glfwGetProcAddress("glEnableVertexAttribArray");
  glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC) glfwGetProcAddress("glVertexAttribPointer");
  glUniform1i               = (PFNGLUNIFORM1IPROC) glfwGetProcAddress("glUniform1i");
  glUniform1f               = (PFNGLUNIFORM1FPROC) glfwGetProcAddress("glUniform1f");
  glBufferSubData           = (PFNGLBUFFERSUBDATAPROC) glfwGetProcAddress("glBufferSubData");
  glClearBufferfv           = (PFNGLCLEARBUFFERFVPROC) glfwGetProcAddress("glClearBufferfv");
  glVertexAttrib4fv         = (PFNGLVERTEXATTRIB4FVPROC) glfwGetProcAddress("glVertexAttrib4fv");
  glGenerateMipmap          = (PFNGLGENERATEMIPMAPPROC) glfwGetProcAddress("glGenerateMipmap");
  glUniformMatrix2fv        = (PFNGLUNIFORMMATRIX2FVPROC) glfwGetProcAddress("glUniformMatrix2fv");
  glUniformMatrix3fv        = (PFNGLUNIFORMMATRIX3FVPROC) glfwGetProcAddress("glUniformMatrix3fv");
  glUniformMatrix4fv        = (PFNGLUNIFORMMATRIX4FVPROC) glfwGetProcAddress("glUniformMatrix4fv");

  if (glfwExtensionSupported("GL_ARB_debug_output")) {
    fprintf(stderr, "INFO: ARB_debug_output is supported\n");
    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) glfwGetProcAddress("glDebugMessageCallback");
  } else {
    fprintf(stderr, "WARN: ARB_debug_output is NOT supported\n");
  }

  if (glfwExtensionSupported("GL_EXT_draw_instanced")) {
    fprintf(stderr, "INFO: EXT_draw_instanced is supported\n");
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) glfwGetProcAddress("glDrawArraysInstanced");
  } else {
    fprintf(stderr, "WARN: EXT_draw_instanced is NOT supported\n");
  }
}
