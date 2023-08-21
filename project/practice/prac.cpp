#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "camera_s.h"
#include "stb_image.h"
#include "stb_image_write.h"
//#include "model.h"

#include <iostream>
#include <vector>
#include <fstream>

#define M_PI 3.14159265358979323846

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

unsigned int loadTexture(const char* path);
void renderScene(const Shader& shader);
void renderCube();
void renderSphere();
void renderTriangle();
void renderCone();
void renderTriangularPrism();

void take_screenshot();
int sceneCounter = 3;
int lightCounter = 1;
int screenshotCounter = 1;


// settings
const unsigned int SCR_WIDTH = 512;
const unsigned int SCR_HEIGHT = 256;
bool shadows = true;
bool spacePressed = false;
//glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
glm::vec3 lightPos[10];


// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);  // 테두리 없애기

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    Shader shader("3.2.1.point_shadows.vs", "3.2.1.point_shadows.fs");
    Shader simpleDepthShader("3.2.1.point_shadows_depth.vs", "3.2.1.point_shadows_depth.fs", "3.2.1.point_shadows_depth.gs");

    // load textures
    // -------------
    unsigned int woodTexture;
    if (sceneCounter == 1) woodTexture = loadTexture("wood.png");
    if (sceneCounter == 2) woodTexture = loadTexture("123.png");
    if (sceneCounter == 3) woodTexture = loadTexture("456.jpg");

    lightPos[0] = glm::vec3(0.0f, 0.0f, 0.0f);
    lightPos[1] = glm::vec3(1.0f, 1.0f, 3.0f);
    lightPos[2] = glm::vec3(2.0f, -2.0f, 1.0f);
    lightPos[3] = glm::vec3(3.0f, 3.0f, 5.0f);
    lightPos[4] = glm::vec3(4.0f, 4.0f, -1.0f);
    lightPos[5] = glm::vec3(-2.0f, 5.0f, 0.3f);
    lightPos[6] = glm::vec3(-4.0f, 6.0f, 3.0f);
    lightPos[7] = glm::vec3(-5.0f, 7.0f, -3.0f);
    lightPos[8] = glm::vec3(5.0f, -8.0f, 0.0f);
    lightPos[9] = glm::vec3(3.0f, 9.0f, -1.0f);

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("depthMap", 1);

    // lighting info
    // -------------
    //glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // move light position over time
        //lightPos.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);          // 빛 이동하는 부분

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos[lightCounter], lightPos[lightCounter] + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos[lightCounter], lightPos[lightCounter] + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos[lightCounter], lightPos[lightCounter] + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos[lightCounter], lightPos[lightCounter] + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos[lightCounter], lightPos[lightCounter] + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos[lightCounter], lightPos[lightCounter] + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        simpleDepthShader.setFloat("far_plane", far_plane);
        simpleDepthShader.setVec3("lightPos", lightPos[lightCounter]);
        renderScene(simpleDepthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal      -     좌측 절반
        // -------------------------

        

        glViewport(0, 0, SCR_WIDTH / 2, SCR_HEIGHT);
        shadows = true;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / 2 / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // set lighting uniforms
        shader.setVec3("lightPos", lightPos[lightCounter]);
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
        shader.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(shader);

        // 3. render scene as normal      -     우측 절반
        // -------------------------
        

        glViewport(SCR_WIDTH / 2, 0, SCR_WIDTH / 2, SCR_HEIGHT);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shadows = false;
        shader.use();
        shader.setInt("shadows", shadows);
        /*
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / 2 / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // set lighting uniforms
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
        shader.setFloat("far_plane", far_plane);
        */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(shader);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// renders the 3D scene
// --------------------
void renderScene(const Shader& shader)
{
    if (sceneCounter == 1) {
        shader.setBool("light", false);

        // room cube
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f));
        shader.setMat4("model", model);
        glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
        shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
        renderCube();
        shader.setInt("reverse_normals", 0); // and of course disable it
        glEnable(GL_CULL_FACE);
        // cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, -3.5f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 3.0f, 1.0));
        model = glm::scale(model, glm::vec3(0.75f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0f, -2.0f, 0.0));
        model = glm::rotate(model, glm::radians(30.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 3.5));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, -2.0f, -4.0));
        model = glm::rotate(model, glm::radians(50.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.75f));
        shader.setMat4("model", model);
        renderCube();

        shader.setBool("light", true);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos[lightCounter]);
        model = glm::scale(model, glm::vec3(0.1f));
        shader.setMat4("model", model);
        renderCube();
    }
    
    if (sceneCounter == 2) {               // 삼각뿔 추가
        shader.setBool("light", false);

        // room cube
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f));
        shader.setMat4("model", model);
        glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
        shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
        renderCube();
        shader.setInt("reverse_normals", 0); // and of course disable it
        glEnable(GL_CULL_FACE);
        // cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.0f, -5.0f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        model = glm::rotate(model, glm::radians(40.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 3.0f, 1.0));
        model = glm::scale(model, glm::vec3(0.1f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0f, -2.0f, 0.0));
        model = glm::rotate(model, glm::radians(30.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.3f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(6.5f, 1.0f, 3.5));
        model = glm::scale(model, glm::vec3(0.6f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f, 2.0f, -1.0));
        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.75f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.0f, 7.0f, -8.0));
        model = glm::rotate(model, glm::radians(20.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.75f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.5f, -9.0f, -4.0));
        model = glm::rotate(model, glm::radians(20.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.75f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 3.0f, -2.0));
        model = glm::rotate(model, glm::radians(20.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(2.0f));
        shader.setMat4("model", model);
        renderCube();


        // 삼각뿔
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, -1.0f, 1.0));     
        shader.setMat4("model", model);
        renderCone();

        
        shader.setBool("another", false);
        shader.setBool("light", true);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos[lightCounter]);
        model = glm::scale(model, glm::vec3(0.1f));
        shader.setMat4("model", model);
        renderCube();
    }

    if (sceneCounter == 3) {        // 구 추가함
        shader.setBool("light", false);

        // room cube
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f));
        shader.setMat4("model", model);
        glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
        shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
        renderCube();
        shader.setInt("reverse_normals", 0); // and of course disable it
        glEnable(GL_CULL_FACE);
        // cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, -3.5f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 3.0f, 1.0));
        model = glm::scale(model, glm::vec3(0.75f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0f, -2.0f, 0.0));
        model = glm::rotate(model, glm::radians(30.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 3.5));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);
        renderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, -2.0f, -4.0));
        model = glm::rotate(model, glm::radians(50.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.75f));
        shader.setMat4("model", model);
        renderCube();

        // 구체

        shader.setBool("another", true);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
        shader.setMat4("model", model);
        renderSphere();


        shader.setBool("another", false);
        shader.setBool("light", true);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos[lightCounter]);
        model = glm::scale(model, glm::vec3(0.1f));
        shader.setMat4("model", model);
        renderCube();
    }
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)   // 스페이스바 누르면 스크린샷 찍는 걸로 수정 (디폴트 단일이벤트라 ㄱㅊ)
        take_screenshot();

    /*

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gKeyPressed) // G 키 누르면, 다음 빛 위치로 바뀌도록 (디폴트 반복이벤트라 따로)
    {
        
        // 스크린샷 함수로 기능 옮김

        gKeyPressed = true;  // 'G' 키가 눌렸음을 기록
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
    {
        gKeyPressed = false;
    }*/
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



// 스크린샷 찍는 함수

void take_screenshot() {

    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    int pixel_count = width * height * 3;

    std::vector<unsigned char> pixels(pixel_count);

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Construct the file name with the current screenshotCounter value
    std::stringstream ss;
    ss << "C:/Users/ppoo9/Desktop/data/test/" << sceneCounter << "_" << lightCounter << "_" << screenshotCounter << ".jpg";
    std::string filename = ss.str();

    // Increment the screenshotCounter for the next screenshot
    screenshotCounter++;

    // Save the screenshot as a JPG image
    stbi_flip_vertically_on_write(1); // Flip the image vertically (OpenGL's origin is bottom-left)
    stbi_write_jpg(filename.c_str(), width, height, 3, pixels.data(), 100); // Quality: 100 (highest)

    std::cout << "Screenshot saved as " << filename << std::endl;
    
    if (screenshotCounter == 11) {
        if (lightCounter == 10) exit(0);
        lightCounter++;
        screenshotCounter = 1;
    }

}


unsigned int sphereVAO = 0;
unsigned int sphereVBO = 0;
const int sphereSlices = 30;
const int sphereStacks = 30;

void renderSphere() {
    float radius = 1.0f;
    if (sphereVAO == 0) {
        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glBindVertexArray(sphereVAO);

        std::vector<float> vertices;
        for (int stack = 0; stack <= sphereStacks; ++stack) {
            float phi = static_cast<float>(M_PI) * stack / sphereStacks;
            float phiNext = static_cast<float>(M_PI) * (stack + 1) / sphereStacks;

            for (int slice = 0; slice <= sphereSlices; ++slice) {
                float theta = static_cast<float>(2.0 * M_PI) * slice / sphereSlices;

                // Calculate vertex positions
                float x = radius * cos(theta) * sin(phi);
                float y = radius * sin(theta) * sin(phi);
                float z = radius * cos(phi);

                // Calculate normals
                float nx = cos(theta) * sin(phi);
                float ny = sin(theta) * sin(phi);
                float nz = cos(phi);

                // Calculate texture coordinates
                float s = static_cast<float>(slice) / sphereSlices;
                float t = static_cast<float>(stack) / sphereStacks;

                // Add the vertex data to the array
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);
                vertices.push_back(s);
                vertices.push_back(t);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Vertex position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        // Normal ( 빛 계산에 필요 )
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        // Texture coordinates (사용 안함)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);
    }

    // Render the sphere
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (sphereSlices + 1) * (sphereStacks + 1) * 2);
    glBindVertexArray(0);
}




unsigned int planeVAO = 0;
unsigned int planeVBO = 0;

void renderTriangle() {
    if (planeVAO == 0) {
        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);

        glBindVertexArray(planeVAO);

        // Vertices of the triangle plane
        float vertices[] = {
            // Position           // Normal         // Texture coordinates
            -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
            0.0f, 0.0f, 0.5f,    0.0f, 1.0f, 0.0f,  0.0f, 1.0f
        };

        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Vertex position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        // Texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);
    }

    // Render the triangle plane
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}




unsigned int coneVAO = 0;
unsigned int coneVBO = 0;
const int coneSegments = 30;
const float coneHeight = 1.0f;
const float coneRadius = 0.5f;

void renderCone() {
    if (coneVAO == 0) {
        glGenVertexArrays(1, &coneVAO);
        glGenBuffers(1, &coneVBO);

        glBindVertexArray(coneVAO);

        std::vector<float> vertices;

        // Base circle
        for (int i = 0; i < coneSegments; ++i) {
            float theta = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(coneSegments);
            float x = coneRadius * cos(theta);
            float z = coneRadius * sin(theta);
            float nx = 0.0f;
            float ny = -1.0f;
            float nz = 0.0f;
            float s = x / coneRadius + 0.5f;
            float t = z / coneRadius + 0.5f;

            vertices.push_back(x);
            vertices.push_back(-coneHeight / 2.0f);
            vertices.push_back(z);
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
            vertices.push_back(s);
            vertices.push_back(t);
        }

        // Apex
        vertices.push_back(0.0f);
        vertices.push_back(coneHeight / 2.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.5f);
        vertices.push_back(0.5f);

        // Side triangles
        for (int i = 0; i < coneSegments; ++i) {
            int nextIndex = (i + 1) % coneSegments;

            // Bottom vertex
            vertices.push_back(vertices[i * 8]);
            vertices.push_back(vertices[i * 8 + 1]);
            vertices.push_back(vertices[i * 8 + 2]);
            // Normal
            vertices.push_back(vertices[i * 8]);
            vertices.push_back(vertices[i * 8 + 1]);
            vertices.push_back(vertices[i * 8 + 2]);
            // Texture coordinates
            vertices.push_back(0.5f + 0.5f * vertices[i * 8] / coneRadius);
            vertices.push_back(0.5f + 0.5f * vertices[i * 8 + 2] / coneRadius);

            // Top vertex
            vertices.push_back(vertices[nextIndex * 8]);
            vertices.push_back(vertices[nextIndex * 8 + 1]);
            vertices.push_back(vertices[nextIndex * 8 + 2]);
            // Normal
            vertices.push_back(vertices[nextIndex * 8]);
            vertices.push_back(vertices[nextIndex * 8 + 1]);
            vertices.push_back(vertices[nextIndex * 8 + 2]);
            // Texture coordinates
            vertices.push_back(0.5f + 0.5f * vertices[nextIndex * 8] / coneRadius);
            vertices.push_back(0.5f + 0.5f * vertices[nextIndex * 8 + 2] / coneRadius);

            // Apex
            vertices.push_back(0.0f);
            vertices.push_back(coneHeight / 2.0f);
            vertices.push_back(0.0f);
            // Normal
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
            // Texture coordinates
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);
        }

        glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Vertex position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        // Texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);
    }

    // Render the cone
    glBindVertexArray(coneVAO);
    glDrawArrays(GL_TRIANGLES, 0, coneSegments * 3);
    glBindVertexArray(0);
}




unsigned int triangularPrismVAO = 0;
unsigned int triangularPrismVBO = 0;

void renderTriangularPrism() {
    if (triangularPrismVAO == 0) {
        glGenVertexArrays(1, &triangularPrismVAO);
        glGenBuffers(1, &triangularPrismVBO);

        glBindVertexArray(triangularPrismVAO);

        std::vector<float> vertices;

        // Vertices of the triangular prism
        float halfBaseWidth = 0.5f;
        float halfBaseLength = 0.5f;
        float height = 1.0f;

        // Base triangle
        vertices.push_back(-halfBaseWidth);  // Vertex 1
        vertices.push_back(0.0f);
        vertices.push_back(-halfBaseLength);
        vertices.push_back(0.0f);  // Normal
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);  // Texture coordinates
        vertices.push_back(0.5f);
        vertices.push_back(0.5f);

        vertices.push_back(halfBaseWidth);   // Vertex 2
        vertices.push_back(0.0f);
        vertices.push_back(-halfBaseLength);
        vertices.push_back(0.0f);  // Normal
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);  // Texture coordinates
        vertices.push_back(1.0f);
        vertices.push_back(0.5f);

        vertices.push_back(0.0f);            // Vertex 3 (Top of the base)
        vertices.push_back(0.0f);
        vertices.push_back(halfBaseLength);
        vertices.push_back(0.0f);  // Normal
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.5f);  // Texture coordinates
        vertices.push_back(0.5f);
        vertices.push_back(0.0f);

        // Top triangle
        vertices.push_back(0.0f);            // Vertex 4 (Apex)
        vertices.push_back(height);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);  // Normal
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.5f);  // Texture coordinates
        vertices.push_back(0.5f);
        vertices.push_back(0.5f);

        vertices.push_back(-halfBaseWidth);  // Vertex 5
        vertices.push_back(0.0f);
        vertices.push_back(-halfBaseLength);
        vertices.push_back(0.0f);  // Normal
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);  // Texture coordinates
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);

        vertices.push_back(halfBaseWidth);   // Vertex 6
        vertices.push_back(0.0f);
        vertices.push_back(-halfBaseLength);
        vertices.push_back(0.0f);  // Normal
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);  // Texture coordinates
        vertices.push_back(1.0f);
        vertices.push_back(1.0f);

        // Side triangles
        for (int i = 0; i < 3; ++i) {
            int nextIndex = (i + 1) % 3;

            // Bottom vertex
            vertices.push_back(vertices[i * 9]);
            vertices.push_back(vertices[i * 9 + 1]);
            vertices.push_back(vertices[i * 9 + 2]);
            // Normal
            vertices.push_back(0.0f);
            vertices.push_back(-1.0f);
            vertices.push_back(0.0f);
            // Texture coordinates
            vertices.push_back(0.5f + vertices[i * 9] / halfBaseWidth * 0.5f);
            vertices.push_back(0.5f + vertices[i * 9 + 2] / halfBaseLength * 0.5f);

            // Top vertex
            vertices.push_back(vertices[nextIndex * 9 + 3]);  // Reuse the apex vertex coordinates
            vertices.push_back(vertices[nextIndex * 9 + 4]);
            vertices.push_back(vertices[nextIndex * 9 + 5]);
            // Normal
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
            // Texture coordinates
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);
        }

        glBindBuffer(GL_ARRAY_BUFFER, triangularPrismVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Vertex position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
        // Texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);
    }

    // Render the triangular prism
    glBindVertexArray(triangularPrismVAO);
    glDrawArrays(GL_TRIANGLES, 0, 15);
    glBindVertexArray(0);
}
