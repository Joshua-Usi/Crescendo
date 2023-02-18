#version 460 core
out vec4 fragment;

in vec4 vColor;

void main()
{
    fragment = vColor;
} 