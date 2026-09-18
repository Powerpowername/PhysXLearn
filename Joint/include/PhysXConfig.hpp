#include "PxPhysicsAPI.h"
#define PVD_HOST "127.0.0.1"	//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.
#include <optional>
#include <functional>
#include <vector>
#include "glm/glm.hpp"
using namespace physx;
using namespace std;
namespace PhysXLearner
{
using JointCreateFunction = std::function<optional<PxJoint*>(PxRigidActor* a0, const PxTransform& t0, PxRigidActor* a1, const PxTransform& t1)>;
class PhysXWorld
{
public:
    static optional<bool> Init();
    static void stepPhysics();
    static void cleanupPhysics();
    static void createActorCallback(vector<function<void()>> callbacks);

    static void createChain(const PxTransform& t, PxU32 length, const PxGeometry& g, PxReal separation, JointCreateFunction createJoint);

    // 渲染要取场景里每个 shape 的变换，所以把场景句柄暴露出来
    static PxScene* getScene();
    // 把一批 shape 的世界变换算成渲染器能直接用的模型矩阵，写进 transforms
    static void getShapeTransforms(std::vector<glm::mat4>& transforms, std::vector<PxShape*> shapes);


private:

    static optional<PxRigidDynamic*> createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity=PxVec3(0));
    static optional<PxJoint*> createLimitedSphericalJoint(PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1);





};


}
