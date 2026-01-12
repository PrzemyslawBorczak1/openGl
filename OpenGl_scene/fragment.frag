#version 330 core
in vec3 vAttr;       // sphere: normal (object space), tetra: color
in vec3 vWorldPos;

out vec4 FragColor;

uniform int uObjectType;   // 0 = left sphere (Phong), 1 = tetra, 2 = right sphere (coord)
uniform vec3 lightPos;
uniform mat4 model;

void main()
{
    if (uObjectType == 1)
    {
        FragColor = vec4(vAttr, 1.0);
        return;
    }

    if (uObjectType == 2)
    {
        vec3 c = 0.5 + 0.5 * normalize(vWorldPos);
        FragColor = vec4(c, 1.0);
        return;
    }

    // LEFT sphere: I = I0 + Il * max(dot(N,L),0)
    // "bia³e" (neutralne) ale nie 1.0, ¿eby nie satur³o do pe³nej bieli
    vec3 I0 = vec3(0.20);   // ambient intensity
    vec3 Il = vec3(0.80);   // diffuse intensity

    vec3 N = normalize(mat3(transpose(inverse(model))) * vAttr);
    vec3 L = normalize(lightPos - vWorldPos);

    float cosNL = max(dot(N, L), 0.0);
    vec3 I = I0 + Il * cosNL;

    vec3 baseColor = vec3(1.0); // bia³a kula

    FragColor = vec4(I * baseColor, 1.0);
}
