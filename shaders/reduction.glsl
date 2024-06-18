#version 460 core

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(binding = 1) buffer maxvalues
{
    uint maxval[];
};
void main()
{
    uint size = gl_WorkGroupSize.x;
    uint id = gl_LocalInvocationID.x;
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
