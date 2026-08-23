#include "PhysXConfig.hpp"
#include "auxi.hpp"
#include <thread>
#define PVD_HOST "127.0.0.1"	//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.

namespace Learner
{

void PhysXConfig::init()
{

    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION,gAllocator,gErrorCallback);
    
    gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);
    
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION,*gFoundation, PxTolerancesScale(), true, gPvd);
	PxInitExtensions(*gPhysics,gPvd);

    const PxU32 numCores = thread::hardware_concurrency();//cpu逻辑核心数量
    // 此处空出一个核心的原因是给主线程留一个使用
	gDispatcher = PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
    // 静摩擦，动摩擦，弹性系数
	gMaterial = gPhysics->createMaterial(1.0f, 1.0f, 0.0f);




}
// 

optional<bool> PhysXConfig::createSphereShape(PxReal radius,bool isExclusive)
{
    
    auto& geometry = PxSphereGeometry(4.0f);

    PxShape* shape = nullptr;


    // failed
    return nullopt;

    return true;
}



}