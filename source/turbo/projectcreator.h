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
        createFile("src/main.c", mainCContent());
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
        createFile("Makefile", makefileContent());
        createFile("src/main.c", mainCContent());
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
        createFile("src/main.c", mainCContentGlfw());
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
LIBS := -lpthread -lm
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

    static std::string mainCContent() noexcept
    {
        return R"(
#include <stdio.h>

int main(void)
{
    printf("Hello, world!\n");
    return 0;
}
)";
    }

	    static std::string mainCContentGlfw() noexcept
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
};

#endif
