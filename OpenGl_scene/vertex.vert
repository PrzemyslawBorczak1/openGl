#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aAttr;   // sphere: normal, tetra: color

out vec3 vAttr;
out vec3 vWorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vAttr = aAttr;

    vec4 world = model * vec4(aPos, 1.0);
    vWorldPos = world.xyz;

    gl_Position = projection * view * world;
}
