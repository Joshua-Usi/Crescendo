#version 460 core
out vec4 fragment;

in vec3 color;

void main()
{
    fragment = vec4(color, 1.0f);
} 