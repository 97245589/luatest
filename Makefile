INCLUDE := -I3rd -Icc -Ilua
FLAG := -O2 -fPIC -std=c++17
LINK := -shared -s -flto=auto

ltest.so : cc/*.cc
	g++ -o $@ $^ $(INCLUDE) $(FLAG) $(LINK)

clean :
	rm *.so