#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "GLFW/glfw3.h"

#include "glextloader.c"
#include "file.h"
#include "gl_compile_errors.h"
#include "base.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEBUG 1

#define NUM_GLYPHS 128
#define LETTER_SPACING 12.75f
#define LINE_SPACING 32 
#define LEFT_PADDING 18
#define TOP_PADDING 32

#define SCREEN_WIDTH  1980 
#define SCREEN_HEIGHT  786

U8 mouse[64];

F32 text_width = 16;
F32 text_height = 16;
U64 font_atlas_width = 416;
U64 font_atlas_height = 64;

US64 indices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3,   // second Triangle
};

void draw_font_atlas();
void draw_text(U8* text, RGBA *rgba, F32 x, F32 y);
void draw_button();
GLuint compile_shaders(void);

GLuint textureID;
GLuint vertex_shader;
GLuint frag_shader;
GLuint quad_vertex_shader;
GLuint quad_frag_shader;
GLuint texture_program;
GLuint quad_program;
GLuint vertex_array_object;
GLuint vertex_buffer_object;
GLuint element_buffer_object;

U64 w = 0;
U64 h = 0;

Character c[128];

C8 *texture_vert_shader = "shaders/texture.vert";
C8 *texture_frag_shader = "shaders/texture.frag";
C8 *quad_vert_shader_path = "shaders/quad.vert";
C8 *quad_frag_shader_path = "shaders/quad.frag";

US64 VAO, VBO;


U8 RUNNING = 1;

void cursor_handler(GLFWwindow* window, F64 xpos, F64 ypos)
{
	sprintf(mouse, "%f, %f", xpos, ypos);

}

void key_handler(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		RUNNING = 0;
	}
	return;
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

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

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glfwSetKeyCallback(window, key_handler);
	glfwSetCursorPosCallback(window, cursor_handler);

	// find memory location of the opengl functions used below
	load_gl_extensions();

	C8 *vert_shader_source = read_file(texture_vert_shader);
	C8 *frag_shader_source = read_file(texture_frag_shader);

	C8 *font_vert_shader_source = read_file(quad_vert_shader_path);
	C8 *quad_frag_shader_source = read_file(quad_frag_shader_path);

	// compiles vert shader
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vert_shader_source, NULL);
	glCompileShader(vertex_shader);
	checkCompileErrors(vertex_shader, "VERTEX");

	quad_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(quad_vertex_shader, 1, &font_vert_shader_source, NULL);
	glCompileShader(quad_vertex_shader);
	checkCompileErrors(quad_vertex_shader, "QUADVERT");

	// compiles frag shader
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_shader_source, NULL);
	glCompileShader(frag_shader);
	checkCompileErrors(frag_shader, "FRAG");

	quad_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(quad_frag_shader, 1, &quad_frag_shader_source, NULL);
	glCompileShader(quad_frag_shader);
	checkCompileErrors(quad_frag_shader, "QUADFRAG");

	// create program
	texture_program = glCreateProgram();
	glAttachShader(texture_program, vertex_shader);
	glAttachShader(texture_program, frag_shader);
	glLinkProgram(texture_program);

	quad_program = glCreateProgram();
	glAttachShader(quad_program, quad_vertex_shader);
	glAttachShader(quad_program, quad_frag_shader);
	glLinkProgram(quad_program);

	// delete shaders because they are part of the program now
	glDeleteShader(vertex_shader);
	glDeleteShader(frag_shader);
	glDeleteShader(quad_vertex_shader);
	glDeleteShader(quad_frag_shader);
	// TODO: free files read into memory

	// 2D set coordinate space match height and width of screen rather than 0.0-1.0
	// ORTHO
	F32 pj[4][4] = {{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f},  
		{0.0f, 0.0f, 0.0f, 0.0f},                    \
		{0.0f, 0.0f, 0.0f, 0.0f}};

	F32 rl, tb, fn;
	F32 left = 0.0f;
	F32 right = (F32)SCREEN_WIDTH;
	F32 bottom = 0.0f;
	F32 top = (F32)SCREEN_HEIGHT;
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

	glUseProgram(texture_program);
	glUniformMatrix4fv(glGetUniformLocation(texture_program, "projection"), 1, GL_FALSE, (F32 *)pj);

	glUseProgram(quad_program);
	glUniformMatrix4fv(glGetUniformLocation(quad_program, "projection"), 1, GL_FALSE, (F32 *)pj);

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

	FT_Set_Pixel_Sizes(face, 20, 20);
	FT_GlyphSlot g = face->glyph;

	U64 rowh = 0;

	w = font_atlas_width;
	h = font_atlas_height;

	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// make textures for ASCII characters 0-128
	U64 cell_size = 16;
	U64 ox = 0;
	U64 oy = 0;
	rowh = 0;

	U64 txidx = 0;
	U64 tyidx = 0;

	for (U8 i = 32; i < NUM_GLYPHS ; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			printf("ERROR::FREETYTPE: Failed to load Glyph\n");
			return -1;
		}

		if (ox + g->bitmap.width + 1 >= w) {
			tyidx++;
			txidx = 0;
			oy += rowh;
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
				g->bitmap.buffer
				);

		c[i].xoffset = txidx++;
		c[i].yoffset = tyidx;

		c[i].ax = g->advance.x >> 6;
		c[i].ay = g->advance.y >> 6;

		c[i].bw = g->bitmap.width;
		c[i].bh = g->bitmap.rows;

		c[i].bl = g->bitmap_left;
		c[i].bt = g->bitmap_top;

		c[i].tx = ox / (F32)w;
		c[i].ty = oy / (F32)h;


		F32 row_height = cell_size - MAX(rowh, g->bitmap.rows);
		rowh = MAX(rowh, g->bitmap.rows) + row_height;

		// column between characters
		F32 col_width = cell_size - g->bitmap.width;
		ox += g->bitmap.width + col_width;
	}


	/* glBindTexture(GL_TEXTURE_2D, 0); */

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

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

	U8 *text = "Good news, everyone!";

	// font color
	/* float r = 1.0f; */
	/* float g = 1.0f; */
	/* float b = 1.0f; */
	glUseProgram(quad_program);
	glUniform3f(glGetUniformLocation(quad_program, "color"), 1.0, 1.0, 1.0);

	while (RUNNING) {
		glfwPollEvents();

		const GLfloat bgColor[] = {0.10f,0.10f,0.10f,1.0f};
		glClearBufferfv(GL_COLOR, 0, bgColor);
		glUseProgram(texture_program);
		int loc = glGetUniformLocation(texture_program, "u_Textures");
		int samplers[1] = {0};
		glUniform1iv(loc, 1, samplers);
		glBindTextureUnit(0, textureID);
		glBindVertexArray(vertex_array_object);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

#if DEBUG
		draw_font_atlas();
#endif
		F32 x = cell_size + LEFT_PADDING;
		RGBA white = {1.0f, 1.0f, 1.0f, 1.0f};
		draw_text(text, &white, x, SCREEN_HEIGHT - TOP_PADDING);

		U8 *text2 = "another bit of text";
		draw_text(text2, &white, (SCREEN_WIDTH / 2), SCREEN_HEIGHT - TOP_PADDING);

		draw_button();

		draw_text(mouse, &white, SCREEN_WIDTH - 300, SCREEN_HEIGHT - TOP_PADDING);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &vertex_array_object);
	glDeleteProgram(texture_program);
	glDeleteProgram(quad_program);
	glfwTerminate();
}

void draw_text(U8 *text, RGBA *rgba, F32 xpos, F32 ypos)
{
	glUseProgram(texture_program);

	for (size_t i = 0; i < strlen(text); i++) {
		Character ch = c[(int)text[i]];

		F32 x = (i + xpos) + (i * LETTER_SPACING);
		F32 y = ypos + ch.bt;
		int yl= ch.yoffset;
		int xl= ch.xoffset;

		F32 blx = ((xl* text_width) / w);
		F32 bly = ((yl* text_height) / h); // bl
		F32 brx = (((1+xl) * text_width) / w);
		F32 bry = ((yl * text_height) / h); // br
		F32 trx = (((1+xl) * text_width) / w);
		F32 try = (((1+yl) * text_height) / h); // tr
		F32 tlx = ((xl * text_width) / w);
		F32 tly = (((1+yl) * text_height) / h); //tl

		F32 verts[] = {
			// pos
			x - text_width, y, 0.0f,
			// color
			rgba->r, rgba->g, rgba->b, rgba->a, 
			// tex coords
			blx, bly, // bottom left
			// tex index
			0.0f,// top left

			x, y, 0.0f, 
			rgba->r, rgba->g, rgba->b, rgba->a, 
			brx, bry, // bottom right
			0.0f,// top right

			x,  y - text_height, 0.0f, 
			rgba->r, rgba->g, rgba->b, rgba->a, 
			trx, try,  // top right
			0.0f, // bottom right

			x - text_width, y - text_height, 0.0f,  
			rgba->r, rgba->g, rgba->b, rgba->a, 
			tlx, tly,  // top left
			0.0f// bottom left
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(F32)*10), (void *)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (sizeof(F32)*10), (void *)12);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (sizeof(F32)*10), (void *)28);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, (sizeof(F32)*10), (void *)36);

		glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
	}
}

void draw_button() {
	glUseProgram(quad_program);
	F32 btn_h = 40;
	F32 btn_w = 200;
	F32 x =  400;
	F32 y = SCREEN_HEIGHT - 100;

	F32 vertices[] = {
		// top left
		// location
		x - btn_w, y, 0.0f, 
		// color
		1.0f, 0.5f, 0.0f, 0.5f,

		// top right
		x, y, 0.0f,
		1.0f, 0.5f, 0.0f, 0.5f,

		// bottom right
		x,  y - btn_h, 0.0f,
		1.0f, 0.5f, 0.0f, 0.5f,

		// bottom left
		x - btn_w, y - btn_h, 0.0f,  
		1.0f, 0.5f, 0.0f, 0.5f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(F32)*7), (void *)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (sizeof(F32)*7), (void *)12);

	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);


	glUseProgram(texture_program);

	RGBA white = {1.0f, 1.0f, 1.0f, 1.0f};
	U8 *text2 = "Do Something";
	draw_text(text2, &white, 235, SCREEN_HEIGHT - 106 - (btn_h / 2));
}

void draw_font_atlas() {
	F32 img_height = font_atlas_height;
	F32 img_width = font_atlas_width;
	F32 xt = (SCREEN_WIDTH / 2) + (img_width / 2);
	F32 yt = SCREEN_HEIGHT / 2;

	F32 vertices[] = {
		xt - img_width, yt, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,// top left

		xt, yt, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,// top right
		xt,  yt - img_height, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // bottom right
		xt - img_width, yt - img_height, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f// bottom left
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(F32)*10), (void *)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (sizeof(F32)*10), (void *)12);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (sizeof(F32)*10), (void *)28);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, (sizeof(F32)*10), (void *)36);

	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
}
