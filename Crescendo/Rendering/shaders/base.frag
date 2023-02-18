#version 460 core
out vec4 fragment;

in vec4 color;

void main()
{
    fragment = color;
} 