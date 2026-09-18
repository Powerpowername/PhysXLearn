#include "windowConfig.hpp"
#include "shader.hpp"
#include "Reneder.hpp"

#include <cstdint>
#include <vector>

int main()
{
    OpenGLCognfig config("Joint - 5 cubes / 5 spheres / 1 plane");

    // 用绝对路径，免得受运行目录影响
    Shader shader("D:/Repos/Physx/LearnPro/Joint/shader/instance.vert",
                  "D:/Repos/Physx/LearnPro/Joint/shader/instance.frag");

    PhysXLearner::CubeRenderOperator   cubes;
    PhysXLearner::SphereRenderOperator spheres;
    PhysXLearner::PlaneRenderOperator  ground;

    // 立方体和球各一排，都坐在地面上（半棱长和半径都是 0.5，所以 y = 0.5）
    std::vector<glm::mat4> cubeTransforms;
    std::vector<glm::mat4> sphereTransforms;
    for (int i = 0; i < 5; ++i)
    {
        const float x = (static_cast<float>(i) - 2.0f) * 1.6f;
        cubeTransforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.5f, -1.6f)));
        sphereTransforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.5f, 1.6f)));
    }
    cubes.setInstances(cubeTransforms);
    spheres.setInstances(sphereTransforms);

    // 地面就是那一个 plane，放大 20 倍（y 方向不缩放）
    std::vector<glm::mat4> groundTransforms = {
        glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 1.0f, 20.0f))};
    ground.setInstances(groundTransforms);

    auto& camera = config.getCamera();
    camera->setPosition(glm::vec3(0.0f, 6.0f, 12.0f));
    camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    uint32_t width = 0;
    uint32_t height = 0;
    config.getSCSize(width, height);

    glEnable(GL_DEPTH_TEST);                      // windowConfig.hpp 里这句是注释掉的，不打开会前后穿透
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    // 只画网格线，不填充

    GLFWwindow* window = config.getGLFWwindowPoint();
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4("uViewProjection",
                       camera->getProjectionMatrix(static_cast<float>(width) / static_cast<float>(height))
                           * camera->getViewMatrix());

        shader.setVec3("uColor", 0.95f, 0.55f, 0.20f);
        cubes.drawInstanced();

        shader.setVec3("uColor", 0.35f, 0.70f, 1.00f);
        spheres.drawInstanced();

        shader.setVec3("uColor", 0.45f, 0.45f, 0.50f);
        ground.drawInstanced();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
