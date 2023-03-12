#version 460 core
out vec4 fragment;

uniform vec4 uColor;

void main()
{
    fragment = uColor;
} 