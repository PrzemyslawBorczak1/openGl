#include <glad/glad.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include <iostream>
#include <vector>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void SetShaderMatrices(const Shader& shader, int screenWidth, int screenHeight)
{
    float t = glfwGetTime();
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), t * glm::radians(40.0f), glm::vec3(0.5f, 1.0f, 0.0f));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), float(screenWidth) / float(screenHeight), 0.1f, 100.0f);

    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);
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

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
   
    glFrontFace(GL_CW);

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("vertex.vert", "fragment.frag"); // you can name your shader files however you like

    // positions and colors (original four verts)
    std::vector<glm::vec3> positions = {
        { 1.0f,  1.0f,  1.0f},
        {-1.0f, -1.0f,  1.0f},
        {-1.0f,  1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f}
    };
    std::vector<glm::vec3> colors = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f}
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 3, 1,
        0, 2, 3,
        1, 3, 2
    };

    // compute per-vertex normals by averaging face normals
    std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));
    for (size_t i = 0; i < sizeof(indices)/sizeof(indices[0]); i += 3)
    {
        unsigned int ia = indices[i + 0];
        unsigned int ib = indices[i + 1];
        unsigned int ic = indices[i + 2];
        glm::vec3 e1 = positions[ib] - positions[ia];
        glm::vec3 e2 = positions[ic] - positions[ia];
        glm::vec3 faceNormal = glm::normalize(glm::cross(e1, e2));
        normals[ia] += faceNormal;
        normals[ib] += faceNormal;
        normals[ic] += faceNormal;
    }
    for (auto &n : normals) n = -glm::normalize(n);

    // build interleaved buffer: pos(3), normal(3), color(3) => stride = 9 floats
    std::vector<float> interleaved;
    interleaved.reserve(positions.size() * 9);
    for (size_t i = 0; i < positions.size(); ++i)
    {
        interleaved.push_back(positions[i].x);
        interleaved.push_back(positions[i].y);
        interleaved.push_back(positions[i].z);

        interleaved.push_back(normals[i].x);
        interleaved.push_back(normals[i].y);
        interleaved.push_back(normals[i].z);

        interleaved.push_back(colors[i].x);
        interleaved.push_back(colors[i].y);
        interleaved.push_back(colors[i].z);
    }

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // attribute layout: location 0 = pos, location 1 = color, location 2 = normal
    GLsizei stride = 9 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // normal is placed next (offset = 3 floats)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // color follows (offset = 6 floats)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window))
    {
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        SetShaderMatrices(ourShader, fbW > 0 ? fbW : SCR_WIDTH, fbH > 0 ? fbH : SCR_HEIGHT);

        // set lighting uniforms (adjust values as needed)
        ourShader.setVec3("lightPos", glm::vec3(0.0f, 0.0f, 5.0f));
        ourShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setVec3("viewPos",  glm::vec3(0.0f, 0.0f, 10.0f)); // camera location (matches your viewTranslate)
        ourShader.setFloat("shininess", 32.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
    return 0;
}