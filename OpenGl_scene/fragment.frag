#version 330 core
in vec3 vAttr;       // sphere: normal (object space), tetra: color
in vec3 vWorldPos;

out vec4 FragColor;

uniform int uObjectType;   // 0 = left sphere (Phong), 1 = tetra, 2 = right sphere (coord)

uniform vec3 lightPos;
uniform vec3 viewPos;      // pozycja kamery w world space
uniform mat4 model;

// parametry Phonga
uniform float ambientStrength;   // np. 0.20
uniform float diffuseStrength;   // np. 0.80
uniform float specularStrength;  // np. 0.50
uniform float shininess;         // np. 32.0

void main()
{
    // tetra: bez zmian
    if (uObjectType == 1)
    {
        FragColor = vec4(vAttr, 1.0);
        return;
    }

    // prawa kula: kolor z koordynatow
    if (uObjectType == 2)
    {
        vec3 c = 0.5 + 0.5 * normalize(vWorldPos);
        FragColor = vec4(c, 1.0);
        return;
    }

    // lewa kula: Phong
    vec3 baseColor  = vec3(1.0); // biala kula
    vec3 lightColor = vec3(1.0); // biale swiatlo

    // normal do world space
    vec3 N = normalize(mat3(transpose(inverse(model))) * vAttr);

    vec3 L = normalize(lightPos - vWorldPos);
    vec3 V = normalize(viewPos  - vWorldPos);

    // ambient
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diffuseStrength * diff * lightColor;

    // specular (Phong)
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, 1.0);
}
