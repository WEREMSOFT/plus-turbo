#ifndef BUILD_OUTPUT_H
#define BUILD_OUTPUT_H

#define Uses_TTerminal
#include <tvision/tv.h>
#include "process.h"

class BuildOutput
{
public:
	TWindow* window;
	TScrollBar *scrollBar;
	TTerminal* interior;
	std::ostream *out;
    static void show(TGroup &owner, const char *workingDir, short command) noexcept;
    static void clean(TGroup &owner, const char *workingDir) noexcept;
    static void run(TGroup &owner, const char *workingDir) noexcept;
};

#include <tvision/help.h>

#define Uses_TWindow
#include <tvision/tv.h>

#include <cstdio>
#include <array>
#include <string>
#include <sstream>

static std::string runMake(const char *workingDir)
{
    std::array<char, 256> buffer;
    std::string result;

    std::string cmd = "cd \"";
    cmd += workingDir;
    cmd += "\" && make build 2>&1";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return "Failed to run make.\n";

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        result += buffer.data();

    pclose(pipe);

    if (result.empty())
        result = "Build finished with no output.\n";

    return result;
}

static std::string runRun(const char *workingDir)
{
    std::array<char, 256> buffer;
    std::string result;

    std::string cmd = "cd \"";
    cmd += workingDir;
    cmd += "\" && make run_main 2>&1";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return "Failed to run make.\n";

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        result += buffer.data();

    pclose(pipe);

    if (result.empty())
        result = "Build finished with no output.\n";

    return result;
}

static std::string runClean(const char *workingDir)
{
    std::array<char, 256> buffer;
    std::string result;

    std::string cmd = "cd \"";
    cmd += workingDir;
    cmd += "\" && make clean 2>&1";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return "Failed to run make.\n";

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        result += buffer.data();

    pclose(pipe);

    if (result.empty())
        result = "Build finished with no output.\n";

    return result;
}

void BuildOutput::show(TGroup &owner, const char *workingDir, short command) noexcept
{

	std::string output;

	switch(command)
	{
		case cmRun:
			output = runRun(workingDir);
			break;
		case cmBuild:
		 	output = runMake(workingDir);
			break;
		case cmClean:
		 	output = runClean(workingDir);
			break;
	}

	TWindow* window;
	TScrollBar *scrollBar;
	TScrollBar *hScrollBar;
	TTerminal* interior;
	std::ostream *out;

	window = new TWindow(owner.getExtent().grow(-1, -1), "Build & Run", 0);
	// window->palette = wpGrayWindow;

	scrollBar = window->standardScrollBar(sbVertical | sbHandleKeyboard);
	hScrollBar = window->standardScrollBar(sbHorizontal | sbHandleKeyboard);
	
	interior = new TTerminal( window->getExtent().grow(-1, -1),
							  hScrollBar,	
                              scrollBar,
                              0x0F00);

    window->insert(interior);
	out = new std::ostream(interior);
	owner.insert(window);

	(*out) << output;

}

#endif