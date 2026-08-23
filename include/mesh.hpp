#include "glm/glm.hpp"
#include <array>
using namespace std;
using namespace glm;
namespace Learner
{
struct Vertex
{
vec3 position;
};

struct Cube
{
    array<Vertex, 8> vetex{
        vec3(-0.5f, -0.5f, -0.5f), // 0
        vec3(0.5f, -0.5f, -0.5f),  // 1
        vec3(0.5f, 0.5f, -0.5f),   // 2
        vec3(-0.5f, 0.5f, -0.5f),  // 3

        vec3(-0.5f, -0.5f, 0.5f), // 4
        vec3(0.5f, -0.5f, 0.5f),  // 5
        vec3(0.5f, 0.5f, 0.5f),   // 6
        vec3(-0.5f, 0.5f, 0.5f)   // 7
    };
    array<int, 36> index{
        // back face, z = -0.5
        0, 2, 1,
        0, 3, 2,

        // front face, z = 0.5
        4, 5, 6,
        4, 6, 7,

        // left face, x = -0.5
        0, 4, 7,
        0, 7, 3,

        // right face, x = 0.5
        1, 2, 6,
        1, 6, 5,

        // bottom face, y = -0.5
        0, 1, 5,
        0, 5, 4,

        // top face, y = 0.5
        3, 7, 6,
        3, 6, 2
    };
};
}