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

static void GenerateSphere(
    float radius,
    int stacks,
    int sectors,
    std::vector<float>& outPositions,
    std::vector<unsigned int>& outIndices
)
{
    outPositions.clear();
    outIndices.clear();

    const float PI = 3.14159265358979323846f;

    for (int i = 0; i <= stacks; ++i)
    {
        float v = (float)i / (float)stacks;   // 0..1
        float theta = v * PI;                 // 0..pi

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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2 biale kule obok siebie", NULL, NULL);
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

    Shader sphereShader("vertex.vert", "fragment.frag");

    // --------- sphere mesh once ---------
    std::vector<float> positions;
    std::vector<unsigned int> indices;

    float radius = 1.5f;
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // --------- camera (both spheres in view) ---------
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 8.0f),  // camera
        glm::vec3(0.0f, 0.0f, 0.0f),  // look at center between spheres
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Offsets so NONE is in the center:
    // left sphere center at x = -2, right sphere center at x = +2 (0 is empty space)
    glm::mat4 modelLeft = glm::translate(glm::mat4(1.0f), glm::vec3(-5.2f, 0.0f, -10.0f));
    glm::mat4 modelRight = glm::translate(glm::mat4(1.0f), glm::vec3(5.2f, 0.0f, -10.0f));

    while (!glfwWindowShouldClose(window))
    {
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (fbW <= 0) fbW = SCR_WIDTH;
        if (fbH <= 0) fbH = SCR_HEIGHT;

        glViewport(0, 0, fbW, fbH);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)fbW / (float)fbH, 0.1f, 200.0f);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sphereShader.use();
        sphereShader.setMat4("view", view);
        sphereShader.setMat4("projection", projection);
        sphereShader.setVec3("uColor", glm::vec3(1.0f, 1.0f, 1.0f)); // white

        glBindVertexArray(VAO);

        // draw left sphere
        sphereShader.setMat4("model", modelLeft);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        // draw right sphere
        sphereShader.setMat4("model", modelRight);
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
