#include "PhysXConfig.hpp"
#include <thread>
#include <iostream>
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

optional<PxJoint*> PhysXWorld::createLimitedSphericalJoint(PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1)
{
    PxFixedJoint* joint = PxFixedJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
    if(!joint)
    {
        std::cerr << "Failed to create PxFixedJoint." << std::endl;
        return nullopt;
    }
    joint->setBreakForce(1000.0f,1000.0f);
    // 设置驱动为力驱动而不是冲量驱动
    joint->setConstraintFlag(PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, true);
    joint->setConstraintFlag(PxConstraintFlag::eDISABLE_PREPROCESSING, true);

    return joint;
}


} // namespace PhysXLearner
