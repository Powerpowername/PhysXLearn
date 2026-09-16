#include "PxPhysicsAPI.h"
#define PVD_HOST "127.0.0.1"	//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.
#include <optional>
#include <functional>
#include <vector>
using namespace physx;
using namespace std;
namespace PhysXLearner
{
class PhysXWorld
{
public:
    static optional<bool> Init();
    static void stepPhysics();
    static void cleanupPhysics();
    static void createActorCallback(vector<function<void()>> callbacks);




private:
};


}
