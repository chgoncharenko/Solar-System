#version 330 core

layout(location=0) in vec3 in_position;
layout(location=2) in vec3 in_normal;
layout(location=8) in vec2 in_texcoord;

out vec4 v2f_positionW; // position in world space
out vec4 v2f_normalW;
out vec2 v2f_texcoord;

uniform mat4 ModelViewProjectionMatrix;
uniform mat4 ModelMatrix;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(in_position, 1);

    v2f_positionW = ModelMatrix * vec4(in_position, 1); //transform VerSpase,surface normal into world space
    v2f_normalW = ModelMatrix * vec4(in_normal, 0);
    v2f_texcoord = in_texcoord;
}
