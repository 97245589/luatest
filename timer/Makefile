ltimer.so : cc/*.cc
	g++ -o $@ $^ -Icc -Ilua -O2 -fPIC -std=c++17 -shared

clean:
	rm *.so