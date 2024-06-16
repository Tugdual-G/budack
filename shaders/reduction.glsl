#version 460 core

layout (local_size_x = 2, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, r32ui) uniform uimage2D red_image;
layout(binding = 1, r32ui) uniform uimage2D green_image;
layout(binding = 2, r32ui) uniform uimage2D blue_image;
// layout(binding = 3, r32ui) uniform uimage2D reduc;
shared uint reduc[2];
void main()
{
    ivec2 texel_coords = ivec2(gl_GlobalInvocationID.xy);

    uint red = uint((imageLoad(red_image, texel_coords).r)*(1.0-texel_coords.x/float(gl_NumWorkGroups.x)));
    uint green = uint((imageLoad(green_image, texel_coords).r)*texel_coords.x/float(gl_NumWorkGroups.x));

    imageStore(red_image, texel_coords, uvec4(red, 0, 0,0));
    imageStore(green_image, texel_coords, uvec4(green, 0,0,0));

}
