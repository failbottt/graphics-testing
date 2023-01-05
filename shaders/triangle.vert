#version 460 core
layout (location = 0) in vec3 aPos;

out vec4 color;

uniform mat4 transform;
uniform vec4 aColor;

void main()
{
    gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, .85);
	color = aColor;
}
