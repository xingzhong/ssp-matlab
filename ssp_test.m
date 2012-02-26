%% test script
% Xingzhong 
% Dec.4 2011
% fft will be overloaded by a variable inside this file scope 
% Then fft variable will be equal to the function handler @sspfft
% check file @sspfft, it is the fft wrapper 

mex sspfft_2.cpp fft_setup.cpp fft_kernelstring.cpp fft_execute.cpp

%% important function overload method 
% make sure execute it at first 
fft = @sspfft;

%% dummy fft 
% only do a simple array add one constant 
n = 16;
s = randn(1,n);
y = fft(s,n);
disp(y);

%% make sure clear the fft variable
% then fft can be used as regular function
clear fft;

% n = 16;
tic;
tstart=tic;
% s = randn(1,n);
yy = fft(s,n);
t = toc(tstart)
disp(yy);