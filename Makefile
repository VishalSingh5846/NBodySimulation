benchmark: simulation.cpp benchmark.cpp
	g++ -O5 simulation.cpp benchmark.cpp -o nbody 
clean:
	rm -rf nbody
