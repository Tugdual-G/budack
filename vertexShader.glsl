#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec4 vertexColor;
out vec2 TexCoord;
void main()
{
   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
   vertexColor = vec4(1, (1+aPos.x)/2.0, (1+aPos.y)/2.0, 1.0);
   TexCoord = aTexCoord;
};
