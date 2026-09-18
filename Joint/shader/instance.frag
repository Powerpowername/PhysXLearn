#version 430 core

// 先只出纯色：顶点数据里只有位置，没有法线，做不了光照。
// 后面要加光照时，法线由片元着色器用屏幕空间导数就地算出来：
//   vec3 faceNormal = normalize(cross(dFdx(worldPos), dFdy(worldPos)));
// 立方体只有 8 个共享顶点，逐顶点法线会被平均掉、看起来像球，必须走这条路。
uniform vec3 uColor;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(uColor, 1.0);
}
