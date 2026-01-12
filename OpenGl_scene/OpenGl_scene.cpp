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

// Generowanie UV-sfery: pozycje + indeksy (trójk¹ty)
static void GenerateSphere(
    float radius,
    int stacks,     // np. 32
    int sectors,    // np. 64
    std::vector<float>& outPositions,        // xyzxyz...
    std::vector<unsigned int>& outIndices    // triangles
)
{
    outPositions.clear();
    outIndices.clear();

    // wierzcho³ki
    // theta: 0..pi (od bieguna do bieguna)
    // phi:   0..2pi (dooko³a)
    const float PI = 3.14159265358979323846f;

    for (int i = 0; i <= stacks; ++i)
    {
        float v = (float)i / (float)stacks;      // 0..1
        float theta = v * PI;                    // 0..pi

        float y = radius * std::cos(theta);
        float r = radius * std::sin(theta);

        for (int j = 0; j <= sectors; ++j)
        {
            float u = (float)j / (float)sectors; // 0..1
            float phi = u * 2.0f * PI;           // 0..2pi

            float x = r * std::cos(phi);
            float z = r * std::sin(phi);

            outPositions.push_back(x);
            outPositions.push_back(y);
            outPositions.push_back(z);
        }
    }

    // indeksy (dwa trójk¹ty na “quad”)
    // siatka ma (stacks+1) x (sectors+1) wierzcho³ków
    int ring = sectors + 1;

    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < sectors; ++j)
        {
            unsigned int k1 = i * ring + j;
            unsigned int k2 = (i + 1) * ring + j;

            // triangle 1
            outIndices.push_back(k1);
            outIndices.push_back(k2);
            outIndices.push_back(k1 + 1);

            // triangle 2
            outIndices.push_back(k1 + 1);
            outIndices.push_back(k2);
            outIndices.push_back(k2 + 1);
        }
    }
}

int main()
{
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Biala kula (wypelniona)", NULL, NULL);
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

    // Shader (u¿ywa Twojej klasy)
    Shader sphereShader("vertex.vert", "fragment.frag");

    // --- Geometria sfery ---
    std::vector<float> positions;            // xyz...
    std::vector<unsigned int> indices;

    float radius = 2.0f;
    int stacks = 32;
    int sectors = 64;

    GenerateSphere(radius, stacks, sectors, positions, indices);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // layout(location=0) -> vec3 position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // --- macierze ---
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 8.0f),  // kamera
        glm::vec3(0.0f, 0.0f, 0.0f),  // patrzy na œrodek
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    while (!glfwWindowShouldClose(window))
    {
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (fbW <= 0) fbW = SCR_WIDTH;
        if (fbH <= 0) fbH = SCR_HEIGHT;

        glViewport(0, 0, fbW, fbH);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)fbW / (float)fbH, 0.1f, 200.0f);

        // t³o czarne
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sphereShader.use();
        sphereShader.setMat4("model", model);
        sphereShader.setMat4("view", view);
        sphereShader.setMat4("projection", projection);

        // bia³a kula
        sphereShader.setVec3("uColor", glm::vec3(1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
    return 0;
}
