// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2022 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glm/gtx/transform.hpp>
#include "glm/gtc/matrix_transform.hpp"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Sphere.h"

#include <filesystem>

// constants
const static int width = 1024;
const static int height = 768;
const static float kSizeSun = 2;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 15;
const static float kRadOrbitMoon = 2;

const static float kSizeMercury = 0.3;
const static float kSizeVenus = 0.49;
const static float kSizeMars = 0.35;
const static float kRadOrbitMercury = 5;
const static float kRadOrbitVenus = 10;
const static float kRadOrbitMars = 20;

const static float kSizeJupiter = 1;
const static float kSizeSaturn = 1;
const static float kSizeUran = 1;
const static float kRadOrbitJupiter = 30;
const static float kRadOrbitSaturn = 40;
const static float kRadOrbitUran = 50;

std::vector<Sphere* > solarS;
glm::vec4 centerearth = glm::vec4(10.0f, 0.0f, 0.0f, 1.0f);
glm::vec4 centermoon = glm::vec4(10.0f, 0.0f, 0.0f, 1.0f);

// Window parameters
GLFWwindow* g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_ibo = 0;

GLuint c_vao = 0;
GLuint c_posVbo = 0;

// All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
std::vector<float> g_vertexColors;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;

// Basic camera model
class Camera {
public:
    inline float getFov() const { return m_fov; }
    inline void setFoV(const float f) { m_fov = f; }
    inline float getAspectRatio() const { return m_aspectRatio; }
    inline void setAspectRatio(const float a) { m_aspectRatio = a; }
    inline float getNear() const { return m_near; }
    inline void setNear(const float n) { m_near = n; }
    inline float getFar() const { return m_far; }
    inline void setFar(const float n) { m_far = n; }
    inline void setPosition(const glm::vec3& p) { m_pos = p; }
    inline glm::vec3 getPosition() { return m_pos; }
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    float speed = 0.005f;
    float sensitivity = 1.0f;
    bool firsClick = true;;

    inline glm::mat4 computeViewMatrix() const {
        return glm::lookAt(m_pos, m_pos + Orientation, Up);
    }

    // Returns the projection matrix stemming from the camera intrinsic parameter.
    inline glm::mat4 computeProjectionMatrix() const {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
    }
 

private:
    glm::vec3 m_pos = glm::vec3(0, 0, 0);
    float m_fov = 45.f;        // Field of view, in degrees
    float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
    float m_near = 10.f; // Distance before which geometry is excluded from the rasterization process
    float m_far = 100.f; // Distance after which the geometry is excluded from the rasterization process
};
Camera g_camera;

void Inputs(GLFWwindow* window)
{
        int nwidth, nheight;
        glfwGetWindowSize(window, &nwidth, &nheight);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            g_camera.setPosition(g_camera.getPosition() + g_camera.speed * g_camera.Orientation);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            g_camera.setPosition(g_camera.getPosition() + g_camera.speed * -g_camera.Orientation);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            g_camera.setPosition(g_camera.getPosition() + g_camera.speed * glm::normalize(glm::cross(g_camera.Orientation, g_camera.Up)));
            
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            g_camera.setPosition(g_camera.getPosition() + g_camera.speed * -glm::normalize(glm::cross(g_camera.Orientation, g_camera.Up)));
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            g_camera.setPosition(g_camera.getPosition() + g_camera.speed * g_camera.Up);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            g_camera.setPosition(g_camera.getPosition() + g_camera.speed * -g_camera.Up);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            g_camera.speed = 0.01;
        }
        else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
            g_camera.speed = 0.005;
        }

        // Change vision direction
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (g_camera.firsClick) {
                glfwSetCursorPos(window, nwidth / 2, nheight / 2);
                g_camera.firsClick = false;
            }
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            double mouseX;
            double mouseY;

            glfwGetCursorPos(window, &mouseX, &mouseY);

            float rotX = g_camera.sensitivity * (float)(mouseX - (nwidth / 2)) / nwidth;
            float rotY = g_camera.sensitivity * (float)(mouseY - (nheight / 2)) / nheight;

            std::cout << "The text is: " << rotX << " " << rotY << std::endl;

            glm::vec3 right = glm::normalize(glm::cross(g_camera.Orientation, g_camera.Up));

            glm::vec3 newOrientation = glm::normalize(glm::rotate(
                glm::mat4(1.0f), glm::radians((float)-rotY), right) *
                glm::vec4(g_camera.Orientation, 0.0f)
            );
            // Calculate the dot product
            float dotProduct1 = glm::dot(newOrientation, g_camera.Up);
            float dotProduct2 = glm::dot(newOrientation, -g_camera.Up);

            // Calculate the magnitude (length) of both vectors
            float magnitudeV1 = glm::length(newOrientation);
            float magnitudeV2 = glm::length(g_camera.Up);

            // Calculate the angle in radians
            float angle1 = glm::acos(dotProduct1 / (magnitudeV1 * magnitudeV2));
            float angle2 = glm::acos(dotProduct2 / (magnitudeV1 * magnitudeV2));
            if (!(angle1 <= glm::radians(5.0f) || angle2 <= glm::radians(5.0f))) {
                g_camera.Orientation = newOrientation;
            }

            g_camera.Orientation = glm::normalize(glm::rotate(
                glm::mat4(1.0f), glm::radians((float)-rotX), g_camera.Up) *
                glm::vec4(g_camera.Orientation, 0.0f)
            );

        }

        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            g_camera.firsClick = true;
        }

}

GLuint loadTextureFromFileToGPU(const std::string& filename) {
    // Loading the image in CPU memory using stb_image
    int width, height, numComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &numComponents, 0);
    GLuint texID = 0; // OpenGL texture identifier
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texID); // generate an OpenGL texture container
    glBindTexture(GL_TEXTURE_2D, texID); // activate the texture
    // Setup the texture filtering option and repeat mode; check www.opengl.org for details.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Fill the GPU texture with the data stored in the CPU image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    // Free useless CPU memory
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture
    return texID;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_Z) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else if (action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
        glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
    }
}

void errorCallback(int error, const char* desc) {
    std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW, the library responsible for window management
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Before creating the window, set some option flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create the window
    g_window = glfwCreateWindow(
        width, height,
        "Interactive 3D Applications (OpenGL) - Simple Solar System",
        nullptr, nullptr);
    if (!g_window) {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, windowSizeCallback);
    glfwSetKeyCallback(g_window, keyCallback);
}

void initOpenGL() {
    // Load extensions for modern OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
    glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string& filename) {
    std::ifstream t(filename.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string& shaderFilename) {
    GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
    std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
    const GLchar* shaderSource = (const GLchar*)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
    glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
    glCompileShader(shader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
    }
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

void initGPUprogram() {
    g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
    loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl");
    loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
    glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

    glUseProgram(g_program);
    // TODO: set shader variables, textures, etc.
}

// Define your mesh(es) in the CPU memory
void initCPUgeometry() {
    // TODO: add vertices and indices for your mesh(es)
    g_vertexPositions = { 0.f, 0.f, 0.f,
                          1.f, 0.f, 0.f,
                          0.f, 1.f, 0.f,
                          1.f, 1.f, 1.f };

    g_triangleIndices = { 0,1,2,0,1,3 };


}

void initGPUgeometry() {
    // Create a single handle, vertex array object that contains attributes,
    // vertex buffer objects (e.g., vertex's position, normal, and color)
#ifdef _MY_OPENGL_IS_33_
    glGenVertexArrays(1, &g_vao);
#else
    glCreateVertexArrays(1, &g_vao);
#endif
    glBindVertexArray(g_vao);

    // Generate a GPU buffer to store the positions of the vertices
    size_t vertexBufferSize = sizeof(float) * g_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
    //size_t colorBufferSize = sizeof(float) * g_vertexColors.size();
#ifdef _MY_OPENGL_IS_33_
    glGenBuffers(1, &g_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
#else
    glCreateBuffers(1, &sphere->m_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, sphere->m_posVbo);
    glNamedBufferStorage(g_posVbo, vertexBufferSize, sphere->getVector().data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
#endif


    // Same for an index buffer object that stores the list of indices of the
    // triangles forming the mesh
    size_t indexBufferSize = sizeof(unsigned int) * g_triangleIndices.size();
#ifdef _MY_OPENGL_IS_33_
    glGenBuffers(1, &g_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_READ);
#else
    glCreateBuffers(1, &sphere->m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->m_ibo);
    glNamedBufferStorage(sphere->m_ibo, indexBufferSize, sphere->m_triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
#endif


    glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
}

void initCamera() {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.setPosition(glm::vec3(5.0, 2, 20));
    g_camera.setNear(0.1);
    g_camera.setFar(500.1);
}

void init() {
    initGLFW();
    initOpenGL();
    for (Sphere* s : solarS) {
        s->init();
        s->initGPUgeometry();
    }
    initGPUprogram();
    initCamera();
}

void clear() {
    glDeleteProgram(g_program);

    glfwDestroyWindow(g_window);
    glfwTerminate();
}

// The main rendering call
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
    glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program

    float currentTime = glfwGetTime();
    glm::vec4 centerearth = glm::vec4(10.0f, 0.0f, 0.0f, 1.0f);
    float rotationangleearth;

    for (Sphere* s : solarS) {
        glm::mat4 modelMatrix(1.f);
        float angspeedO = 360.f / s->getPeriodO();
        float angspeedR = 360.f / s->getPeriodR();

        float orbitang = angspeedO * currentTime;//calculate angle that the planet made
        float rotationang = angspeedR * currentTime; //calculate angle that the planet made

        if (s->getName() == "earth") {
            //orbit
            centerearth = s->getPosition();
            glm::mat4 orbitMat = glm::rotate(glm::radians(orbitang), glm::vec3(0.0f, 1.0f, 0.0f));
            centerearth = orbitMat * centerearth; //move center to new place
            modelMatrix = glm::translate(glm::vec3(centerearth));

            //angle of earth
            glm::mat4 angleMat = glm::rotate(glm::radians(-23.5f), glm::vec3(1, 0, 0));
            modelMatrix = modelMatrix * angleMat;
            //rotation 

            glm::mat4 rotateMat = glm::rotate(glm::radians(rotationang), glm::vec3(0.0f, 1.0f, 0.0f));
            rotateMat = rotateMat * glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            modelMatrix = modelMatrix * rotateMat;
        }

        else if (s->getName() == "venus") {
            //orbit
            centerearth = s->getPosition();
            glm::mat4 orbitMat = glm::rotate(glm::radians(orbitang), glm::vec3(0.0f, 1.0f, 0.0f));
            centerearth = orbitMat * centerearth; //move center to new place
            modelMatrix = glm::translate(glm::vec3(centerearth));

            //angle of earth
            glm::mat4 angleMat = glm::rotate(glm::radians(-23.5f), glm::vec3(1, 0, 0));
            modelMatrix = modelMatrix * angleMat;
            //rotation 

            glm::mat4 rotateMat = glm::rotate(glm::radians(-rotationang), glm::vec3(0.0f, 1.0f, 0.0f));
            rotateMat = rotateMat * glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            modelMatrix = modelMatrix * rotateMat;
        }

        else if (s->getName() == "moon") {
            //orbit
            glm::vec4 center = s->getPosition();

            glm::mat4 orbitMat = glm::rotate(glm::radians(orbitang), glm::vec3(0.0f, 1.0f, 0.0f));
            center = orbitMat * center; //move center to new place

            modelMatrix = glm::translate(glm::vec3(center) + glm::vec3(centerearth));

            //rotate

            glm::mat4 rotateMat = glm::rotate(glm::radians(rotationang), glm::vec3(0.0f, 1.0f, 0.0f));
            rotateMat = rotateMat * glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            modelMatrix = modelMatrix * rotateMat;



        }
        glm::mat4 transformationMatrix = projMatrix * viewMatrix * modelMatrix;
        glUniform1i(glGetUniformLocation(g_program, "test"), 1);
        s->render(g_program, modelMatrix, viewMatrix, transformationMatrix);
    }



    const glm::vec3 camPosition = g_camera.getPosition();
    glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);


}

#include <direct.h>
#define GetCurrentDir _getcwd

int main(int argc, char** argv) {
    // Create sun earth and moon
    Sphere* sun = new Sphere(kSizeSun, 0, 0, 0);
    Sphere* earth = new Sphere(kSizeEarth * 2, kRadOrbitEarth, 0, 0);
    Sphere* moon = new Sphere(kSizeMoon, (kRadOrbitMoon), 0, 0);
    Sphere* mercury = new Sphere(kSizeMercury, (kRadOrbitMercury), 0, 0);
    Sphere* venus = new Sphere(kSizeVenus, (kRadOrbitVenus), 0, 0);
    Sphere* mars = new Sphere(kSizeMars, (kRadOrbitMars), 0, 0);
    Sphere* jup = new Sphere(kSizeJupiter, (kRadOrbitJupiter), 0, 0);
    Sphere* sat = new Sphere(kSizeSaturn, (kRadOrbitSaturn), 0, 0);
    Sphere* uran = new Sphere(kSizeUran, (kRadOrbitUran), 0, 0);

    float opt, rpt, opm, rpm;

    rpt = 10;
    opt = 2 * rpt;
    opm = rpt / 2;
    rpm = opm;

    // Give them the name and period
    sun->setName("sun");
    moon->setName("moon");
    venus->setName("venus");

    earth->setPeriode(rpt, opt);
    moon->setPeriode(opm, rpm);
    mercury->setPeriode(1.7* rpt, 0.39* opt); 
    venus->setPeriode(243 * rpt, 0.72 * opt);
    mars->setPeriode(1.03 * rpt, 1.52 * opt);
    jup->setPeriode(11.86 * rpt, 5.20 * opt);
    sat->setPeriode(29.46 * rpt, 9.58 * opt);
    uran->setPeriode(84.01 * rpt, 19.22 * opt);


    // add them to our vector
    solarS.push_back(sun);
    solarS.push_back(earth);
    solarS.push_back(moon);
    solarS.push_back(mercury);
    solarS.push_back(venus);
    solarS.push_back(mars);
    solarS.push_back(jup);
    solarS.push_back(sat);
    solarS.push_back(uran);

    init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
    //loadtextures and add to the planets
    GLint g_sunTexID = loadTextureFromFileToGPU("media/sun.png");
    GLint g_earthTexID = loadTextureFromFileToGPU("media/earth.jpg");
    GLint g_moonTexID = loadTextureFromFileToGPU("media/moon.jpg");
    GLint g_mercTexID = loadTextureFromFileToGPU("media/mercury.jpg");
    GLint g_venTexID = loadTextureFromFileToGPU("media/venus.jpg");
    GLint g_marsTexID = loadTextureFromFileToGPU("media/mars.jpg");
    GLint g_jupTexID = loadTextureFromFileToGPU("media/jupiter.jpg");
    GLint g_satTexID = loadTextureFromFileToGPU("media/saturn.jpg");
    GLint g_uranTexID = loadTextureFromFileToGPU("media/uranus.jpg");

    sun->setTex(g_sunTexID);
    earth->setTex(g_earthTexID);
    moon->setTex(g_moonTexID);
    mercury->setTex(g_mercTexID);
    venus->setTex(g_venTexID);
    mars->setTex(g_marsTexID);
    jup->setTex(g_jupTexID);
    sat->setTex(g_satTexID);
    uran->setTex(g_uranTexID);
    while (!glfwWindowShouldClose(g_window)) {
        render();
        glfwSwapBuffers(g_window);
        Inputs(g_window);
        glfwPollEvents();
    }
    clear();
    return EXIT_SUCCESS;
}
