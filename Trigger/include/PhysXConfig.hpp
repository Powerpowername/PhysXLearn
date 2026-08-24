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

enum class SimulationType
{
    CONCAT,
    TRIGGER,
    CALLBACK
};

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
    static optional<bool> createSphereShape(PxReal radius,bool isExclusive,SimulationType simulationType);
    static PxFilterFlags filterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
												PxFilterObjectAttributes attributes1, PxFilterData filterData1,
												PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);

};


};