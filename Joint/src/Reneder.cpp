#include "Reneder.hpp"

#include <glad/glad.h>

namespace PhysXLearner
{

RenderOperator::RenderOperator(const MeshData& data)
{
    // 把一份 MeshData 变成可以用 glDrawElementsInstanced 绘制的 GPU 资源：
    //   VAO          记录下面所有属性配置与 EBO 绑定
    //   VBO          location 0，顶点位置
    //   EBO          索引
    //   instanceVBO  location 1~4，每实例一个 mat4
    vertexCount_ = data.vertexCount();
    indexCount_ = data.indexCount();
    instanceCount_ = 0;
    instanceCapacity_ = 1000;

    // VAO 必须先绑定：下面的 glVertexAttribPointer / glVertexAttribDivisor
    // 以及 EBO 的绑定，都是在修改「当前绑定的 VAO」的状态
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // ---- 顶点缓冲：位置，location = 0 -----------------------------------
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(data.vertices.size() * sizeof(Vertex)),
                 data.vertices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,                                  // location
                          3,                                  // vec3
                          GL_FLOAT,
                          GL_FALSE,
                          static_cast<GLsizei>(sizeof(Vertex)), // 步长 12 字节
                          reinterpret_cast<const void*>(0));    // 偏移 0
    // 顶点属性不需要 glVertexAttribDivisor，默认就是 0（每个顶点前进一次）

    // ---- 索引缓冲 -------------------------------------------------------
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(data.indices.size() * sizeof(Index)),
                 data.indices.data(),
                 GL_STATIC_DRAW);

    // ---- 实例缓冲：1000 个 mat4，只预留不填数据 -------------------------
    glGenBuffers(1, &instanceVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(instanceCapacity_ * sizeof(mat4)),
                 nullptr,
                 GL_DYNAMIC_DRAW);

    // 一个 mat4 拆成 4 个 vec4，分别占 location 1~4，步长都是 64 字节。
    // divisor = 1：这 4 个属性每画一个实例才前进一次，而不是每个顶点前进。
    for (std::size_t column = 0; column < 4; ++column)
    {
        const auto location = static_cast<GLuint>(1 + column);

        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location,
                              4,                                     // vec4
                              GL_FLOAT,
                              GL_FALSE,
                              static_cast<GLsizei>(sizeof(mat4)),     // 步长 64 字节
                              reinterpret_cast<const void*>(column * sizeof(vec4)));
        glVertexAttribDivisor(location, 1);
    }

    // 解绑顺序有讲究：先解 VAO，再解 EBO。
    // 因为 EBO 的绑定是记在 VAO 里的，VAO 还绑着就去解 EBO，
    // 会把 EBO 从 VAO 的状态里摘掉。
    // GL_ARRAY_BUFFER 的绑定不属于 VAO（它属于给 glVertexAttribPointer 用的上下文状态），可随时解绑。
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// 三个派生类的唯一区别就是默认用哪一份 MeshData，其余全部复用基类
CubeRenderOperator::CubeRenderOperator(const MeshData& data) : RenderOperator(data) {}
SphereRenderOperator::SphereRenderOperator(const MeshData& data) : RenderOperator(data) {}
PlaneRenderOperator::PlaneRenderOperator(const MeshData& data) : RenderOperator(data) {}

RenderOperator::~RenderOperator()
{
    // 句柄为 0 时 glDelete* 会被驱动直接忽略，所以不必额外判空
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteBuffers(1, &instanceVbo_);
}

// 移动语义的要点：GL 句柄不是指针，是显存对象的「名字」，谁都不能复制。
// 所谓移动就是把名字接过来，再把源对象的名字全部清 0。
// 源对象被置 0 之后：它的析构会调 glDelete*(0)（空操作，安全），
// 它的成员也都变成 0（绘制时因 indexCount_/instanceCount_ 为 0 直接返回，安全）。
RenderOperator::RenderOperator(RenderOperator&& other) noexcept
    : vao_(other.vao_)
    , vbo_(other.vbo_)
    , ebo_(other.ebo_)
    , instanceVbo_(other.instanceVbo_)
    , indexCount_(other.indexCount_)
    , vertexCount_(other.vertexCount_)
    , instanceCount_(other.instanceCount_)
    , instanceCapacity_(other.instanceCapacity_)
{
    other.vao_ = 0;
    other.vbo_ = 0;
    other.ebo_ = 0;
    other.instanceVbo_ = 0;
    other.indexCount_ = 0;
    other.vertexCount_ = 0;
    other.instanceCount_ = 0;
    other.instanceCapacity_ = 0;
}

RenderOperator& RenderOperator::operator=(RenderOperator&& other) noexcept
{
    // 自移动必须挡住：否则下面第一件事就是把自己的缓冲删掉，
    // 而此时 other 就是自己，接过来的会是一堆已经被删掉的死名字。
    if (this == &other)
    {
        return *this;
    }

    // 接管之前先把自己原有的资源还回去，否则这些句柄直接丢失 → 显存泄漏
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteBuffers(1, &instanceVbo_);

    vao_ = other.vao_;
    vbo_ = other.vbo_;
    ebo_ = other.ebo_;
    instanceVbo_ = other.instanceVbo_;
    indexCount_ = other.indexCount_;
    vertexCount_ = other.vertexCount_;
    instanceCount_ = other.instanceCount_;
    instanceCapacity_ = other.instanceCapacity_;

    other.vao_ = 0;
    other.vbo_ = 0;
    other.ebo_ = 0;
    other.instanceVbo_ = 0;
    other.indexCount_ = 0;
    other.vertexCount_ = 0;
    other.instanceCount_ = 0;
    other.instanceCapacity_ = 0;

    return *this;
}

void RenderOperator::setInstances(std::span<const glm::mat4> transforms)
{
    instanceCount_ = transforms.size();

    if (instanceCount_ == 0)
    {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);

    // 容量不够就按 2 倍扩容。这里只是重新分配同一个缓冲对象的存储，
    // VAO 记录的是缓冲对象的编号，不是它的存储地址，所以属性配置不用重做。
    if (instanceCount_ > instanceCapacity_)
    {
        while (instanceCapacity_ < instanceCount_)
        {
            instanceCapacity_ *= 2;
        }

        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(instanceCapacity_ * sizeof(mat4)),
                     nullptr,
                     GL_DYNAMIC_DRAW);
    }

    // 每次全量覆盖，缓冲不重新分配时走的就是这一条 glBufferSubData
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(instanceCount_ * sizeof(mat4)),
                    transforms.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderOperator::drawInstanced() const
{
    if (instanceCount_ == 0 || indexCount_ == 0)
    {
        return;
    }

    glBindVertexArray(vao_);

    // 第二个参数是「索引个数」，不是三角形个数：传 indexCount_ / 3 会静默地只画出三分之一
    glDrawElementsInstanced(GL_TRIANGLES,
                            static_cast<GLsizei>(indexCount_),
                            GL_UNSIGNED_INT,
                            nullptr,                                 // EBO 已记录在 VAO 里，偏移传 0 即可
                            static_cast<GLsizei>(instanceCount_));

    glBindVertexArray(0);
}

void RenderOperatorFactory::FactoryInit(RenderOperator &renderOperator)
{
    this->renderOperator = &renderOperator;
}

void RenderOperatorFactory::setInstances(std::span<const glm::mat4> transforms)
{
    renderOperator->setInstances(transforms);
}

void RenderOperatorFactory::drawInstanced()
{
    renderOperator->drawInstanced();
}

} // namespace PhysXLearner