#version 460 core
out vec4 FragColor;
in vec4 vertexColor;
in vec2 TexCoord;
uniform sampler2D ourTexture;
void main()
{
    FragColor = texture(ourTexture, TexCoord);
};
