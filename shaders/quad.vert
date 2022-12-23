#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

uniform mat4 projection;

out vec4 vs_color;

void main(void)
{
  gl_Position = projection * vec4(position, 1.0);
  vs_color = color; 
}
