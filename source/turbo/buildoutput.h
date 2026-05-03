#ifndef BUILD_OUTPUT_H
#define BUILD_OUTPUT_H

#define Uses_TTerminal
#include <tvision/tv.h>
#include <vector>
#include "process.h"
#include "cmds.h"

class BuildOutput
{
public:
	static std::ostream *out;
	static std::ostream *progOut;
	static std::ostream *localsOut;
	static bool isDebugging;
	static process_t runningProcess;
	static std::vector<std::string> breakpoints;
	static process_t runRunAsync();
	static process_t runBuildAsync();
	static process_t runCleanAsync();
	static process_t runDebugAsync();
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
#include <vector>
#include <algorithm>

std::ostream* BuildOutput::out = nullptr;
std::ostream* BuildOutput::progOut = nullptr;
std::ostream* BuildOutput::localsOut = nullptr;
bool BuildOutput::isDebugging = false;
process_t BuildOutput::runningProcess = {};
std::vector<std::string> BuildOutput::breakpoints;

process_t BuildOutput::runRunAsync()
{
	char *args[] = {(char*)"sh", (char*)"-c", (char*)"cd . && make run_main 2>&1", NULL};
	process_t p = process_start("sh", args);
	return p;
}

process_t BuildOutput::runBuildAsync()
{
	char *args[] = {(char*)"sh", (char*)"-c", (char*)"make build 2>&1", NULL};
	process_t p = process_start("sh", args);
	return p;
}

process_t BuildOutput::runCleanAsync()
{
	char *args[] = {(char*)"sh", (char*)"-c", (char*)"make clean 2>&1", NULL};
	process_t p = process_start("sh", args);
	return p;
}

process_t BuildOutput::runDebugAsync()
{
	process_t p = process_start("./bin/main.bin", NULL);
	process_cmd(&p, "-gdb-set exec-wrapper stdbuf -o0 -e0");
	process_cmd(&p, "-gdb-set target-async on");
	for (const auto& bp : breakpoints)
	{
		process_break(&p, bp.c_str());
	}
	process_run(&p);
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

void BuildOutput::show(TGroup &owner, const char *workingDir, short command) noexcept
{
	std::string output;
	isDebugging = false;
	switch(command)
	{
		case cmRun:
			BuildOutput::runningProcess = runRunAsync();
			break;
		case cmBuild:
			BuildOutput::runningProcess = runBuildAsync();
			break;
		case cmClean:
			BuildOutput::runningProcess = runCleanAsync();
			break;
		case cmDebug:
			isDebugging = true;
			BuildOutput::runningProcess = runDebugAsync();
			break;
	}

	TWindow* window;
	TScrollBar *scrollBar;
	TScrollBar *hScrollBar;
	TTerminal* interior;

	TRect r = owner.getExtent().grow(-1, -1);
	if (isDebugging)
	{
		TRect r1 = r;
		r1.b.y = r.a.y + r.b.y / 3;
		TRect r2 = r;
		r2.a.y = r1.b.y;
		r2.b.y = r.a.y + 2 * r.b.y / 3;
		TRect r3 = r;
		r3.a.y = r2.b.y;

		// Debug Console
		window = new TWindow(r1, "Debugger Console", 0);
		scrollBar = window->standardScrollBar(sbVertical | sbHandleKeyboard);
		hScrollBar = window->standardScrollBar(sbHorizontal | sbHandleKeyboard);
		interior = new TTerminal(window->getExtent().grow(-1, -1), hScrollBar, scrollBar, 0x0F00);
		window->insert(interior);
		out = new std::ostream(interior);
		owner.insert(window);

		// Program Output
		window = new TWindow(r2, "Program Output", 0);
		scrollBar = window->standardScrollBar(sbVertical | sbHandleKeyboard);
		hScrollBar = window->standardScrollBar(sbHorizontal | sbHandleKeyboard);
		interior = new TTerminal(window->getExtent().grow(-1, -1), hScrollBar, scrollBar, 0x0F00);
		window->insert(interior);
		progOut = new std::ostream(interior);
		owner.insert(window);

		// Locals
		window = new TWindow(r3, "Local Variables", 0);
		scrollBar = window->standardScrollBar(sbVertical | sbHandleKeyboard);
		hScrollBar = window->standardScrollBar(sbHorizontal | sbHandleKeyboard);
		interior = new TTerminal(window->getExtent().grow(-1, -1), hScrollBar, scrollBar, 0x0F00);
		window->insert(interior);
		localsOut = new std::ostream(interior);
		owner.insert(window);
	}
	else
	{
		window = new TWindow(r, "Build & Run", 0);

		scrollBar = window->standardScrollBar(sbVertical | sbHandleKeyboard);
		hScrollBar = window->standardScrollBar(sbHorizontal | sbHandleKeyboard);
		
		interior = new TTerminal( window->getExtent().grow(-1, -1),
								hScrollBar,	
								scrollBar,
								0x0F00);

		window->insert(interior);
		out = new std::ostream(interior);
		owner.insert(window);
	}

	if(output.length() > 0)
	{
		(*out) << output;
	}
}

#endif