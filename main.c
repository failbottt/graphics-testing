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
#include "gl.h"
#include "font.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEBUG 1

#define LETTER_SPACING 12.75f
#define LINE_SPACING 32 
#define LEFT_PADDING 18
#define TOP_PADDING 32

#define SCREEN_WIDTH  1980 
#define SCREEN_HEIGHT  786

U8 mouse[64];

F32 text_width = 16;
F32 text_height = 16;

unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3,   // second Triangle
};

void draw_font_atlas();
void draw_text(U8* text, RGBA *rgba, F32 x, F32 y);
void draw_button();
GLuint compile_shaders(void);

GLuint vertex_array_object;
GLuint vertex_buffer_object;
GLuint element_buffer_object;


unsigned int VAO, VBO;


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

	load_gl_extensions();
	init_gl_programs();
	gl_set_perspective_as_ortho(SCREEN_WIDTH, SCREEN_HEIGHT);
	init_ft_font();

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

	const GLfloat bgColor[] = {0.10f,0.10f,0.10f,1.0f};
	while (RUNNING) {
		glfwPollEvents();

		glClearBufferfv(GL_COLOR, 0, bgColor);
		int loc = glGetUniformLocation(texture_program, "u_Textures");
		int samplers[1] = {0};
		glUniform1iv(loc, 1, samplers);
		glBindTextureUnit(0, texture_atlas_id);
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
