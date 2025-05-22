all:
	g++ -DUNICODE -DPSAPI_VERSION=1 -D_WIN32_WINNT=0x0601 ./src/*.cpp -lpsapi