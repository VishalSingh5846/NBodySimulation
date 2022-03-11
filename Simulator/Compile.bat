call del /F /Q bin\*
call g++ -O5 src\simulationOpenGL.cpp -o bin\nbodyGUI -lopengl32 -lglu32 -lfreeglut
call g++ -fopenmp  src\simulationOpenMP.cpp -o bin\nbodySerial
call g++ -fopenmp -O3  src\simulationOpenMP.cpp -o bin\nbodyOpenMP
call g++ -fopenmp  src\simulationOpenMPSinglePrecision.cpp -o bin\nbodyOpenMPSinglePrecision
call nvcc -Xptxas -O5, src\simulationCuda.cu -o bin\nbodyCuda
call nvcc -Xptxas -O5, src\simulationCudaSinglePrecision.cu -use_fast_math -o bin\nbodyCudaSinglePrecision 
