#pragma once

#include "glm/glm.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <vector>
#include <span>
using namespace glm;


namespace PhysXLearner
{

// ---------------------------------------------------------------------------
// 几何体数据
//
// 三个几何体共用一套尺寸约定：中心在原点，且都内接于边长为 1 的立方体。
//   * Cube   边长 1
//   * Sphere 半径 0.5
//   * Plane  边长 1，位于 XZ 平面（y = 0，法线 +Y，可直接当地面）
// 这样正好对上 PhysX 的 PxBoxGeometry(0.5) / PxSphereGeometry(0.5)，
// 物理形状与渲染网格之间不需要再额外做缩放换算。
//
// 顶点只存位置（3 float = 12 字节），这是刻意的取舍：
// 真正强迫索引无法跨面共享顶点的，只有"逐面不同"的属性（法线、UV）。
// 去掉它们之后，立方体的 8 个角点、球体的接缝与两极、平面网格的内部点
// 都能被多个三角形复用，索引数不变而顶点数明显下降。
//
// 由此带来的两个后续约束，先记在这里免得以后踩坑：
//   * 要做光照，得在着色器里由位置反推法线。球体最简单，normalize(position) 就是法线。
//   * 要加 UV，球体接缝必须拆开（u = 0 与 u = 1 不是同一个点），
//     顶点数会从 (rings-1)*segments+2 涨回 (rings+1)*(segments+1)。
// ---------------------------------------------------------------------------

struct Vertex
{
    vec3 position;
};

// 索引统一用 32 位。默认细分下顶点数远低于 65536（球 32x16 只有 482 个），
// 换 uint16 能省一半索引带宽，但把 segments 调大就会静默溢出，不值得冒这个险。
using Index = std::uint32_t;

// 与图形 API 无关的纯 CPU 端数据，直接对应 glBufferData 的两块缓冲。
struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<Index> indices;

    [[nodiscard]] std::size_t vertexCount() const { return vertices.size(); }
    [[nodiscard]] std::size_t indexCount() const { return indices.size(); }
    [[nodiscard]] std::size_t triangleCount() const { return indices.size() / 3; }
    [[nodiscard]] bool empty() const { return vertices.empty() || indices.empty(); }
};

// 单位立方体：8 顶点 / 36 索引。
// 每个角点被 3 个面共享，这是没有逐面属性时的最省方案
// （常见的 24 顶点写法纯粹是为了给每个面独立的法线）。
// 六个面全部 CCW 朝外，可直接启用背面剔除。
inline MeshData createCube()
{
    MeshData mesh;
    mesh.vertices = {
        Vertex{vec3(-0.5f, -0.5f, -0.5f)}, // 0
        Vertex{vec3(0.5f, -0.5f, -0.5f)},  // 1
        Vertex{vec3(0.5f, 0.5f, -0.5f)},   // 2
        Vertex{vec3(-0.5f, 0.5f, -0.5f)},  // 3
        Vertex{vec3(-0.5f, -0.5f, 0.5f)},  // 4
        Vertex{vec3(0.5f, -0.5f, 0.5f)},   // 5
        Vertex{vec3(0.5f, 0.5f, 0.5f)},    // 6
        Vertex{vec3(-0.5f, 0.5f, 0.5f)},   // 7
    };
    mesh.indices = {
        // 后面 z = -0.5
        0, 2, 1, 0, 3, 2,
        // 前面 z = +0.5
        4, 5, 6, 4, 6, 7,
        // 左面 x = -0.5
        0, 4, 7, 0, 7, 3,
        // 右面 x = +0.5
        1, 2, 6, 1, 6, 5,
        // 底面 y = -0.5
        0, 1, 5, 0, 5, 4,
        // 顶面 y = +0.5
        3, 7, 6, 3, 6, 2,
    };
    return mesh;
}

// XZ 平面上的正方形：边长 1，中心在原点，y = 0，法线朝 +Y。
// subdivisions = 1 时是最省的 4 顶点 / 2 三角形，可直接当 PhysX 地面用。
// 细分后是 (subdivisions+1)^2 个顶点的规则网格，位置全部唯一、没有重复顶点：
// 内部顶点最多被 4 个格子复用。
inline MeshData createPlane(int subdivisions = 1)
{
    const int n = std::max(subdivisions, 1); // 至少 1，否则一个三角形都产生不了
    const int side = n + 1;

    MeshData mesh;
    mesh.vertices.reserve(static_cast<std::size_t>(side) * side);
    mesh.indices.reserve(static_cast<std::size_t>(n) * n * 6);

    const float step = 1.0f / static_cast<float>(n);
    for (int z = 0; z < side; ++z)
    {
        for (int x = 0; x < side; ++x)
        {
            mesh.vertices.push_back(Vertex{vec3(
                static_cast<float>(x) * step - 0.5f,
                0.0f,
                static_cast<float>(z) * step - 0.5f)});
        }
    }

    // 行优先摊平后的网格索引
    const auto at = [side](int x, int z) -> Index
    {
        return static_cast<Index>(z * side + x);
    };

    for (int z = 0; z < n; ++z)
    {
        for (int x = 0; x < n; ++x)
        {
            const Index a = at(x, z);         // min x, min z
            const Index b = at(x + 1, z);     // max x, min z
            const Index c = at(x + 1, z + 1); // max x, max z
            const Index d = at(x, z + 1);     // min x, max z

            // 这两个顺序算出的面法线都是 +Y，即从上方俯视时可见
            mesh.indices.push_back(a); mesh.indices.push_back(c); mesh.indices.push_back(b);
            mesh.indices.push_back(a); mesh.indices.push_back(d); mesh.indices.push_back(c);
        }
    }

    return mesh;
}

// UV 球：半径 0.5，中心在原点。
//   segments 经线分段数（绕 Y 轴一圈）
//   rings    纬线分段数（南北极之间）
// 顶点复用方式：
//   * 南北极各压成 1 个顶点，而不是一整圈退化的重复点；
//   * 经线首尾用 % segments 环绕，接缝处直接复用同一批顶点，不复制。
// 于是顶点数是 (rings-1)*segments + 2，而朴素的 UV 球是 (rings+1)*(segments+1)。
// 32 x 16 时分别是 482 与 561，省 14%。
inline MeshData createSphere(int segments = 32, int rings = 16)
{
    const int seg = std::max(segments, 3); // 少于 3 段围不成闭合环
    const int ring = std::max(rings, 2);   // 少于 2 段分不出南北

    constexpr float radius = 0.5f;
    constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;

    MeshData mesh;
    mesh.vertices.reserve(static_cast<std::size_t>(ring - 1) * seg + 2);
    mesh.indices.reserve(static_cast<std::size_t>(ring - 1) * seg * 6);

    // 0 号顶点固定是北极
    mesh.vertices.push_back(Vertex{vec3(0.0f, radius, 0.0f)});

    // 中间环带，r 取 1 到 ring-1，正好避开南北极
    for (int r = 1; r < ring; ++r)
    {
        const float phi = std::numbers::pi_v<float> * static_cast<float>(r) / static_cast<float>(ring);
        const float y = radius * std::cos(phi);
        const float ringRadius = radius * std::sin(phi);

        for (int s = 0; s < seg; ++s)
        {
            const float theta = twoPi * static_cast<float>(s) / static_cast<float>(seg);
            mesh.vertices.push_back(Vertex{vec3(
                ringRadius * std::cos(theta),
                y,
                ringRadius * std::sin(theta))});
        }
    }

    constexpr Index northPole = 0;
    const Index southPole = static_cast<Index>(mesh.vertices.size());
    mesh.vertices.push_back(Vertex{vec3(0.0f, -radius, 0.0f)});

    // 第 r 环（1 <= r <= ring-1）第 s 个顶点的索引，s 自动取模以实现接缝复用
    const auto ringAt = [seg](int r, int s) -> Index
    {
        return static_cast<Index>(1 + (r - 1) * seg + (s % seg));
    };

    // 北极扇形：极点相当于退化掉的一整环，所以只出三角形
    for (int s = 0; s < seg; ++s)
    {
        mesh.indices.push_back(northPole);
        mesh.indices.push_back(ringAt(1, s + 1));
        mesh.indices.push_back(ringAt(1, s));
    }

    // 中间环带：每个四边形拆成两个三角形，s 环绕取模，接缝不额外占顶点
    for (int r = 1; r + 1 < ring; ++r)
    {
        for (int s = 0; s < seg; ++s)
        {
            const Index a = ringAt(r, s);
            const Index b = ringAt(r, s + 1);
            const Index c = ringAt(r + 1, s + 1);
            const Index d = ringAt(r + 1, s);

            mesh.indices.push_back(a); mesh.indices.push_back(b); mesh.indices.push_back(c);
            mesh.indices.push_back(a); mesh.indices.push_back(c); mesh.indices.push_back(d);
        }
    }

    // 南极扇形
    for (int s = 0; s < seg; ++s)
    {
        mesh.indices.push_back(southPole);
        mesh.indices.push_back(ringAt(ring - 1, s));
        mesh.indices.push_back(ringAt(ring - 1, s + 1));
    }

    return mesh;
}

class RenderOperator
{
public:
    RenderOperator() = default;        // 空对象，不建任何 GL 资源
    explicit RenderOperator(const MeshData& data);   // 上传 VBO/EBO、建 VAO、配置实例属性
    virtual ~RenderOperator();                                    // glDeleteVertexArrays/Buffers

    RenderOperator(const RenderOperator&)            = delete;      // 关键：拷贝会导致 VAO 被删两次
    RenderOperator& operator=(const RenderOperator&) = delete;
    RenderOperator(RenderOperator&& other) noexcept;                // 移动：转移句柄，源置 0
    RenderOperator& operator=(RenderOperator&& other) noexcept;

    // 每帧更新实例数据；容量不足时按 2 倍扩容
    virtual void setInstances(std::span<const glm::mat4> transforms);
    virtual void drawInstanced() const;                 // 绑定 VAO + glDrawElementsInstanced

    virtual std::size_t vertexCount()  const{ return vertexCount_; };
    virtual std::size_t indexCount()   const{ return indexCount_; };
    virtual std::size_t instanceCount() const{ return instanceCount_; };


// 派生类（CubeRenderOperator 等）的构造函数要在这里填充 VAO/VBO/EBO/实例缓冲，
// 所以资源句柄必须是 protected 而不能是 private
protected:
    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    unsigned int ebo_ = 0;
    unsigned int instanceVbo_ = 0;
    std::size_t  indexCount_ = 0;
    std::size_t  vertexCount_ = 0;
    std::size_t  instanceCount_ = 0;
    std::size_t  instanceCapacity_ = 0;
};

// 三种几何体唯一的区别就是「默认用哪一份 MeshData」：
// 上传 VBO/EBO、建 VAO、配置实例属性、逐帧绘制，全部由基类 RenderOperator 完成，
// 所以派生类只需要一行转发给基类构造函数的构造函数。
class CubeRenderOperator : public RenderOperator
{
public:
    explicit CubeRenderOperator(const MeshData& data = createCube());
};

class SphereRenderOperator : public RenderOperator
{
public:
    explicit SphereRenderOperator(const MeshData& data = createSphere());
};

class PlaneRenderOperator : public RenderOperator
{
public:
    explicit PlaneRenderOperator(const MeshData& data = createPlane());
};


class RenderOperatorFactory
{
private:
    // 引用成员一旦绑定就不能重新指向，而 FactoryInit 的语义是「事后再指定目标」，
    // 所以这里必须存指针。默认 nullptr，调用过 FactoryInit 之前不能使用。
    RenderOperator* renderOperator = nullptr;
public:
    void FactoryInit(RenderOperator& renderOperator);
    void setInstances(std::span<const glm::mat4> transforms);
    void drawInstanced();                 // 绑定 VAO + glDrawElementsInstanced

};

}// namespace PhysXLearner