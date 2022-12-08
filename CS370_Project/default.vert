#version 400 core
uniform mat4 proj_matrix;
uniform mat4 camera_matrix;
uniform mat4 model_matrix;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;

out vec4 oColor;

void main()
{
    gl_Position = proj_matrix*camera_matrix*model_matrix*vPosition;
    oColor = vColor;
}
