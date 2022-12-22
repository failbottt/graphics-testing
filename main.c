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

#define NUM_GLYPHS 128

#define SCREEN_WIDTH  1980 
#define SCREEN_HEIGHT  786
#define MAX(a,b) (((a)>(b))?(a):(b))

unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3,   // second Triangle
};

void render_text(const char* text, float x, float y, float scale);
GLuint compile_shaders(void);

GLuint textureID;
GLuint vertex_shader;
GLuint frag_shader;
GLuint font_vertex_shader;
GLuint font_frag_shader;
GLuint texture_program;
GLuint font_program;
GLuint vertex_array_object;
GLuint vertex_buffer_object;
GLuint element_buffer_object;

int w = 0;
int h = 0;

typedef struct Character {
	int xoffset; // x offset in texture
	int yoffset; // y offset in texture

	float ax;	// advance.x
	float ay;	// advance.y

	float bw;	// bitmap.width;
	float bh;	// bitmap.height;

	float bl;	// bitmap_left;
	float bt;	// bitmap_top;

	float tx;	// x offset of glyph in texture coordinates
	float ty;	// y offset of glyph in texture coordinates
} Character;

Character c[128];

const char *texture_vert_shader = "shaders/texture.vert";
const char *texture_frag_shader = "shaders/texture.frag";
const char *font_vert_shader_path = "shaders/font.vert";
const char *font_frag_shader_path = "shaders/font.frag";

unsigned int VAO, VBO;

typedef struct {
	float x;
	float y;
} VEC2;

typedef struct point{
	GLfloat x;
	GLfloat y;
	GLfloat s;
	GLfloat t;
} point;

int RUNNING = 1;

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

	glfwSetKeyCallback(window, key_handler);

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

	FT_Set_Pixel_Sizes(face, 20, 20);
	FT_GlyphSlot g = face->glyph;

	int font_atlas_width = 416;
	int font_atlas_height = 64;
	unsigned int rowh = 0;
	unsigned int roww = 0;

	/* for (int i = 32; i < NUM_GLYPHS; i++) { */
	/* 	if (FT_Load_Char(face, i, FT_LOAD_RENDER)) { */
	/* 		fprintf(stderr, "Loading character %c failed!\n", i); */
	/* 		continue; */
	/* 	} */
	/* 	if (roww + g->bitmap.width + 1 >= font_atlas_width) { */
	/* 		w = MAX(w, roww); */
	/* 		h += rowh; */
	/* 		roww = 0; */
	/* 		rowh = 0; */
	/* 	} */
	/* 	roww += g->bitmap.width + 1; */
	/* 	rowh = MAX(rowh, g->bitmap.rows); */
	/* } */

	/* 1024 × 384 pixels */

	w = font_atlas_width;
	h = font_atlas_height;


	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	// make textures for ASCII characters 0-128
	int cell_size = 16;
	int ox = 2;
	int oy = 0;
	float img_height = h;
	float img_width = w;
	rowh = 0;

	unsigned int txidx = 0;
	unsigned int tyidx = 0;

	for (uint8_t i = 32; i < NUM_GLYPHS ; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			printf("ERROR::FREETYTPE: Failed to load Glyph\n");
			return -1;
		}

		if (ox + g->bitmap.width + 1 >= img_width) {
			tyidx++;
			txidx = 0;
			oy += rowh;
			ox = 2;
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

		c[i].xoffset = txidx++;
		c[i].yoffset = tyidx;

		c[i].ax = g->advance.x >> 6;
		c[i].ay = g->advance.y >> 6;

		c[i].bw = g->bitmap.width;
		c[i].bh = g->bitmap.rows;

		c[i].bl = g->bitmap_left;
		c[i].bt = g->bitmap_top;

		c[i].tx = ox / (float)w;
		c[i].ty = oy / (float)h;


		float row_height = cell_size - MAX(rowh, g->bitmap.rows);
		rowh = MAX(rowh, g->bitmap.rows) + row_height;

		// column between characters
		float col_width = cell_size - g->bitmap.width;
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float vertices[] = {};

	// step one create teh buffer objects

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

	float sx = 2.0 / SCREEN_WIDTH;
	float sy = 2.0 / SCREEN_HEIGHT;

	while (RUNNING) {
		glfwPollEvents();

		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);

		const GLfloat bgColor[] = {0.10f,0.10f,0.10f,1.0f};
		glClearBufferfv(GL_COLOR, 0, bgColor);

		glUseProgram(texture_program);
		int loc = glGetUniformLocation(texture_program, "u_Textures");
		int samplers[1] = {0};
		glUniform1iv(loc, 1, samplers);

		glBindTextureUnit(0, textureID);
		glBindVertexArray(vertex_array_object);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

		float img_height = font_atlas_height;
		float img_width = font_atlas_width;
		float xt = (SCREEN_WIDTH / 2) + (img_width / 2);
		float yt = SCREEN_HEIGHT / 2;

		float vertices[] = {
			// snake
			xt - img_width, yt, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,// top left

			xt, yt, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,// top right
			xt,  yt - img_height, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // bottom right
			xt - img_width, yt - img_height, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f// bottom left
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

		float spritewidth = 16;
		float spriteheight = 16;

		char *text = "Good news, everyone!";
		for (int i = 0; i < strlen(text); i++) {
			Character ch = c[(int)text[i]];

			int yl= ch.yoffset;
			int xl= ch.xoffset;
			float x = w - w / 2 + (i * spritewidth + 40);
			float y = SCREEN_HEIGHT - spriteheight + ch.bt;

			float blx = ((xl* spritewidth) / w);
			float bly = ((yl* spriteheight) / h); // bl
			float brx = (((1+xl) * spritewidth) / w);
			float bry = ((yl * spriteheight) / h); // br
			float trx = (((1+xl) * spritewidth) / w);
			float try = (((1+yl) * spriteheight) / h); // tr
			float tlx = ((xl * spritewidth) / w);
			float tly = (((1+yl) * spriteheight) / h); //tl

			float verts[] = {
				// pos
				x - spritewidth, y, 0.0f,
				// color
				1.0f, 0.0f, 0.0f, 1.0f, 
				// tex coords
				blx, bly, // bottom left
				// tex index
				0.0f,// top left

				x, y, 0.0f, 
				1.0f, 0.0f, 0.0f, 1.0f,
				brx, bry, // bottom right
				0.0f,// top right

				x,  y - spriteheight, 0.0f, 
				1.0f, 0.0f, 0.0f, 1.0f, 
				trx, try,  // top right
				0.0f, // bottom right

				x - spritewidth, y - spriteheight, 0.0f,  
				1.0f, 0.0f, 0.0f, 1.0f, 
				tlx, tly,  // top left
				0.0f// bottom left
			};

			glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)0);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)12);

			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)28);

			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)36);

			glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
		}


		/* glEnable(GL_CULL_FACE); */
		/* glEnable(GL_BLEND); */
		/* glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */

		float text_padding_left = SCREEN_WIDTH / 2;
		float text_y_coordinates = SCREEN_HEIGHT / 2;

		/* render_text( */
		/* 		"A", */ 
		/* 		text_padding_left, */ 
		/* 		text_y_coordinates, */
		/* 		1.0f */
		/* 		); */

		glfwSwapBuffers(window);
	}

	/* glDeleteVertexArrays(1, &vertex_array_object); */
	glDeleteProgram(texture_program);
	glDeleteProgram(font_program);
	glfwTerminate();
}

void render_text(const char *text, float x, float y, float scale)
{
	glUseProgram(texture_program);

	// font color
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;

	int loc = glGetUniformLocation(texture_program, "u_Textures");
	int samplers[1] = {0};
	glUniform1iv(loc, 1, samplers);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glBindVertexArray(VAO);

	float xpos = 0.0f;
	float ypos = 0.0f;

	for (unsigned int i = 0; i < strlen(text); i++) 
	{
		Character ch = c[(int)text[i]];
		xpos = x + ch.bl * scale;
		ypos = y - (ch.bw - ch.bt) * scale;

		/* VEC2 size = {face->glyph->bitmap.width, face->glyph->bitmap.rows}; */
		/* VEC2 bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top}; */
		/* struct { */
		/* 	float ax;	// advance.x */
		/* 	float ay;	// advance.y */

		/* 	float bw;	// bitmap.width; */
		/* 	float bh;	// bitmap.height; */

		/* 	float bl;	// bitmap_left; */
		/* 	float bt;	// bitmap_top; */

		/* 	float tx;	// x offset of glyph in texture coordinates */
		/* 	float ty;	// y offset of glyph in texture coordinates */
		/* } c[128]; */
		float w = ch.bw * scale;
		float h = ch.bh * scale;
		/* float vertices[6][4] = { */
		/* 	{ xpos,     ypos + h,   0.0f, 0.0f }, */            
		/* 	{ xpos,     ypos,       0.0f, 1.0f }, */
		/* 	{ xpos + w, ypos,       1.0f, 1.0f }, */

		/* 	{ xpos,     ypos + h,   0.0f, 0.0f }, */
		/* 	{ xpos + w, ypos,       1.0f, 1.0f }, */
		/* 	{ xpos + w, ypos + h,   1.0f, 0.0f } */           
		/* }; */
		float vertices[] = {
			// snake
			xpos, ypos+h, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,// top left

			x, y, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,// top right
			x+w,  ypos, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // bottom right
			x+w, y+h, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f// bottom left
		};


		glBufferData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)12);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)28);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, (sizeof(float)*10), (void *)36);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		x += ((int)ch.ax >> 6) * scale;

	}

	/* glDrawElements(GL_TRIANGLES, 0, 6); */
	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
