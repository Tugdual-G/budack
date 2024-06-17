#version 460 core

float sigmoidal_contrast(float alpha, float beta, float x) {
  // x must be normalized to 1
  return (1 / (1 + exp(beta * (alpha - x))) - 1 / (1 + exp(beta * (alpha)))) /
         (1 / (1 + exp(beta * (alpha - 1))) - 1 / (1 + exp(beta * alpha)));
}

// Output color
out vec4 FragColor;
in vec2 TexCoords;
// Image load/store
//
uniform uvec3 maxv;
layout(binding = 0, r32ui) uniform uimage2D redImage;
layout(binding = 1, r32ui) uniform uimage2D greenImage;
layout(binding = 2, r32ui) uniform uimage2D blueImage;
void main()
{
    // Get the fragment coordinates
    ivec2 imsize = imageSize(redImage).xy;
    ivec2 imcoords = ivec2(TexCoords*imsize);

    float red = float(imageLoad(redImage, imcoords).r)/maxv.x;
    float green = float(imageLoad(greenImage, imcoords).r)/maxv.y;
    float blue = float(imageLoad(blueImage, imcoords).r)/maxv.z;
    red = sigmoidal_contrast(0.1, 5, red);
    green = sigmoidal_contrast(0.1, 5, green);
    blue = sigmoidal_contrast(0.1, 5, blue);
    // Assemble the color
   FragColor = vec4(red, green, blue, 1.0);

}
