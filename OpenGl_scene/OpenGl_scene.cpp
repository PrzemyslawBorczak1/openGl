#include <glad/glad.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include <iostream>
#include <vector>
#include <cmath>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ---------- Sphere generation: outputs interleaved pos(3) + normal(3) ----------
static void GenerateSpherePN(
    float radius,
    int stacks,
    int sectors,
    std::vector<float>& outInterleavedPN,   // [px,py,pz,nx,ny,nz]...
    std::vector<unsigned int>& outIndices
)
{
    outInterleavedPN.clear();
    outIndices.clear();

    const float PI = 3.14159265358979323846f;

    // vertices
    for (int i = 0; i <= stacks; ++i)
    {
        float v = (float)i / (float)stacks;
        float theta = v * PI; // 0..pi

        float y = radius * std::cos(theta);
        float r = radius * std::sin(theta);

        for (int j = 0; j <= sectors; ++j)
        {
            float u = (float)j / (float)sectors;
            float phi = u * 2.0f * PI; // 0..2pi

            float x = r * std::cos(phi);
            float z = r * std::sin(phi);

            glm::vec3 pos(x, y, z);
            glm::vec3 nrm = glm::normalize(pos); // for a sphere centered at origin

            outInterleavedPN.push_back(pos.x);
            outInterleavedPN.push_back(pos.y);
            outInterleavedPN.push_back(pos.z);

            outInterleavedPN.push_back(nrm.x);
            outInterleavedPN.push_back(nrm.y);
            outInterleavedPN.push_back(nrm.z);
        }
    }

    // indices
    int ring = sectors + 1;
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < sectors; ++j)
        {
            unsigned int k1 = i * ring + j;
            unsigned int k2 = (i + 1) * ring + j;

            outIndices.push_back(k1);
            outIndices.push_back(k2);
            outIndices.push_back(k1 + 1);

            outIndices.push_back(k1 + 1);
            outIndices.push_back(k2);
            outIndices.push_back(k2 + 1);
        }
    }
}

static glm::mat4 RotatingModel()
{
    float t = (float)glfwGetTime();
    return glm::rotate(glm::mat4(1.0f),
        t * glm::radians(40.0f),
        glm::vec3(0.5f, 1.0f, 0.0f));
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1 shader: Phong sphere + coord sphere + tetra", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // ONE shader for everything
    Shader shader("vertex.vert", "fragment.frag");

    // ===================== SPHERE (pos+normal) =====================
    std::vector<float> spherePN;
    std::vector<unsigned int> sphereIndices;

    float radius = 1.5f;
    int stacks = 32;
    int sectors = 64;
    GenerateSpherePN(radius, stacks, sectors, spherePN, sphereIndices);

    unsigned int sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, spherePN.size() * sizeof(float), spherePN.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    // location 0: position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // location 1: "attr" = normal for sphere
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // sphere transforms
    glm::mat4 modelLeft = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, -10.0f));
    glm::mat4 modelRight = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, -10.0f));

    // ===================== TETRAHEDRON (pos+color) =====================
    float tetraVertices[] = {
        // positions           // colors
         1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.0f, // v0
        -1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, // v1
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f, 1.0f, // v2
         1.0f, -1.0f, -1.0f,    1.0f, 1.0f, 0.0f  // v3
    };

    unsigned int tetraIndices[] = {
        0, 1, 2,
        0, 3, 1,
        0, 2, 3,
        1, 3, 2
    };

    unsigned int tetraVAO, tetraVBO, tetraEBO;
    glGenVertexArrays(1, &tetraVAO);
    glGenBuffers(1, &tetraVBO);
    glGenBuffers(1, &tetraEBO);

    glBindVertexArray(tetraVAO);

    glBindBuffer(GL_ARRAY_BUFFER, tetraVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetraVertices), tetraVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tetraEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetraIndices), tetraIndices, GL_STATIC_DRAW);

    // location 0: position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // location 1: "attr" = color for tetra
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // ===================== VIEW (shared) =====================
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 8.0f),
        glm::vec3(0.0f, 0.0f, -10.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // simple light/camera settings
    glm::vec3 lightPos(0.0f, 5.0f, 5.0f);

    while (!glfwWindowShouldClose(window))
    {
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (fbW <= 0) fbW = SCR_WIDTH;
        if (fbH <= 0) fbH = SCR_HEIGHT;

        glViewport(0, 0, fbW, fbH);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)fbW / (float)fbH,
            0.1f, 200.0f);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("lightPos", glm::vec3(0.0f, 5.0f, 5.0f));
        shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 8.0f)); // kamera jak w lookAt()

        shader.setFloat("ambientStrength", 0.20f);
        shader.setFloat("diffuseStrength", 0.80f);
        shader.setFloat("specularStrength", 0.50f);
        shader.setFloat("shininess", 32.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // uniforms for Phong (used only by left sphere)
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", glm::vec3(1.0f));
        shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 8.0f)); // camera position from lookAt
        shader.setFloat("ambientStrength", 0.20f);
        shader.setFloat("diffuseStrength", 1.00f);

        // ---------- LEFT SPHERE: Phong (ambient+diffuse only) ----------
        shader.setInt("uObjectType", 0);
        shader.setMat4("model", modelLeft);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // ---------- RIGHT SPHERE: color from coordinates ----------
        shader.setInt("uObjectType", 2);
        shader.setMat4("model", modelRight);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // ---------- TETRAHEDRON: unchanged (vertex colors), rotating ----------
        shader.setInt("uObjectType", 1);
        glm::mat4 modelTetra =
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)) *
            RotatingModel();

        shader.setMat4("model", modelTetra);

        glBindVertexArray(tetraVAO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteVertexArrays(1, &sphereVAO);

    glDeleteBuffers(1, &tetraVBO);
    glDeleteBuffers(1, &tetraEBO);
    glDeleteVertexArrays(1, &tetraVAO);

    glfwTerminate();
    return 0;
}
