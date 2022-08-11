#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "GLFW/glfw3.h"

#include "glextloader.c"
#include "file.h"
#include "gl_compile_errors.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SCREEN_WIDTH   1024
#define SCREEN_HEIGHT  786
#define NUM_GLYPHS 128
#define MAX(a,b) (((a)>(b))?(a):(b))

struct glyph_info {
	int x0, y0, x1, y1;	// coords of glyph in the texture atlas
	int x_off, y_off;   // left & top bearing when rendering
	int advance;        // x advance when rendering
} info[NUM_GLYPHS];

void render_text(const char* text, float x, float y, float scale);
GLuint compile_shaders(void);

GLuint vertex_shader;
GLuint frag_shader;
GLuint font_vertex_shader;
GLuint font_frag_shader;
GLuint texture_program;
GLuint font_program;

const char *texture_vert_shader = "shaders/texture.vert";
const char *texture_frag_shader = "shaders/texture.frag";
const char *font_vert_shader_path = "shaders/font.vert";
const char *font_frag_shader_path = "shaders/font.frag";

unsigned int VAO, VBO;

typedef struct {
	float x;
	float y;
} VEC2;

/// Holds all state information relevant to a character as loaded using FreeType
typedef struct {
	int16_t TextureID; // ID handle of the glyph texture
	VEC2 Size;      // Size of glyph
	VEC2 Bearing;   // Offset from baseline to left/top of glyph
	int16_t Advance;   // Horizontal offset to advance to next glyph
	struct FT_Bitmap_ *bitmap;
} Character;

Character Characters[128];

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ed", NULL, NULL);
	if (window == NULL)
	{
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// find memory location of the opengl functions used below
	load_gl_extensions();

	const char *vert_shader_source = read_file(texture_vert_shader);
	const char *frag_shader_source = read_file(texture_frag_shader);

	const char *font_vert_shader_source = read_file(font_vert_shader_path);
	const char *font_frag_shader_source = read_file(font_frag_shader_path);

	// compiles vert shader
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vert_shader_source, NULL);
	glCompileShader(vertex_shader);
	checkCompileErrors(vertex_shader, "VERTEX");

	font_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(font_vertex_shader, 1, &font_vert_shader_source, NULL);
	glCompileShader(font_vertex_shader);
	checkCompileErrors(font_vertex_shader, "FONT VERTEX");

	// compiles frag shader
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_shader_source, NULL);
	glCompileShader(frag_shader);
	checkCompileErrors(frag_shader, "FRAG");

	font_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(font_frag_shader, 1, &font_frag_shader_source, NULL);
	glCompileShader(font_frag_shader);
	checkCompileErrors(font_frag_shader, "FONTFRAG");

	// create program
	texture_program = glCreateProgram();
	glAttachShader(texture_program, vertex_shader);
	glAttachShader(texture_program, frag_shader);
	glLinkProgram(texture_program);

	font_program = glCreateProgram();
	glAttachShader(font_program, font_vertex_shader);
	glAttachShader(font_program, font_frag_shader);
	glLinkProgram(font_program);

	// delete shaders because they are part of the program now
	glDeleteShader(vertex_shader);
	glDeleteShader(frag_shader);
	glDeleteShader(font_vertex_shader);
	glDeleteShader(font_frag_shader);
	// TODO: free files read into memory

	// 2D set coordinate space match height and width of screen rather than 0.0-1.0
	// ORTHO
	float pj[4][4] = {{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f},  
		{0.0f, 0.0f, 0.0f, 0.0f},                    \
		{0.0f, 0.0f, 0.0f, 0.0f}};

	float rl, tb, fn;
	float left = 0.0f;
	float right = (float)SCREEN_WIDTH;
	float bottom = 0.0f;
	float top = (float)SCREEN_HEIGHT;
	float nearZ = -1.0f;
	float farZ = 1.0f;

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

	glUseProgram(texture_program);
	glUniformMatrix4fv(glGetUniformLocation(texture_program, "projection"), 1, GL_FALSE, (float *)pj);

	glUseProgram(font_program);
	glUniformMatrix4fv(glGetUniformLocation(font_program, "projection"), 1, GL_FALSE, (float *)pj);

	// FREETYPE
	// ----------------------
	//
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
	{
		printf("ERROR::FREETYPE: Could not init FreeType Library\n");
		return -1;
	}

	// load font as face
	FT_Face face;
	if (FT_New_Face(ft, "Hack-Regular.ttf", 0, &face)) {
		printf("ERROR::FREETYPE: Failed to load font at ./external/fonts/Hack-Regular.ttf\n");
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 14, 14);
	FT_GlyphSlot g = face->glyph;
	int w = 0;
	int h = 0;

	unsigned int rowh = 0;
	unsigned int roww = 0;

	for (int i = 32; i < 127; i++) {
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			fprintf(stderr, "Loading character %c failed!\n", i);
			continue;
		}
		if (roww + g->bitmap.width + 1 >= 350) {
			w = MAX(w, roww);
			h += rowh;
			roww = 0;
			rowh = 0;
		}
		roww += g->bitmap.width + 1;
		rowh = MAX(rowh, g->bitmap.rows);
	}

	w = MAX(w, roww);
	h += rowh;


	GLuint textureID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// make textures for ASCII characters 0-128
	int ox = 0;
	int oy = 0;

	/* for (int i = 65; i < 66; i++) { */
	/* 	if (FT_Load_Char(face, i, FT_LOAD_RENDER)) { */
	/* 		fprintf(stderr, "Loading character %c failed!\n", i); */
	/* 		continue; */
	/* 	} */


	/* 	if (ox + g->bitmap.width + 1 >= SCREEN_WIDTH) { */
	/* 		oy += rowh; */
	/* 		rowh = 0; */
	/* 		ox = 0; */
	/* 	} */

	/* 	/1* glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RGBA, GL_UNSIGNED_BYTE, g->bitmap.buffer); *1/ */
	/* 	/1* c[i].ax = g->advance.x >> 6; *1/ */
	/* 	/1* c[i].ay = g->advance.y >> 6; *1/ */

	/* 	/1* c[i].bw = g->bitmap.width; *1/ */
	/* 	/1* c[i].bh = g->bitmap.rows; *1/ */

	/* 	/1* c[i].bl = g->bitmap_left; *1/ */
	/* 	/1* c[i].bt = g->bitmap_top; *1/ */

	/* 	/1* c[i].tx = ox / (float)w; *1/ */
	/* 	/1* c[i].ty = oy / (float)h; *1/ */

	/* 	rowh = MAX(rowh, g->bitmap.rows); */
	/* 	ox += g->bitmap.width + 1; */
	/* } */

	float img_height = h;
	float img_width = w;
	rowh = 0;

	for (uint8_t c = 32; c <127 ; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			printf("ERROR::FREETYTPE: Failed to load Glyph\n");
			return -1;
		}

		if (ox + g->bitmap.width + 1 >= img_width) {
			oy += rowh;
			rowh = 0;
			ox = 0;
		}

		glTexSubImage2D(GL_TEXTURE_2D,
				0,
				ox,
				oy,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
				);
		rowh = MAX(rowh, g->bitmap.rows);
		ox += g->bitmap.width + 1;
	}
	

	/* glBindTexture(GL_TEXTURE_2D, 0); */

	/* FT_Done_Face(face); */
	/* FT_Done_FreeType(ft); */

	// configure VAO/VBO for texture quads
	// -----------------------------------
	/* glGenVertexArrays(1, &VAO); */
	/* glGenBuffers(1, &VBO); */
	/* glBindVertexArray(VAO); */
	/* glBindBuffer(GL_ARRAY_BUFFER, VBO); */
	/* glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); */
	/* glEnableVertexAttribArray(0); */
	/* glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); */
	/* glBindBuffer(GL_ARRAY_BUFFER, 0); */
	/* glBindVertexArray(0); */

	char *code = "Good news, everyone!";

	float vertices[] = {};

	unsigned int indices[] = {  // note that we start from 0!
		// snake head
		0, 1, 3,  // first Triangle
		1, 2, 3,   // second Triangle
	};

	// step one create teh buffer objects
	GLuint vertex_array_object;
	GLuint vertex_buffer_object;
	GLuint element_buffer_object;

	glGenVertexArrays(1, &vertex_array_object);
	glGenBuffers(1, &vertex_buffer_object);
	glGenBuffers(1, &element_buffer_object);
	glBindVertexArray(vertex_array_object);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	int quit = 0;

	while (!quit) {
		glfwPollEvents();

		/* glDisable(GL_CULL_FACE); */
		/* glDisable(GL_BLEND); */

		const GLfloat bgColor[] = {0.2f,0.3f,0.5f,1.0f};
		glClearBufferfv(GL_COLOR, 0, bgColor);

		glUseProgram(texture_program);
		int loc = glGetUniformLocation(texture_program, "u_Textures");
		int samplers[1] = {0};
		glUniform1iv(loc, 1, samplers);

		glBindTextureUnit(0, textureID);
		glBindVertexArray(vertex_array_object);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

		float x = img_height + 400;
		float y = img_width + 400;

		float vertices[] = {
			// snake
			x - img_width, y, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,// top left

			x, y, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,// top right
			x,  y - img_height, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // bottom right
			x - img_width, y - img_height, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f// bottom left
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)12);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)28);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)36);

		glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);

		/* glEnable(GL_CULL_FACE); */
		/* glEnable(GL_BLEND); */
		/* glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */

		/* float text_padding_left = 10.0f; */
		/* float text_y_coordinates = (float)(SCREEN_HEIGHT) - 18; */

		/* render_text( */
		/* 		code, */ 
		/* 		text_padding_left, */ 
		/* 		text_y_coordinates, */
		/* 		1.0f */
		/* 		); */

		glfwSwapBuffers(window);
	}

	/* glDeleteVertexArrays(1, &vertex_array_object); */
	/* glDeleteProgram(program); */
	glfwTerminate();
}

void render_text(const char *text, float x, float y, float scale)
{
	glUseProgram(font_program);

	// font color
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	glUniform3f(glGetUniformLocation(font_program, "textColor"), r, g, b);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	float xpos = 0.0f;
	float ypos = 0.0f;

	for (unsigned int c = 0; c != strlen(text); c++) 
	{
		Character ch = Characters[(int)text[c]];
		xpos = x + ch.Bearing.x * scale;
		ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },            
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }           
		};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.Advance >> 6) * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
