#pragma once
#include "PxPhysicsAPI.h"
#include <vector>
#include <memory>
#include <optional>
#include <variant>
using namespace physx;
using namespace std;
using namespace Learner;



namespace Learner
{
// struct SphereShape
// {
//     double radius;
// };
// struct Cube
// {



// };
class PhysXConfig
{
private:
    static	PxDefaultAllocator		gAllocator;
    static	PxDefaultErrorCallback	gErrorCallback;
    inline static	PxFoundation*			gFoundation = NULL;
    inline static	PxPhysics*				gPhysics	= NULL;
    inline static	PxDefaultCpuDispatcher*	gDispatcher = NULL;
    inline static	PxScene*				gScene		= NULL;
    inline static	PxMaterial*				gMaterial	= NULL;
    inline static	PxPvd*                  gPvd        = NULL;



public:
    static void init();
    static optional<bool> createSphereShape(PxReal radius,bool isExclusive);
};


};