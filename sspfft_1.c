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


#include "mex.h"
/* #include opencl lib */

/* The computational routine FIXME*/
void sspfft_1(double *input, double *output, double h, mwSize n)
{
    mwSize i;
    /* multiply each element y by x */
    for (i=0; i<n; i++) {
        output[i] = input[i] + h;
    }
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
