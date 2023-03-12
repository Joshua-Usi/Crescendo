#version 460 core
layout (location = 0) in vec3 aPosition;

uniform mat4 uModel;
uniform mat4 uProjectionView;

void main()
{
    gl_Position = uProjectionView * uModel * vec4(aPosition, 1.0);
}