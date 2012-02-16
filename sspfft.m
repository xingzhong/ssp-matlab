function y = sspfft(varargin)
%FFT Overloaded function for SSP Project.
%Xingzhong Dec.4 2011

disp('Overload FFT Function')

%% call mex object 
y = sspfft_2(varargin{:});
