#version 460 core
layout(location = 0) in vec3 aPos;        // Vertex position
layout(location = 1) in vec2 aTexCoords;  // Vertex texture coordinates
out vec2 TexCoords;
void main()
{
    gl_Position = vec4(aPos, 1.0);  // Pass-through transformation
    TexCoords = aTexCoords;         // Pass the texture coordinates to the fragment shader
}
