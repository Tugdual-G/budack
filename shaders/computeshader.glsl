#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(location=0) uniform float dx;
layout(binding = 0) buffer starting_points
{
    double strt[];
};
layout(binding = 1) buffer points
{
    double pts[];
};
layout(binding = 0, r32ui) uniform uimage2D red_image;
layout(binding = 1, r32ui) uniform uimage2D green_image;
//layout(binding = 2, r32ui) uniform uimage2D blue_image;
void main()
{
    double dx_ = double(dx);
    double y0 = strt[2*gl_GlobalInvocationID.x];
    double x0 = strt[2*gl_GlobalInvocationID.x+1];
    double y = pts[2*gl_GlobalInvocationID.x];
    double x = pts[2*gl_GlobalInvocationID.x+1];
    double y2 = y*y;

    ivec2 coords = ivec2(int((2.3 + x)/dx_), int((1.5 + y )/dx_));

    imageAtomicAdd(red_image, coords, 1);

    // coords.y = int((1.5 - y )/dx_);
    // imageAtomicAdd(red_image, coords, 1);

    y = 2 * x * y + y0;
    x = x*x - y2 + x0;

    pts[2*gl_GlobalInvocationID.x] = y;
    pts[2*gl_GlobalInvocationID.x+1] = x;

}
