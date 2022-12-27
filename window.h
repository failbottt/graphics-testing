#include "GLFW/glfw3.h"

#define SCREEN_WIDTH  1980 
#define SCREEN_HEIGHT  786

U8 mouse[64];
F64 mouse_x_pos;
F64 mouse_y_pos;

GLFWwindow* window;

// NOTE (spangler): these function signatures are specific to 
// GLFW. It might be a good idea to use them as the entry point
// to the applications input handling code, rather than tie
// the application directly to GLFW keycodes, actions, etc.
void cursorHandler(GLFWwindow* window, F64 xpos, F64 ypos)
{
	mouse_x_pos = xpos;
	mouse_y_pos = ypos;
	sprintf(mouse, "%f, %f", xpos, ypos);
}

void keyHandler(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		RUNNING = 0;
	}
	return;
}

void
setInputHandlers()
{
	glfwSetKeyCallback(window, keyHandler);
	glfwSetCursorPosCallback(window, cursorHandler);
}

void 
initWindow() 
{
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
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "<program>", NULL, NULL);
	if (window == NULL)
	{
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	setInputHandlers();

	loadGlExtensions();
}

void
swap_window_buffers()
{
	glfwSwapBuffers(window);
}
