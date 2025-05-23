VIEW_SOURCES := $(wildcard ./View/*.cpp)
COMPONENT_SOURCES := $(wildcard ./Component/*.cpp)
IMG_SOURCES := Front/imgui/imgui.cpp Front/imgui/imgui_draw.cpp Front/imgui/imgui_tables.cpp Front/imgui/imgui_widgets.cpp Front/imgui/imgui_demo.cpp Front/imgui/backends/imgui_impl_glfw.cpp Front/imgui/backends/imgui_impl_opengl3.cpp

all:
	g++ -DUNICODE -DPSAPI_VERSION=1 -D_WIN32_WINNT=0x0601 main.cpp $(VIEW_SOURCES) $(COMPONENT_SOURCES) $(IMG_SOURCES) -IFront/imgui -IFront/imgui/backends -Ilib/include -Llib -lglfw3 -lopengl32 -lgdi32 -ldwmapi -limm32 -lole32 -loleaut32 -luuid -static -o dashboard.exe -lpsapi
