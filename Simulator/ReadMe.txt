
==============================
AUTHOR
==============================
Vishal Singh (vs2202)


==============================
ENVIRONMENT
==============================
OS: Windows (May also work on linux)
OpenGL 4.6
OpenMP 4.5
gcc/g++ 7.5
CUDA 10.2


==============================
COMPILATION
==============================
Run the Compile.bat, all the code from the src folder will be compiled and all the binaries will be placed in the bin folder.


==============================
BINARY/EXECUTABLE INVOCATION
==============================
nbodyGUI binary takes no parameters and launches the openGL GUI for showing simulation.
All the binaries (even the nbodySerial) takes 3 command line parameters, first is number of objects (aka n of n-body), second is the number of iterations, and third is the number of threads. For CUDA executives, you will specify the total number of threads, and allocation of these threads into blocks will be handled automatically.

Command: executable_name number_of_bodies number_of_iteration number_of_threads


==============================
OTHER SCRIPTS
==============================
Plot.py - This script is used for plotting time and speedup comparisons.


