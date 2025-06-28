# VIEW_SOURCES := $(wildcard ./View/*.cpp)
CONTROLLER_SOURCES := $(wildcard ./Controller/src/*.cpp)
MODEL_SOURCES := $(wildcard ./Model/src/*.cpp)
# IMG_SOURCES := View/imgui/imgui.cpp View/imgui/imgui_draw.cpp View/imgui/imgui_tables.cpp View/imgui/imgui_widgets.cpp View/imgui/imgui_demo.cpp View/imgui/backends/imgui_impl_glfw.cpp View/imgui/backends/imgui_impl_opengl3.cpp

# all:
# 	g++ -DUNICODE -DPSAPI_VERSION=1 -D_WIN32_WINNT=0x0601 main.cpp $(VIEW_SOURCES) $(CONTROLLER_SOURCES) $(IMG_SOURCES) $(MODEL_SOURCES) -IView/imgui -IView/imgui/backends -Ilib/include -Llib -lglfw3 -lopengl32 -lgdi32 -ldwmapi -limm32 -lole32 -loleaut32 -luuid -static -o dashboard.exe -lpsapi -lpdh

all: 
	g++ -Iinclude main.cpp $(CONTROLLER_SOURCES) $(MODEL_SOURCES) -o server -pthread -lws2_32 -lmswsock
