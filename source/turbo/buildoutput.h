#ifndef BUILD_OUTPUT_H
#define BUILD_OUTPUT_H

#define Uses_TTerminal
#include <tvision/tv.h>
#include "process.h"

class BuildOutput
{
public:
	static std::ostream *out;
	static process_t runningProcess;
	static process_t runRunAsync();
	static process_t runBuildAsync();
	static int getProcessOutput(process_t p, char *buffer, size_t buffer_size);
	static void processKill(process_t p);
	TWindow* window;
	TScrollBar *scrollBar;
	TTerminal* interior;
    static void show(TGroup &owner, const char *workingDir, short command) noexcept;
    static void clean(TGroup &owner, const char *workingDir) noexcept;
    static void run(TGroup &owner, const char *workingDir) noexcept;
	static void updateWindow();
};

#include <tvision/help.h>

#define Uses_TWindow
#include <tvision/tv.h>

#include <cstdio>
#include <array>
#include <string>
#include <sstream>

std::ostream* BuildOutput::out = nullptr;
process_t BuildOutput::runningProcess = {};

process_t BuildOutput::runRunAsync()
{
	process_t p = process_start("cd . && make run_main 2>&1", NULL);
	return p;
}

process_t BuildOutput::runBuildAsync()
{
	process_t p = process_start("make build 2>&1", NULL);
	return p;
}

int BuildOutput::getProcessOutput(process_t p, char *buffer, size_t buffer_size)
{
	return process_poll_output(&p, buffer, sizeof(buffer));
}

void BuildOutput::processKill(process_t p)
{
	process_kill(&p);
}

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
			// output = runRun(workingDir);
			BuildOutput::runningProcess = runRunAsync();
			break;
		case cmBuild:
		 	output = runMake(workingDir);
			// BuildOutput::runningProcess = runBuildAsync();
			break;
		case cmClean:
		 	output = runClean(workingDir);
			break;
	}

	TWindow* window;
	TScrollBar *scrollBar;
	TScrollBar *hScrollBar;
	TTerminal* interior;

	window = new TWindow(owner.getExtent().grow(-1, -1), "Build & Run", 0);

	scrollBar = window->standardScrollBar(sbVertical | sbHandleKeyboard);
	hScrollBar = window->standardScrollBar(sbHorizontal | sbHandleKeyboard);
	
	interior = new TTerminal( window->getExtent().grow(-1, -1),
							  hScrollBar,	
                              scrollBar,
                              0x0F00);

    window->insert(interior);
	out = new std::ostream(interior);
	owner.insert(window);

	if(output.length() > 0)
	{
		(*out) << output;
	}
}

#endif