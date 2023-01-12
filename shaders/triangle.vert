#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec4 color;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

//uniform mat4 transform;
//uniform vec4 aColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	color = vec4(aColor, 1.0);
}
