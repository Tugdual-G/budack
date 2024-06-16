#version 460 core

struct pts_msg {
  double x;
  double y;
  uint nit;
  uint color;
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(location=0) uniform float dx;
layout(binding = 0) buffer starting_points
{
    pts_msg strt[];
};
layout(binding = 0, r32ui) uniform uimage2D red_image;
layout(binding = 1, r32ui) uniform uimage2D green_image;
layout(binding = 2, r32ui) uniform uimage2D blue_image;
void main()
{
    ivec2 coords;
    double dx_ = double(dx);
    double y0 = strt[gl_GlobalInvocationID.x].y;
    double x0 = strt[gl_GlobalInvocationID.x].x;
    uint nit = strt[gl_GlobalInvocationID.x].nit;
    uint col = strt[gl_GlobalInvocationID.x].color;
    double y = y0;
    double x = x0;
    double x2 = x*x;
    double y2 = y*y;
    coords = ivec2(int((2.3 + x)/dx_), int((1.5 + y )/dx_));
    switch (col){
        case 114:{
            imageAtomicAdd(red_image, coords, 1);
            for (uint k = 0; k < nit; ++k) {
                y = 2 * x * y + y0;
                x = x2 - y2 + x0;
                x2 = x * x;
                y2 = y * y;
                coords.x = int((2.3 + x ) / dx_);
                coords.y = int((1.5 + y ) / dx_);
                imageAtomicAdd(red_image, coords, 1);
                // coords.y = int((1.5 - y ) / dx_);
                // imageAtomicAdd(red_image, coords, 1);
            }
            break;
        }
        case 103:{
            imageAtomicAdd(green_image, coords, 1);
            for (uint k = 0; k < nit; ++k) {
                y = 2 * x * y + y0;
                x = x2 - y2 + x0;
                x2 = x * x;
                y2 = y * y;
                coords.x = int((2.3 + x ) / dx_);
                coords.y = int((1.5 + y ) / dx_);
                imageAtomicAdd(green_image, coords, 1);
                // coords.y = int((1.5 - y ) / dx_);
                // imageAtomicAdd(green_image, coords, 1);
            }
            break;
        }
        case 98:{
            imageAtomicAdd(blue_image, coords, 1);
            for (uint k = 0; k < nit; ++k) {
                y = 2 * x * y + y0;
                x = x2 - y2 + x0;
                x2 = x * x;
                y2 = y * y;
                coords.x = int((2.3 + x ) / dx_);
                coords.y = int((1.5 + y ) / dx_);
                imageAtomicAdd(blue_image, coords, 1);
                // coords.y = int((1.5 - y ) / dx_);
                // imageAtomicAdd(blue_image, coords, 1);
            }
            break;
        }
    }

}
