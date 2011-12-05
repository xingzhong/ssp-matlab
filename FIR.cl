__kernel void FIR(
__global float* input,
__global float* output,
__global float* tap,
const int TAP_SIZE,
const int DATA_SIZE)
{
int iid = get_local_id(0);
int gid = get_group_id(0);
int gsize = get_local_size(0);

if (iid+gid*gsize < DATA_SIZE){
    float temp=0.0;
    for (int i=0; i<TAP_SIZE; i++){
        temp = temp + input[i+iid+gid*gsize]*tap[i];
    }
    //printf("the temp is %f", temp);
    output[iid+gid*gsize] = temp;
}

}
