%% test script
% Xingzhong 
% Dec.4 2011
% fft will be overloaded by a variable inside this file scope 
% Then fft variable will be equal to the function handler @sspfft
% check file @sspfft, it is the fft wrapper 
 
%% important function overload method 
% make sure execute it at first 
fft = @sspfft;

%% dummy fft 
% only do a simple array add one constant 
y = fft(3, [0,0,0]);
disp(y);

%% make sure clear the fft variable
% then fft can be used as regular function
clear fft