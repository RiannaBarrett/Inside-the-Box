#version 400 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D envMap;

void main()
{
    FragColor = texture(envMap, TexCoords);
}