#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <cstdint>
#include <memory>
#include <iostream>
#include <string>
#include "glm/glm.hpp"
#include "camera.hpp"
#include <array>
using namespace std;
using namespace glm;
 
// void processInput(GLFWwindow *window);

class OpenGLCognfig
{
private:
    inline static uint32_t scrWidth = 800;
    inline static uint32_t scrHeight = 600;
    inline static GLFWwindow* window = nullptr;
    inline static shared_ptr<Camera> gCamera;
    inline static bool firstMouse;
    inline static float lastX;
    inline static float lastY;
private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        if (!gCamera) return;

        if (firstMouse)
        {
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
            firstMouse = false;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
        {
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
            return;
        }

        float xoffset = static_cast<float>(xpos) - lastX;
        float yoffset = lastY - static_cast<float>(ypos);
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);

        gCamera->processMouse(xoffset, yoffset);
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        if (!gCamera) return;
        gCamera->processScroll(static_cast<float>(yoffset));
    }
public:
    OpenGLCognfig(string windowName = "PhysX")
    {
        gCamera = make_shared<Camera>(glm::vec3(0.0f, 4.0f, 50.0f));
        gCamera->lookAt(glm::vec3(0.0f, 0.0f, -10.0f));
        firstMouse = true;
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(scrWidth, scrHeight, windowName.c_str(), NULL, NULL);
    
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        // tell GLFW to capture our mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::printf("Failed to initialize GLAD\n");
            return;
        }
        glfwSwapInterval(0);
        // // configure global opengl state
        // // -----------------------------
        // glEnable(GL_DEPTH_TEST);
    }

    void getSCSize(uint32_t& scrWidth,uint32_t& scrHeight)
    {
        scrWidth = this->scrWidth;
        scrHeight = this->scrHeight;
    };
    GLFWwindow* getGLFWwindowPoint() const {return window;}
    
    shared_ptr<Camera>& getCamera() const {return gCamera;}
};

