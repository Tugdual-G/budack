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
uniform uvec3 maxval;
layout(binding = 0, r32ui) uniform uimage2D red_image;
layout(binding = 1, r32ui) uniform uimage2D green_image;
layout(binding = 2, r32ui) uniform uimage2D blue_image;
void main()
{
    // Get the fragment coordinates
    ivec2 imsize = imageSize(red_image).xy;
    ivec2 imcoords = ivec2(TexCoords*imsize);
    float red = float(imageLoad(red_image, imcoords).r)/maxval.x;
    float green = float(imageLoad(green_image, imcoords).r)/maxval.y;
    float blue = float(imageLoad(blue_image, imcoords).r)/maxval.z;
    red = sigmoidal_contrast(0.1, 15, red);
    green = sigmoidal_contrast(0.1, 15, green);
    blue = sigmoidal_contrast(0.1, 15, blue);
   FragColor = vec4(red, green, blue, 1.0);

}
