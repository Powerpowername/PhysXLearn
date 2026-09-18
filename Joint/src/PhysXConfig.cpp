#include "PhysXConfig.hpp"
#include <thread>
#include <iostream>
#include "auxi.hpp"
#include "glm/gtc/matrix_transform.hpp"
namespace PhysXLearner
{
static PxDefaultAllocator		gAllocator;
static PxDefaultErrorCallback	gErrorCallback;
static PxFoundation*			gFoundation = NULL;
static PxPhysics*				gPhysics	= NULL;
static PxDefaultCpuDispatcher*	gDispatcher = NULL;
static PxScene*					gScene		= NULL;
static PxMaterial*				gMaterial	= NULL;
static PxPvd*					gPvd        = NULL;

optional<bool> PhysXWorld::Init()
{
    try{
        gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
        gPvd = PxCreatePvd(*gFoundation);
        PxPvdTransport *transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
        gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

        gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
        PxInitExtensions(*gPhysics, gPvd);

        PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
        sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
        gDispatcher = PxDefaultCpuDispatcherCreate(thread::hardware_concurrency());
        sceneDesc.cpuDispatcher = gDispatcher;
        sceneDesc.filterShader = PxDefaultSimulationFilterShader;
        gScene = gPhysics->createScene(sceneDesc);

        PxPvdSceneClient *pvdClient = gScene->getScenePvdClient();
        if (pvdClient)
        {
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
        gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    	
        PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0,1,0,0), *gMaterial);
	    gScene->addActor(*groundPlane);
	    createChain(PxTransform(PxVec3(0.0f, 20.0f, 0.0f)), 5, PxBoxGeometry(2.0f, 0.5f, 0.5f), 4.0f, createLimitedSphericalJoint);
    
    
    
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception during PhysX initialization: " << e.what() << std::endl;
        return nullopt; // Return an empty optional to indicate failure
    }
    return true;
}

void PhysXWorld::stepPhysics()
{
	gScene->simulate(1.0f/60.0f);
	gScene->fetchResults(true);
}

PxScene* PhysXWorld::getScene()
{
	return gScene;
}

// 把一批 shape 的世界变换算成渲染器能直接用的模型矩阵。
// 直接喂给渲染器就够了：网格都是单位尺寸，所以这里顺手把各自几何体的实际尺寸也乘进去。
void PhysXWorld::getShapeTransforms(std::vector<glm::mat4>& transforms, std::vector<PxShape*> shapes)
{
	transforms.clear();
	transforms.reserve(shapes.size());

	for(PxShape* shape : shapes)
	{
		PxRigidActor* actor = shape->getActor();
		if(!actor)
		{
			continue;	// 共享 shape 不属于任何 actor，拿不到位姿
		}

		// 完整世界位姿 = 刚体的全局位姿 * shape 相对刚体的局部位姿
		const glm::mat4 world = Learner::toGlm(actor->getGlobalPose() * shape->getLocalPose());

		const PxGeometry& geometry = shape->getGeometry();

		if(geometry.getType() == PxGeometryType::eBOX)
		{
			const PxBoxGeometry& box = static_cast<const PxBoxGeometry&>(geometry);
			// 网格是边长 1 的立方体，所以要缩放「半尺寸的两倍」
			transforms.push_back(world * glm::scale(glm::mat4(1.0f),
				glm::vec3(box.halfExtents.x * 2.0f, box.halfExtents.y * 2.0f, box.halfExtents.z * 2.0f)));
		}
		else if(geometry.getType() == PxGeometryType::eSPHERE)
		{
			const PxSphereGeometry& sphere = static_cast<const PxSphereGeometry&>(geometry);
			// 网格是半径 0.5 的球，所以要缩放「半径的两倍」
			transforms.push_back(world * glm::scale(glm::mat4(1.0f), glm::vec3(sphere.radius * 2.0f)));
		}
		else if(geometry.getType() == PxGeometryType::ePLANE)
		{
			// PhysX 平面的法线是 shape 局部 +X，而网格是 XZ 平面、法线 +Y，所以要绕 Z 转 -90° 把两者对上。
			// 另外 PhysX 的 plane 是无限平面，这里拿单位正方形放大 60 倍当作有限地面画。
			transforms.push_back(world
				* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(60.0f, 1.0f, 60.0f)));
		}
		// 其余几何类型没有对应网格，直接跳过
	}
}
void PhysXWorld::cleanupPhysics()
{
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PxCloseExtensions();
	PX_RELEASE(gPhysics);
	if(gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		PX_RELEASE(gPvd);
		PX_RELEASE(transport);
	}
	PX_RELEASE(gFoundation);
	
	printf("SnippetJoint done.\n");
}

void PhysXWorld::createActorCallback(vector<function<void()>> callbacks)
{
    for(auto& callback : callbacks)
    {
        callback();
    }
}

void PhysXWorld::createChain(const PxTransform &t, PxU32 length, const PxGeometry &g, PxReal separation, JointCreateFunction createJoint)
{
	PxVec3 offset(separation/2, 0, 0);
	PxTransform localTm(offset);
	PxRigidDynamic* prev = NULL;

	for(PxU32 i=0;i<length;i++)
	{
        PxRigidDynamic* current = PxCreateDynamic(*gPhysics,t*localTm,g,*gMaterial,10.0f);
        // 如果actor传入的是NULL，那么就从“相对于刚体局部坐标系”的变换，变成了“在世界坐标系中”的绝对位姿
		createJoint(prev, prev ? PxTransform(offset) : t, current, PxTransform(-offset));
		gScene->addActor(*current);
		prev = current;
		localTm.p.x += separation;
	}

}

optional<PxRigidDynamic*> PhysXWorld::createDynamic(const PxTransform &t, const PxGeometry &geometry, const PxVec3 &velocity)
{
    PxRigidDynamic* dynamic = gPhysics->createRigidDynamic(t);
    if(!dynamic)
    {
        std::cerr << "Failed to create PxRigidDynamic." << std::endl;
        return nullopt;
    }
    // 设置旋转阻尼
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	gScene->addActor(*dynamic);
    return dynamic;
}

optional<PxJoint*> PhysXWorld::createLimitedSphericalJoint(PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
    // 对齐参考实现 SnippetJoint.cpp 的 createLimitedSpherical：
    // 球形关节 + 圆锥限位（绕 Y / Z 各最多 pi/4），再打开限位开关。
    PxSphericalJoint* joint = PxSphericalJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
    if(!joint)
    {
        std::cerr << "Failed to create PxSphericalJoint." << std::endl;
        return nullopt;
    }
    joint->setLimitCone(PxJointLimitCone(PxPi/4, PxPi/4));
    joint->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, true);

    return joint;
}


} // namespace PhysXLearner
