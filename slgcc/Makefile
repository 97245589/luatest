lworld.so : cc/*.cc
	g++ -o $@ $^ -Icc -Ilua -fPIC -shared -std=c++17 -O2

.PHONY : clean
clean:
	rm *.so