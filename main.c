#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "GLFW/glfw3.h"
#include "cglm/call.h"

#include "glextloader.c"
#include "gl_compile_errors.h"
#include "file.h"

#define SCREEN_WIDTH  1980 
#define SCREEN_HEIGHT  786

int RUNNING = 1;

void 
key_handler(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
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
	glfwSetKeyCallback(window, key_handler);

	loadGlExtensions();
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	const char *triVertSource = read_file("./shaders/triangle.vert");
	unsigned int triVertShader;
	triVertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(triVertShader, 1, &triVertSource, NULL);
	glCompileShader(triVertShader);
	checkCompileErrors(triVertShader, "TRIVERT");

	const char *triFragSource = read_file("./shaders/triangle.frag");
	unsigned int triFragShader;
	triFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(triFragShader, 1, &triFragSource, NULL);
	glCompileShader(triFragShader);
	checkCompileErrors(triFragShader, "TRIFRAG");

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, triVertShader);
	glAttachShader(shaderProgram, triFragShader);
	glLinkProgram(shaderProgram);

	const GLfloat bgColor[] = {0.84f,1.0f,1.0f,1.0f};

	float vertices[] = {
		// triangle
		-0.5f, -0.5f, 0.0f, // bottom left
		0.5f, -0.5f, 0.0f, // bottom right
		0.0f,  0.5f, 0.0f, // top

		// quad
		-0.8f,  0.6f,  0.0f, //bottom left
		-0.6f, 0.6f, 0.0f, // bottom right
		-0.6f, 0.8f, 0.0f, // top
		-0.8f,  0.6f,  0.0f, //bottom left
		-0.8f, 0.8f, 0.0f, // bottom right
		-0.6f, 0.8f, 0.0f, // top
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	float id[4][4], dest[4][4];

	glm_mat4_identity(id);
	glm_mat4_identity(dest);

	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");

	float tricolor[] = {0.0f, 0.0f, 1.0f, 1.0f};
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "aColor");
	while (RUNNING) {
		glfwPollEvents();

		glClearBufferfv(GL_COLOR, 0, bgColor);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);

		glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0);
		glm_rotated_x(id, (float)glfwGetTime(), dest);
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, (float*)dest);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUniform4f(colorLoc, 0.0f, 1.0f, 1.0f, 1.0);
		glm_rotated_z(id, (float)glfwGetTime(), dest);
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, (float*)dest);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glm_rotated_y(id, (float)glfwGetTime(), dest);
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, (float*)dest);
		glUniform4f(colorLoc, tricolor[0], tricolor[1], tricolor[2], tricolor[3]);
		glDrawArrays(GL_TRIANGLES, 0, 9);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
}
