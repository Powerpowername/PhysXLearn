#version 430 core

// ---------------------------------------------------------------------------
// 属性布局必须和 C++ 侧 CubeRenderOperator 构造函数里的配置逐项对上：
//   location 0  VBO          步长 12 字节  偏移 0    divisor 0  vec3  顶点位置
//   location 1  instanceVBO  步长 64 字节  偏移 0    divisor 1  vec4  实例矩阵第 0 列
//   location 2  instanceVBO  步长 64 字节  偏移 16   divisor 1  vec4  实例矩阵第 1 列
//   location 3  instanceVBO  步长 64 字节  偏移 32   divisor 1  vec4  实例矩阵第 2 列
//   location 4  instanceVBO  步长 64 字节  偏移 48   divisor 1  vec4  实例矩阵第 3 列
//
// location 编号两边写死的是同一套数字，不匹配不会报错，只会画错。
// ---------------------------------------------------------------------------

layout(location = 0) in vec3 aPosition;

// GLSL 不支持给一个 mat4 直接指定 location 段，
// 所以实例矩阵按列拆成 4 个 vec4，各自占一个 location。
layout(location = 1) in vec4 aInstanceColumn0;
layout(location = 2) in vec4 aInstanceColumn1;
layout(location = 3) in vec4 aInstanceColumn2;
layout(location = 4) in vec4 aInstanceColumn3;

uniform mat4 uViewProjection;

void main()
{
    // mat4 的构造函数按「列」接收参数，顺序不能颠倒
    mat4 instanceMatrix = mat4(aInstanceColumn0,
                               aInstanceColumn1,
                               aInstanceColumn2,
                               aInstanceColumn3);

    gl_Position = uViewProjection * instanceMatrix * vec4(aPosition, 1.0);
}
