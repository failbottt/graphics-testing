#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "GLFW/glfw3.h"

#include "file.h"
#include "base.h"
#include "graphics.h"
#include "font.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "settings.h"

U8 mouse[64];
F64 mouse_x_pos;
F64 mouse_y_pos;

void draw_font_atlas();
void draw_text(U8* text, RGBA *rgba, F32 x, F32 y);
void draw_button(UI_Button button);
GLuint compile_shaders(void);

GLuint vertex_array_object;
GLuint vertex_buffer_object;
GLuint element_buffer_object;

unsigned int VAO, VBO;

U8 RUNNING = 1;

void cursor_handler(GLFWwindow* window, F64 xpos, F64 ypos)
{
	mouse_x_pos = xpos;
	mouse_y_pos = ypos;
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
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "glfw_starter", NULL, NULL);
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

	initGraphics();
	setPerspective2D(SCREEN_WIDTH, SCREEN_HEIGHT);
	initFont();

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

	RGBA c = {1.0f, 0.5f, 0.0f, 0.5f};
	UI_Button btn1 = {0, 200, 40, 400, SCREEN_HEIGHT - 100, c};
	buttons[0] = btn1;

	GLuint textureID;
	{
		int w, h, bits;
		unsigned char *pixels = stbi_load("./external/images/bg_space_seamless.png", &w, &h, &bits, STBI_rgb_alpha);

		glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		stbi_image_free(pixels);
	}

	GLuint textureID2;
	{
		int w, h, bits;
		unsigned char *pixels = stbi_load("./external/images/background.png", &w, &h, &bits, STBI_rgb_alpha);

		glCreateTextures(GL_TEXTURE_2D, 1, &textureID2);
		glBindTexture(GL_TEXTURE_2D, textureID2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		stbi_image_free(pixels);
	}

	while (RUNNING) {
		glfwPollEvents();

		glClearBufferfv(GL_COLOR, 0, bgColor);

		{
			glUseProgram(image_program);
			int loc = glGetUniformLocation(image_program, "u_Textures");
			int samplers[1] = {0};
			glUniform1iv(loc, 1, samplers);

			glBindTextureUnit(0, textureID);
			glBindVertexArray(vertex_array_object);

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

			float img_height = SCREEN_HEIGHT;
			float img_width = SCREEN_WIDTH;
			float x = SCREEN_WIDTH;
			float y = SCREEN_HEIGHT;

			float vertices[] = {
				// pos									// color					/tex coord	 texindex
				x - img_width, y, 0.0f,					1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,  0.0f,// top left
				x, y, 0.0f,								1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f,  0.0f,// top right
				x,  y - img_height, 0.0f,				1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f,  0.0f, // bottom right
				x - img_width, y - img_height, 0.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,  0.0f// bottom left
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
		}
		// draw second image
		{
			glUseProgram(image_program);
			int loc = glGetUniformLocation(image_program, "u_Textures");
			int samplers[1] = {0};
			glUniform1iv(loc, 1, samplers);

			glBindTextureUnit(0, textureID2);
			glBindVertexArray(vertex_array_object);

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

			float img_height = 400;
			float img_width = 600;
			float x = SCREEN_WIDTH - 100;
			float y = SCREEN_HEIGHT - 200;

			float vertices[] = {
				// pos									// color					/tex coord	 texindex
				x - img_width, y, 0.0f,					1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,  0.0f,// top left
				x, y, 0.0f,								1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f,  0.0f,// top right
				x,  y - img_height, 0.0f,				1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f,  0.0f, // bottom right
				x - img_width, y - img_height, 0.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,  0.0f// bottom left
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
		}

		int loc = glGetUniformLocation(atlas_program, "u_Textures");
		int samplers[1] = {0};
		glUniform1iv(loc, 1, samplers);
		glBindTextureUnit(0, texture_atlas_id);
		glBindVertexArray(vertex_array_object);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

		draw_font_atlas();

		F32 x = cell_size + LEFT_PADDING;
		RGBA white = {1.0f, 1.0f, 1.0f, 1.0f};
		draw_text(text, &white, x, SCREEN_HEIGHT - TOP_PADDING);

		U8 *text2 = "another bit of text";
		draw_text(text2, &white, (SCREEN_WIDTH / 2), SCREEN_HEIGHT - TOP_PADDING);

		draw_button(btn1);

		draw_text(mouse, &white, SCREEN_WIDTH - 300, SCREEN_HEIGHT - TOP_PADDING);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &vertex_array_object);
	glDeleteProgram(atlas_program);
	glDeleteProgram(quad_program);
	glfwTerminate();
}

void 
draw_text(U8 *text, RGBA *rgba, F32 xpos, F32 ypos)
{
	F32 text_width = 16;
	F32 text_height = 16;

	glUseProgram(atlas_program);

	for (size_t i = 0; i < strlen(text); i++) {
		Character ch = c[(int)text[i]];

		F32 x = (i + xpos) + (i * LETTER_SPACING);
		F32 y = ypos + ch.bt;
		int yl= ch.yoffset;
		int xl= ch.xoffset;

		// NOTE (spangler): could cache this on the character and 
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

void draw_button(UI_Button button) 
{
	glUseProgram(quad_program);

	F32 tlx = button.x - button.width;
	F32 tly = button.y;
	
	F32 trx = button.x;
	F32 try = button.y;

    F32 brx = button.x;
	F32 bry = button.y - button.height;

	F32 blx	= button.x - button.width; 
	F32 bly = button.y - button.height;

	button.top_left  =  (VEC2) {tlx, tly};
	button.top_right =  (VEC2) {trx, try};
	button.bot_left  =  (VEC2) {brx, bry};
	button.bot_right =  (VEC2) {blx, bly};

	F32 vertices[] = {
		// top left
		// location
		button.x - button.width, button.y, 0.0f, 
		// color
		button.color.r, button.color.g, button.color.b, button.color.a,

		// top right
		button.x, button.y, 0.0f,
		button.color.r, button.color.g, button.color.b, button.color.a,

		// bottom right
		button.x,  button.y - button.height, 0.0f,
		button.color.r, button.color.g, button.color.b, button.color.a,

		// bottom left
		button.x - button.width, button.y - button.height, 0.0f,  
		button.color.r, button.color.g, button.color.b, button.color.a
	};


	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(F32)*7), (void *)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (sizeof(F32)*7), (void *)12);

	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);


	RGBA white = {1.0f, 1.0f, 1.0f, 1.0f};
	U8 *text2 = "Button";
	draw_text(text2, &white, 235 + ((5 * 16) / 2), SCREEN_HEIGHT - 106 - (button.height / 2));
}

void draw_font_atlas() {
	glUseProgram(atlas_program);
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
