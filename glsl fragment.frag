#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixFactor; // 0.0..1.0

void main()
{
    vec4 c1 = texture(texture1, TexCoord);
    vec4 c2 = texture(texture2, TexCoord);
    FragColor = mix(c1, c2, mixFactor);
}