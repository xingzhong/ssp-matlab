/*==========================================================
 * sspfft_1.c
 *
 * Dummy subroutine, just add constant at the input to 
 * generate output.
 * FIXME: Bring the fft functionality into it
 *
 * The calling syntax is:
 *
 *		output = sspfft_1(input, h)
 *
 * This is a MEX-file for MATLAB.
 * The file need be compiled by mex {
 * if macintosh, means xcode
 * if Unix-like, means gcc
 * }
 * SSP Project
 * Xingzhong Dec. 4 2011
 *========================================================*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/opencl.h>
#include <time.h>

#define MAX_SOURCE_SIZE (0x100000)


#include "mex.h"
/* #include opencl lib */

/* The computational routine FIXME*/
void sspfft_1(double *input, double *output, double h, mwSize n)
{
    mwSize iii;
    /* multiply each element y by x */
    for (iii=0; iii<n; iii++) {
        output[iii] = input[iii] + h;
    }
    
    
    ////////////////////////////////////////////////////////////////////////////////


    int err;                            // error code returned from api calls
    
    int TAP_SIZE = 20;
    int DATA_SIZE = 1024;
    
    float input_h[DATA_SIZE+TAP_SIZE];              // original data set given to device
    float tap_h[TAP_SIZE];                // original tap coefficients given to device
    float results_GPU[DATA_SIZE];           // results returned from device
    float results_CPU[DATA_SIZE];           // results returned from host
    unsigned int correct;               // number of correct results returned
    
    size_t global;                      // global domain size for our calculation
    size_t local;                       // local domain size for our calculation
    
    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel
    
    cl_mem tap_d;                         // device memory used for the tap coefficients
    cl_mem input_d;                       // device memory used for the input array
    cl_mem output_d;                      // device memory used for the output array
    
    FILE *fp;
    char *KernelSource;
    size_t source_size;
    
    
    fp = fopen("/Users/neilhan0210/Desktop/sspfft", "r");
    
    //fp = fopen("square.txt", "r");
    
    if (!fp)
    {
        fprintf(stderr, "Failed to load kernel. \n");
        exit(1);
    }
    KernelSource = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( KernelSource, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp); 
    /* 
     // Load the kernel source code into the array source_str
     FILE *fp;
     char *source_str;
     size_t source_size;
     
     fp = fopen("vector_add_kernel.cl", "r");
     if (!fp) {
     fprintf(stderr, "Failed to load kernel.\n");
     exit(1);
     }
     source_str = (char*)malloc(MAX_SOURCE_SIZE);
     source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
     fclose( fp );
     */
    
    
    // Fill our data set with random float values
    //
    int i = 0;
    int j;
    for(i = 0; i < (DATA_SIZE+TAP_SIZE); i++)
        input_h[i] = rand() / (float)RAND_MAX;
        //input_h[i] = 1.0;
    
    for(i=0 ; i<TAP_SIZE ; i++)
        tap_h[i] = rand() / (float)RAND_MAX;
        //tap_h[i] = 1.0;
    // Connect to a compute device
    //
    int gpu = 1;
    err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return EXIT_FAILURE;
    }
    
    // Create a compute context 
    //
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }
    
    // Create a command commands
    //
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
    }
    
    // Create the compute program from the source buffer
    //
    program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);
    if (!program)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }
    
    // Build the program executable
    //
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];
        
        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }
    
    // Create the compute kernel in the program we wish to run
    //
    kernel = clCreateKernel(program, "FIR", &err);
    if (!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
    }
    
    // Create the input, tap and output arrays in device memory for our calculation
    //
    tap_d = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * TAP_SIZE, NULL, NULL);
    input_d = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * (DATA_SIZE+TAP_SIZE), NULL, NULL);
    output_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * (DATA_SIZE), NULL, NULL);
    if (!input_d || !output_d)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }    
    
    
    // Write our data set into the input array in device memory 
    //
    clock_t t1_move = clock();
    err = clEnqueueWriteBuffer(commands, input_d, CL_TRUE, 0, sizeof(float) * (DATA_SIZE+TAP_SIZE), input_h, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        exit(1);
    }
    
    err = clEnqueueWriteBuffer(commands,tap_d,CL_TRUE,0, sizeof(float)*TAP_SIZE, tap_h, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        exit(1);
    } 
    
    clock_t t2_move = clock();
    
    // Set the arguments to our compute kernel
    //
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_d);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_d);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &tap_d);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &TAP_SIZE);
    err |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &DATA_SIZE);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }
    
    // Get the maximum work group size for executing the kernel on the device
    //
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }
    
    printf("the maximum work group size is %lu\n", local);  // added by Ning 
    
    
    //===== Observe the starting time =====//
    clock_t t1 = clock();
    
    
    // Execute the kernel over the entire range of our 1d input data set
    // using the maximum number of work group items for this device
    //
    global = DATA_SIZE;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err)
    {
        printf("Error: Failed to execute kernel!\n");
        return EXIT_FAILURE;
    }
    
    // Wait for the command commands to get serviced before reading back results
    //
    clFinish(commands);
    
    //===== Observe the ending time =====//
    clock_t t2 = clock();
    
    //===== Print out the running time =====%
    printf("%.4lf seconds of processing for GPU\n", (t2-t1)/(double)CLOCKS_PER_SEC);
    
    // Read back the results from the device to verify the output
    //
    clock_t t3_move = clock();
    err = clEnqueueReadBuffer( commands, output_d, CL_TRUE, 0, sizeof(float) * DATA_SIZE, results_GPU, 0, NULL, NULL );  
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
    }
    
    clock_t t4_move = clock();
    
    //===== Print out the moving data time =====%
    printf("%.4lf seconds of moving data to GPU\n", (t2_move-t1_move + t4_move-t3_move)/(double)CLOCKS_PER_SEC);
    
    //===== Observe the starting time =====//
    clock_t t1c = clock();
    float temp;
    for(i = 0; i < DATA_SIZE; i++)
    {
        temp = 0;
        for (j=0; j<TAP_SIZE; j++)
        {
            temp = temp + input_h[i+j]*tap_h[j];
        }
        results_CPU[i] = temp;
    }
    
    //===== Observe the ending time =====//
    clock_t t2c = clock();
    
    //===== Print out the running time =====%
    printf("%.4lf seconds of processing for CPU\n", (t2c-t1c)/(double)CLOCKS_PER_SEC);
    
    // Validate our results
    //
    correct = 0;
    for(i = 0; i < DATA_SIZE; i++)
    {
        //printf("i=%d : results on GPU = %f ; results on CPU = %f. \n", i, results_GPU[i], results_CPU[i]);
        if(results_GPU[i] == results_CPU[i] )
            correct++;
    }
    
    // Print a brief summary detailing the results
    //
    printf("Computed '%d/%d' correct values!\n", correct, DATA_SIZE);
    
    // Shutdown and cleanup
    //
    clReleaseMemObject(tap_d);
    clReleaseMemObject(input_d);
    clReleaseMemObject(output_d);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

///////////////////////////////////////////////////////////////////////


}

/* The gateway function */
/* Real entry point comming from the MATLAB */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    double h;              /* input scalar */
    double *input;               /* 1xN input matrix */
    mwSize ncols;                   /* size of matrix */
    double *output;              /* output matrix */

    /* check for proper number of arguments */
    if(nrhs!=2) {
        mexErrMsgIdAndTxt("SSP:fft:nrhs","Two inputs required.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt("SSP:fft:nlhs","One output required.");
    }
    /* make sure the first input argument is scalar */
    if( !mxIsDouble(prhs[0]) || 
         mxIsComplex(prhs[0]) ||
         mxGetNumberOfElements(prhs[0])!=1 ) {
        mexErrMsgIdAndTxt("SSP:fft:notScalar","h must be a scalar.");
    }
    
    /* check that number of rows in second input argument is 1 */
    if(mxGetM(prhs[1])!=1) {
        mexErrMsgIdAndTxt("SSP:fft:notRowVector","Input must be a row vector.");
    }
    
    /* get the value of the scalar input  */
    h = mxGetScalar(prhs[0]);

    /* create a pointer to the real data in the input matrix  */
    input = mxGetPr(prhs[1]);

    /* get dimensions of the input matrix */
    ncols = mxGetN(prhs[1]);

    /* create the output matrix */
    plhs[0] = mxCreateDoubleMatrix(1,ncols,mxREAL);

    /* get a pointer to the real data in the output matrix */
    output = mxGetPr(plhs[0]);

    /* call the computational routine */
    sspfft_1(input,output,h,ncols);
}
