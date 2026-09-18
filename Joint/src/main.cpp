#include "windowConfig.hpp"
#include "shader.hpp"
#include "Reneder.hpp"
#include "PhysXConfig.hpp"
#include "auxi.hpp"

#include <cstdint>
#include <vector>

int main()
{
    OpenGLCognfig config("Joint - PhysX scene");

    // 用绝对路径，免得受运行目录影响
    Shader shader("D:/Repos/Physx/LearnPro/Joint/shader/instance.vert",
                  "D:/Repos/Physx/LearnPro/Joint/shader/instance.frag");

    PhysXLearner::CubeRenderOperator   cubes;
    PhysXLearner::SphereRenderOperator spheres;
    PhysXLearner::PlaneRenderOperator  ground;

    // 场景内容全部由 PhysXWorld::Init() 自己建，这里只管取
    if (!PhysXLearner::PhysXWorld::Init().value_or(false))
    {
        return 1;
    }
    PxScene* scene = PhysXLearner::PhysXWorld::getScene();

    auto& camera = config.getCamera();
    camera->setPosition(glm::vec3(10.0f, 24.0f, 45.0f));
    camera->lookAt(glm::vec3(10.0f, 12.0f, 0.0f));

    uint32_t width = 0;
    uint32_t height = 0;
    config.getSCSize(width, height);

    glEnable(GL_DEPTH_TEST);                      // windowConfig.hpp 里这句是注释掉的，不打开会前后穿透
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    // 只画网格线，不填充

    std::vector<glm::mat4> cubeTransforms;
    std::vector<glm::mat4> sphereTransforms;
    std::vector<glm::mat4> groundTransforms;
    std::vector<PxActor*>  actors;
    std::vector<PxShape*>  shapes;
    std::vector<PxShape*>  boxShapes;
    std::vector<PxShape*>  sphereShapes;
    std::vector<PxShape*>  planeShapes;

    GLFWwindow* window = config.getGLFWwindowPoint();
    while (!glfwWindowShouldClose(window))
    {
        const double frameStart = glfwGetTime();

        PhysXLearner::PhysXWorld::stepPhysics();

        // ---- 把场景里所有 shape 按几何类型分到三个桶 ----
        boxShapes.clear();
        sphereShapes.clear();
        planeShapes.clear();

        const PxActorTypeFlags types = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
        actors.resize(scene->getNbActors(types));
        scene->getActors(types, actors.data(), static_cast<PxU32>(actors.size()));

        for (PxActor* actor : actors)
        {
            // getActors 按这两种类型过滤，拿回来的必定是刚体
            PxRigidActor* rigid = static_cast<PxRigidActor*>(actor);

            shapes.resize(rigid->getNbShapes());
            rigid->getShapes(shapes.data(), static_cast<PxU32>(shapes.size()));

            for (PxShape* shape : shapes)
            {
                switch (shape->getGeometry().getType())
                {
                case PxGeometryType::eBOX:    boxShapes.push_back(shape);    break;
                case PxGeometryType::eSPHERE: sphereShapes.push_back(shape); break;
                case PxGeometryType::ePLANE:  planeShapes.push_back(shape);  break;
                default: break;
                }
            }
        }

        // ---- 变换全部交给 PhysXWorld 算 ----
        PhysXLearner::PhysXWorld::getShapeTransforms(cubeTransforms, boxShapes);
        PhysXLearner::PhysXWorld::getShapeTransforms(sphereTransforms, sphereShapes);
        PhysXLearner::PhysXWorld::getShapeTransforms(groundTransforms, planeShapes);

        cubes.setInstances(cubeTransforms);
        spheres.setInstances(sphereTransforms);
        ground.setInstances(groundTransforms);

        // ---- 画 ----
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4("uViewProjection",
                       camera->getProjectionMatrix(static_cast<float>(width) / static_cast<float>(height))
                           * camera->getViewMatrix());

        shader.setVec3("uColor", 0.35f, 0.70f, 1.00f);
        spheres.drawInstanced();

        shader.setVec3("uColor", 0.95f, 0.55f, 0.20f);
        cubes.drawInstanced();

        shader.setVec3("uColor", 0.45f, 0.45f, 0.50f);
        ground.drawInstanced();

        glfwSwapBuffers(window);
        glfwPollEvents();

        // ---- 帧尾等待：把本帧补足到 1/60 秒 ----
        // 物理是固定 1/60 秒一步。不压住渲染循环的话，循环跑多快仿真就跑多快，
        // 物理时间会远远快于真实时间。
        // 这里刻意用忙等而不是 glfwWaitEventsTimeout/sleep_for：
        // Windows 的等待函数精度只有系统时钟节拍级（约 15.6 ms），且向上取整，
        // 实测单用它每帧会变成约 22 ms（≈45 fps），比不等还偏。
        constexpr double frameTarget = 1.0 / 60.0;
        while (glfwGetTime() - frameStart < frameTarget)
        {
        }
    }

    PhysXLearner::PhysXWorld::cleanupPhysics();
    return 0;
}
