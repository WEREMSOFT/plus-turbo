#ifndef TURBO_PROJECTCREATOR_H
#define TURBO_PROJECTCREATOR_H

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

class ProjectCreator
{
public:
    static void create() noexcept
    {

        // Create directories
        createDir("bin");
        createDir("build");
        createDir("assets");
        createDir("src");

        // Create files with contents
        createFile(".gitignore", gitignoreContent());
        createFile("Makefile", makefileContent());
        createFile("src/main.c", mainContent());
    }

    static void createSDL() noexcept
    {

        // Create directories
        createDir("bin");
        createDir("build");
        createDir("assets");
        createDir("src");

        // Create files with contents
        createFile(".gitignore", gitignoreContent());
        createFile("Makefile", makefileContentSDL());
        createFile("src/main.c", mainContentSDL());
    }

	static void createGLFW() noexcept
    {

        // Create directories
        createDir("bin");
        createDir("build");
        createDir("assets");
        createDir("src");

        // Create files with contents
        createFile(".gitignore", gitignoreContent());
        createFile("Makefile", makefileContentGlfw());
        createFile("src/main.c", mainContentGlfw());
    }
private:
    static void createDir(const fs::path &path) noexcept
    {
        std::error_code ec;
        fs::create_directories(path, ec);
    }

    static void createFile(const fs::path &path, const std::string &content) noexcept
    {
        if (std::filesystem::exists(path))
            return; // don't overwrite

        std::ofstream file(path);
        if (file)
            file << content;
    }

    // ------------------------
    // Hardcoded file contents
    // ------------------------

    static std::string gitignoreContent() noexcept
    {
        return R"(
# Build artifacts
build/
bin/

# OS files
.DS_Store
Thumbs.db

# Editor
*.swp
*.swo
.vscode
)";
    }

 static std::string makefileContentGlfw() noexcept
    {
        return R"(
# Compiler
CC := gcc

# Source files
# Find all .c files from the project and libs.
SRC_C := $(shell find src -name *.c)

# Object files
OBJ_C := $(patsubst %.c,%.o,$(SRC_C))
OBJS := $(OBJ_C) $(OBJ_CPP)

# Object files for cleaning
OBJ_FOR_CLEAN := $(shell find . -name '*.o')

# Libraries and Flags
LIBS := -lpthread -lm -lGL -lglfw
CFLAGS := -g -O0 -w -Wall -Wextra

# Target
TARGET := bin/main.bin

# Phony targets
.PHONY: all run_main clean deep_clean copy_assets web statistics

# Default target
all: $(TARGET)

# Linking the final executable
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LIBS)

# Rule to compile C source files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

build: all copy_assets

# Other targets
run_main: all copy_assets
	$(TARGET)

copy_assets:
	mkdir -p bin
	cp -r assets bin

clean:
	rm -rf $(OBJ_FOR_CLEAN)
	rm -rf $(TARGET)
	rm -rf bin/assets
	rm -rf bin/main.bin

deep_clean: clean

# Emscripten build
web:
	emcc -O2 -g -sSTACK_SIZE=1024000 -sEXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" -sEXPORTED_FUNCTIONS=_malloc,_free,_main -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 -DWITH_MINIAUDIO=1 -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s MAX_WEBGL_VERSION=2 --preload-file assets -s MIN_WEBGL_VERSION=2 -gsource-map $(SRC_C)  -o docs/index.html

# Statistics target
statistics:
	echo >> metrics.txt
	git log -1 --format="%H - %s" >> metrics.txt
	complexity --histogram --score --thresh=3 `find src -name '*.c' -o -name '*.cpp'` >> metrics.txt
)";
    }

    static std::string makefileContent() noexcept
    {
        return R"(# Compiler
CC := gcc

# Source files
# Find all .c files from the project and libs.
SRC_C := $(shell find src -name *.c)

# Object files
OBJ_C := $(patsubst %.c,%.o,$(SRC_C))
OBJS := $(OBJ_C) $(OBJ_CPP)

# Object files for cleaning
OBJ_FOR_CLEAN := $(shell find . -name '*.o')

# Libraries and Flags
LIBS := -lpthread -lm
CFLAGS := -g -O0 -w -Wall -Wextra

# Target
TARGET := bin/main.bin

# Phony targets
.PHONY: all run_main clean deep_clean copy_assets web statistics

build: all copy_assets

# Default target
all: $(TARGET)

# Linking the final executable
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LIBS)

# Rule to compile C source files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# Other targets
run_main: all copy_assets
	$(TARGET)

copy_assets:
	mkdir -p bin
	cp -r assets bin

clean:
	rm -rf $(OBJ_FOR_CLEAN)
	rm -rf $(TARGET)
	rm -rf bin/assets
	rm -rf bin/main.bin

deep_clean: clean

# Emscripten build
web:
	emcc -O2 -g -sSTACK_SIZE=1024000 -sEXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" -sEXPORTED_FUNCTIONS=_malloc,_free,_main -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 -DWITH_MINIAUDIO=1 -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s MAX_WEBGL_VERSION=2 --preload-file assets -s MIN_WEBGL_VERSION=2 -gsource-map $(SRC_C)  -o docs/index.html

# Statistics target
statistics:
	echo >> metrics.txt
	git log -1 --format="%H - %s" >> metrics.txt
	complexity --histogram --score --thresh=3 `find src -name '*.c' -o -name '*.cpp'` >> metrics.txt
)";
    }

	static std::string makefileContentSDL() noexcept
    {
        return R"(# Compiler
CC := gcc

# Source files
# Find all .c files from the project and libs.
SRC_C := $(shell find src -name *.c)

# Object files
OBJ_C := $(patsubst %.c,%.o,$(SRC_C))
OBJS := $(OBJ_C) $(OBJ_CPP)

# Object files for cleaning
OBJ_FOR_CLEAN := $(shell find . -name '*.o')

# Libraries and Flags
LIBS := -lpthread -lm -lSDL3
CFLAGS := -g -O0 -w -Wall -Wextra

# Target
TARGET := bin/main.bin

# Phony targets
.PHONY: all run_main clean deep_clean copy_assets web statistics

build: all copy_assets

# Default target
all: $(TARGET)

# Linking the final executable
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LIBS)

# Rule to compile C source files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# Other targets
run_main: all copy_assets
	$(TARGET)

copy_assets:
	mkdir -p bin
	cp -r assets bin

clean:
	rm -rf $(OBJ_FOR_CLEAN)
	rm -rf $(TARGET)
	rm -rf bin/assets
	rm -rf bin/main.bin

deep_clean: clean

# Emscripten build
web:
	emcc -O2 -g -sSTACK_SIZE=1024000 -sEXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" -sEXPORTED_FUNCTIONS=_malloc,_free,_main -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 -DWITH_MINIAUDIO=1 -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s MAX_WEBGL_VERSION=2 --preload-file assets -s MIN_WEBGL_VERSION=2 -gsource-map $(SRC_C)  -o docs/index.html

# Statistics target
statistics:
	echo >> metrics.txt
	git log -1 --format="%H - %s" >> metrics.txt
	complexity --histogram --score --thresh=3 `find src -name '*.c' -o -name '*.cpp'` >> metrics.txt
)";
    }

    static std::string mainContent() noexcept
    {
        return R"(#include <stdio.h>

int main(void)
{
    printf("Hello, world!\n");
    return 0;
}
)";
    }

	    static std::string mainContentGlfw() noexcept
    {
        return R"(
#include <GLFW/glfw3.h>

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
)";
    }
	static std::string mainContentSDL() noexcept
	{
		return R"(#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
	#include <SDL3/SDL.h>
	#include <SDL3/SDL_main.h>
	
	/* We will use this renderer to draw into this window every frame. */
	static SDL_Window *window = NULL;
	static SDL_Renderer *renderer = NULL;
	
	/* This function runs once at startup. */
	SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
	{
	SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");
	
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	
	if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	SDL_SetRenderLogicalPresentation(renderer, 640, 480, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	
	return SDL_APP_CONTINUE;  /* carry on with the program! */
	}
	
	/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
	SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
	{
		if (event->type == SDL_EVENT_QUIT) {
			return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
		}

		if(event->type == SDL_EVENT_KEY_DOWN)
		{
			if(event->key.scancode == SDL_SCANCODE_ESCAPE)
			{
				return SDL_APP_SUCCESS;
			}
		}

		return SDL_APP_CONTINUE;  /* carry on with the program! */
	}
	
	/* This function runs once per frame, and is the heart of the program. */
	SDL_AppResult SDL_AppIterate(void *appstate)
	{
	const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
	/* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
	const float red = (float) (0.5 + 0.5 * SDL_sin(now));
	const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
	const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
	SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */
	
	/* clear the window to the draw color. */
	SDL_RenderClear(renderer);
	
	/* put the newly-cleared rendering on the screen. */
	SDL_RenderPresent(renderer);
	
	return SDL_APP_CONTINUE;  /* carry on with the program! */
	}
	
	/* This function runs once at shutdown. */
	void SDL_AppQuit(void *appstate, SDL_AppResult result)
	{
	/* SDL will clean up the window/renderer for us. */
	}
	)";
	}
};


#endif

