#include "glextloader.c"
#include "gl_compile_errors.h"

unsigned int indices[] = {
	0, 1, 3,  // first Triangle
	1, 2, 3,   // second Triangle
};

GLuint compile_shaders(void);
GLuint vertex_array_object;
GLuint vertex_buffer_object;
GLuint element_buffer_object;

unsigned int VAO, VBO;

const char *atlas_vert_shader_path = "shaders/atlas.vert";
const char *atlas_frag_shader_path = "shaders/atlas.frag";

const char *quad_vert_shader_path = "shaders/quad.vert";
const char *quad_frag_shader_path = "shaders/quad.frag";

const char *image_vert_shader_path = "shaders/image.vert";
const char *image_frag_shader_path = "shaders/image.frag";

GLuint atlas_program;
GLuint atlas_vertex_shader;
GLuint atlas_frag_shader;

GLuint quad_program;
GLuint quad_vertex_shader;
GLuint quad_frag_shader;

GLuint image_vertex_shader;
GLuint image_frag_shader;
GLuint image_program;

	void 
initGraphics() 
{
	loadGlExtensions();

	const char *atlas_vert_shader_source = read_file(atlas_vert_shader_path);
	const char *atlas_frag_shader_source = read_file(atlas_frag_shader_path);

	const char *quad_vert_shader_source = read_file(quad_vert_shader_path);
	const char *quad_frag_shader_source = read_file(quad_frag_shader_path);

	const char *image_vert_shader_source = read_file(image_vert_shader_path);
	const char *image_frag_shader_source = read_file(image_frag_shader_path);

	// VERT 
	atlas_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(atlas_vertex_shader, 1, &atlas_vert_shader_source, NULL);
	glCompileShader(atlas_vertex_shader);
	checkCompileErrors(atlas_vertex_shader, "ATLASVERTEX");

	quad_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(quad_vertex_shader, 1, &quad_vert_shader_source, NULL);
	glCompileShader(quad_vertex_shader);
	checkCompileErrors(quad_vertex_shader, "QUADVERT");

	image_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(image_vertex_shader, 1, &image_vert_shader_source, NULL);
	glCompileShader(image_vertex_shader);
	checkCompileErrors(image_vertex_shader, "IMAGEVERT");

	// FRAG
	atlas_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(atlas_frag_shader, 1, &atlas_frag_shader_source, NULL);
	glCompileShader(atlas_frag_shader);
	checkCompileErrors(atlas_frag_shader, "atlas_FRAG");

	quad_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(quad_frag_shader, 1, &quad_frag_shader_source, NULL);
	glCompileShader(quad_frag_shader);
	checkCompileErrors(quad_frag_shader, "QUADFRAG");

	image_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(image_frag_shader, 1, &image_frag_shader_source, NULL);
	glCompileShader(image_frag_shader);
	checkCompileErrors(image_frag_shader, "IMAGEFRAG");

	// create program
	atlas_program = glCreateProgram();
	glAttachShader(atlas_program, atlas_vertex_shader);
	glAttachShader(atlas_program, atlas_frag_shader);
	glLinkProgram(atlas_program);

	quad_program = glCreateProgram();
	glAttachShader(quad_program, quad_vertex_shader);
	glAttachShader(quad_program, quad_frag_shader);
	glLinkProgram(quad_program);

	image_program = glCreateProgram();
	glAttachShader(image_program, image_vertex_shader);
	glAttachShader(image_program, image_frag_shader);
	glLinkProgram(image_program);

	// delete shaders because they are part of the program now
	glDeleteShader(atlas_vertex_shader);
	glDeleteShader(atlas_frag_shader);
	glDeleteShader(quad_vertex_shader);
	glDeleteShader(quad_frag_shader);
	glDeleteShader(image_vertex_shader);
	glDeleteShader(image_frag_shader);

	// TODO (spangler): figure out a better place for this
	// configure VAO/VBO for texture quads
	// -----------------------------------
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(F32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(F32), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	F32 vertices[] = {};

	glGenVertexArrays(1, &vertex_array_object);
	glGenBuffers(1, &vertex_buffer_object);
	glGenBuffers(1, &element_buffer_object);
	glBindVertexArray(vertex_array_object);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(F32), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void 
setPerspective2D(U64 screen_width, U64 screen_height) 
{
	F32 pj[4][4] = {{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f},  
		{0.0f, 0.0f, 0.0f, 0.0f},                    \
		{0.0f, 0.0f, 0.0f, 0.0f}};

	F32 rl, tb, fn;
	F32 left = 0.0f;
	F32 right = (F32)screen_width;
	F32 bottom = 0.0f;
	F32 top = (F32)screen_height;
	F32 nearZ = -1.0f;
	F32 farZ = 1.0f;

	rl = 1.0f / (right  - left);
	tb = 1.0f / (top    - bottom);
	fn =-1.0f / (farZ - nearZ);

	pj[0][0] = 2.0f * rl;
	pj[1][1] = 2.0f * tb;
	pj[2][2] = 2.0f * fn;
	pj[3][0] =-(right  + left)    * rl;
	pj[3][1] =-(top    + bottom)  * tb;
	pj[3][2] = (farZ + nearZ) * fn;
	pj[3][3] = 1.0f;
	// ORTHO END

	glUseProgram(atlas_program);
	glUniformMatrix4fv(glGetUniformLocation(atlas_program, "projection"), 1, GL_FALSE, (F32 *)pj);

	glUseProgram(quad_program);
	glUniformMatrix4fv(glGetUniformLocation(quad_program, "projection"), 1, GL_FALSE, (F32 *)pj);

	glUseProgram(image_program);
	glUniformMatrix4fv(glGetUniformLocation(image_program, "projection"), 1, GL_FALSE, (F32 *)pj);
}

void
graphics_cleanup()
{
	glDeleteVertexArrays(1, &vertex_array_object);
	glDeleteProgram(atlas_program);
	glDeleteProgram(quad_program);
	glfwTerminate();
}
