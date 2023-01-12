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
#define PI 3.1415926535

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
	glEnable(GL_CULL_FACE);
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

	const GLfloat bgColor[] = {0.2f,0.34f,0.67f,1.0f};

	float cube[] = {
		// coords			 // color
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f
    };
	/* float vertices[] = { */
	/* 	// triangle */
	/* 	-0.5f, -0.5f, 0.0f, // bottom left */
	/* 	0.5f, -0.5f, 0.0f, // bottom right */
	/* 	0.0f,  0.5f, 0.0f, // top */

	/* 	// quad */
	/* 	-0.8f,  0.6f,  0.0f, //bottom left */
	/* 	-0.6f, 0.6f, 0.0f, // bottom right */
	/* 	-0.6f, 0.8f, 0.0f, // top */
	/* 	-0.8f,  0.6f,  0.0f, //bottom left */
	/* 	-0.8f, 0.8f, 0.0f, // bottom right */
	/* 	-0.6f, 0.8f, 0.0f, // top */

	/* }; */

	/* int segments = 120; */
	/* float cverts[3*segments]; */
	/* float radius = 200.0f; */
	/* float x = -SCREEN_WIDTH + 600; */
	/* float y = -SCREEN_HEIGHT + 600; */

	/* int j = 0; */
	/* for(int i = 0; i < segments; i++) { */ 
	/* 	// polygon */
	/* 	float theta = 2.0f * PI * ((float)i) / (segments); */
	/* 	cverts[j++] = (x + radius * cos(theta)) / SCREEN_WIDTH; */
	/* 	cverts[j++] = (y = radius * sin(theta)) / SCREEN_HEIGHT; */
	/* 	cverts[j++] = 0.0f; */
	/* } */

	unsigned int VBO, VAO, VAOC, VBOC;
	glGenVertexArrays(1, &VAO);
	glGenVertexArrays(1, &VAOC);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBOC);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	// loc
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 *sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	/* glBindVertexArray(VAOC); */
	/* glBindBuffer(GL_ARRAY_BUFFER, VBOC); */
	/* glBufferData(GL_ARRAY_BUFFER, sizeof(cverts), cverts, GL_STATIC_DRAW); */
	/* glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); */
	/* glEnableVertexAttribArray(0); */

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	float id[4][4], dest[4][4];

	glm_mat4_identity(id);
	glm_mat4_identity(dest);

	/* unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform"); */

	/* float tricolor[] = {0.0f, 0.0f, 1.0f, 1.0f}; */
	/* unsigned int colorLoc = glGetUniformLocation(shaderProgram, "aColor"); */

	vec3 cameraPos = {0.0, 0.0f, 3.0f}; // zoom (in/out)
	vec3 cameraFront = {0.0, 0.0f, -1.0f}; // 
	vec3 cameraUp = {0.0, 1.0f, 0.0f};
	float view[4][4];
	glm_mat4_identity(view);
	glm_lookat(cameraPos, cameraFront, cameraUp, view);

	float model[4][4];
	glm_mat4_identity(model);

	float projection[4][4];

	glm_perspective(glm_rad(45.0f), ((float) SCREEN_WIDTH / (float) SCREEN_HEIGHT), 0.1f, 100.0f, projection);

	unsigned int uview = glGetUniformLocation(shaderProgram, "view");
	unsigned int umodel = glGetUniformLocation(shaderProgram, "model");
	unsigned int uprojection = glGetUniformLocation(shaderProgram, "projection");


	glEnable(GL_DEPTH_TEST);
	while (RUNNING) {
		glfwPollEvents();

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glClearBufferfv(GL_COLOR, 0, bgColor);

		glUseProgram(shaderProgram);

		glBindVertexArray(VAO);

		glm_rotated_y(id, (float)glfwGetTime(), model);
		glm_rotated_x(model, (float)glfwGetTime(), model);
		glUniformMatrix4fv(umodel, 1, GL_FALSE, (float*)model);
		glUniformMatrix4fv(uview, 1, GL_FALSE, (float*)view);
		glUniformMatrix4fv(uprojection, 1, GL_FALSE, (float*)projection);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* glUniform4f(colorLoc, 0.0f, 1.0f, 1.0f, 1.0); */
		/* glm_rotated_y(id, (float)glfwGetTime(), dest); */
		/* glUniformMatrix4fv(transformLoc, 1, GL_FALSE, (float*)dest); */
		/* glDrawArrays(GL_TRIANGLES, 0, 9); */

		/* glBindVertexArray(VAOC); */
		/* glm_rotated_x(id, (float)glfwGetTime(), dest); */
		/* glUniformMatrix4fv(transformLoc, 1, GL_FALSE, (float*)dest); */
		/* glUniform4f(colorLoc, tricolor[0], tricolor[1], tricolor[2], tricolor[3]); */
		/* glDrawArrays(GL_TRIANGLE_FAN, 0, 120); */

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
}
