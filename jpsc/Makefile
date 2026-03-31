ljps.so : cc/*.cc
	g++ -o $@ $^ -Icc -Ilua -O2 -fPIC -shared -std=c++17 

.PHONY : clean
clean:
	rm *.so