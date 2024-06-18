#version 460 core

struct pts_msg {
  double x;
  double y;
  uint nit;
  uint color;
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(location=0)  uniform float dx;
layout(binding = 0) readonly buffer starting_points
{
    pts_msg strt[];
};
layout(binding = 1) buffer maxvalues
{
    uint maxval[];
};
layout(binding = 0, r32ui) uniform uimage2D red_image;
layout(binding = 1, r32ui) uniform uimage2D green_image;
layout(binding = 2, r32ui) uniform uimage2D blue_image;
void main()
{

    uint size = gl_WorkGroupSize.x;
    //uint size = gl_NumWorkGroups.x;
    uint id = gl_LocalInvocationID.x;
    uint maxr=maxval[3*id];
    uint maxg=maxval[3*id+1];
    uint maxb=maxval[3*id+2];

    uint read;
    ivec2 coords;
    double dx_ = double(dx);

    double y0 = strt[id].y;
    double x0 = strt[id].x;
    uint nit = strt[id].nit;
    uint col = strt[id].color;
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
                read = imageLoad(red_image, coords).r;
                if (read > maxr){
                maxr = read;
                }
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
                read = imageLoad(green_image, coords).r;
                if (read > maxg){
                maxg = read;
                }
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
                read = imageLoad(blue_image, coords).r;
                if (read > maxb){
                maxb = read;
                }
                // coords.y = int((1.5 - y ) / dx_);
                // imageAtomicAdd(blue_image, coords, 1);
            }
            break;
        }
    }

    maxval[3*id] = maxr;
    maxg=maxval[3*id+1] = maxg;
    maxval[3*id+2] = maxb;
    barrier();
    while (size > 0){
        size /= 2;
        if (id<size){
            maxval[3 * id] = max(maxval[3 * id], maxval[3 * id + 3 * size]);
            maxval[3 * id + 1] = max(maxval[3 * id + 1], maxval[3 * id + 3 * size + 1]);
            maxval[3 * id + 2] = max(maxval[3 * id + 2], maxval[3 * id + 3 * size + 2]);
        }
        barrier();
    }
}
