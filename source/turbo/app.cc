#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TKeys
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu
#define Uses_TWindow
#define Uses_TFrame
#define Uses_TFileDialog
#define Uses_TIndicator
#define Uses_TStaticText
#define Uses_TParamText
#define Uses_TScreen
#define Uses_TChDirDialog
#define Uses_TButton
#define Uses_MsgBox
#include <tvision/tv.h>
#include <filesystem>
#include <cstdlib>

#include "app.h"
#include "help.h"
#include "buildoutput.h"
#include "apputils.h"
#include "editwindow.h"
#include "widgets.h"
#include "listviews.h"
#include "doctree.h"
#include "foldertree.h"
#include "projectcreator.h"
#include "process.h"
#include <turbo/fileeditor.h>
#include <turbo/tpath.h>
#include <variant>
#include <vector>

using namespace Scintilla;
namespace fs = std::filesystem;

TurboApp::TurboApp(int argc, const char *argv[]) noexcept :
    TProgInit( &TurboApp::initStatusLine,
               &TurboApp::initMenuBar,
               &TApplication::initDeskTop
             ),
    argc(argc),
    argv(argv)
{
    TCommandSet ts;
    ts += cmSave;
    ts += cmSaveAs;
    ts += cmRename;
    ts += cmToggleWrap;
    ts += cmToggleLineNums;
    ts += cmFind;
    ts += cmReplace;
    ts += cmGoToLine;
    ts += cmSearchAgain;
    ts += cmSearchPrev;
    ts += cmToggleIndent;
    ts += cmCloseEditor;
    ts += cmSelUppercase;
    ts += cmSelLowercase;
    ts += cmSelCapitalize;
    ts += cmToggleComment;
    ts += cmUndo;
    ts += cmRedo;
    ts += cmCut;
    ts += cmCopy;
    ts += cmPaste;
    disableCommands(ts);

    // Actions that only make sense when there is at least one editor.
    editorCmds += cmEditorNext;
    editorCmds += cmEditorPrev;
    editorCmds += cmTreeNext;
    editorCmds += cmTreePrev;
    editorCmds += cmCloseAll;
    editorCmds += cmCloseEditor;
    disableCommands(editorCmds);

    // Create the clock view.
    TRect r = getExtent();
    r.a.x = r.b.x - 9;
    r.b.y = r.a.y + 1;
    clock = new TClockView(r);
    clock->growMode = gfGrowLoX | gfGrowHiX;
    insert(clock);

    // Create the document tree view
    {
        TRect r = deskTop->getExtent();
        // Try to make it between 22 and 30 columns wide, and try to leave
        // at least 82 empty columns on screen (so that an editor view is
        // at least ~80 columns by default).
        if (r.b.x > 22)
            r.a.x = r.b.x - min(max(r.b.x - 82, 22), 30);
        docTree = new DocumentTreeWindow(r, &docTree);
        docTree->flags &= ~wfZoom;
        // The grow mode assumes it's placed on the right side of the screen.
        // Greater flexibility would require some trick or a dedicated class
        // for side views.
        docTree->growMode = gfGrowLoX | gfGrowHiX | gfGrowHiY;
        docTree->setState(sfShadow, False);
        deskTop->insert(docTree);
        // Show by default only on large terminals.
        if (deskTop->size.x - docTree->size.x < 82)
            docTree->hide();
    }


    // Create the folder tree view
    {
        TRect r = deskTop->getExtent();
        // Try to make it between 22 and 30 columns wide, and try to leave
        // at least 82 empty columns on screen (so that an editor view is
        // at least ~80 columns by default).
        if (r.b.x > 22)
            r.a.x = r.b.x - min(max(r.b.x - 82, 22), 30);
        folderTree = new FolderTreeWindow(r, &folderTree);
        folderTree->flags &= ~wfZoom;
        // The grow mode assumes it's placed on the right side of the screen.
        // Greater flexibility would require some trick or a dedicated class
        // for side views.
        folderTree->growMode = gfGrowLoX | gfGrowHiX | gfGrowHiY;
        folderTree->setState(sfShadow, False);
        deskTop->insert(folderTree);
        // Show by default only on large terminals.
        if (deskTop->size.x - folderTree->size.x < 82)
            folderTree->hide();
    }
}

TMenuBar *TurboApp::initMenuBar(TRect r)
{
    r.b.y = r.a.y+1;
    return new TMenuBar( r,
        *new TSubMenu( "~P~roject", kbAltP, hcNoContext ) +
			*new TMenuItem( "~N~ew Basic C Project", cmNewBasicCProject, kbNoKey, hcNoContext ) +
			*new TMenuItem( "~N~ew Basic SDL3 C Project", cmNewBasicSDL3CProject, kbNoKey, hcNoContext ) +
			*new TMenuItem( "~N~ew Basic GLFW C Project", cmNewBasicGLFWCProject, kbNoKey, hcNoContext ) +
			*new TMenuItem( "~N~ew SDL FrameBuffer Project", cmNewSDLFrameBufferProject, kbNoKey, hcNoContext ) +
        *new TSubMenu( "~F~ile", kbAltF, hcNoContext ) +
            *new TMenuItem( "~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl-N" ) +
            *new TMenuItem( "~O~pen", cmOpen, kbCtrlO, hcNoContext, "Ctrl-O" ) +
            *new TMenuItem( "Quick Open", cmOpenFileSearch, kbCtrlP, hcNoContext, "Ctrl-P" ) +
            *new TMenuItem( "Open Folder", cmOpenFolder, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~R~efresh Folder", cmRefreshFolder, kbAltF5, hcNoContext, "Alt-F5" ) +
            newLine() +
            *new TMenuItem( "~S~ave", cmSave, kbCtrlS, hcNoContext, "Ctrl-S" ) +
            *new TMenuItem( "S~a~ve As...", cmSaveAs, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~R~ename...", cmRename, kbF2, hcNoContext, "F2" ) +
            newLine() +
            *new TMenuItem( "~C~lose", cmCloseEditor, kbCtrlW, hcNoContext, "Ctrl-W" ) +
            *new TMenuItem( "Close All", cmCloseAll, kbNoKey, hcNoContext ) +
            newLine() +
            *new TMenuItem( "S~u~spend", cmDosShell, kbNoKey, hcNoContext ) +
            *new TMenuItem( "E~x~it", cmQuit, kbCtrlQ, hcNoContext, "Ctrl-Q" ) +
        *new TSubMenu( "~E~dit", kbAltE ) +
            *new TMenuItem( "~U~ndo", cmUndo, kbCtrlZ, hcNoContext, "Ctrl-Z" ) +
            *new TMenuItem( "Re~d~o", cmRedo, kbCtrlY, hcNoContext, "Ctrl-Y" ) +
            newLine() +
            *new TMenuItem( "Cu~t~", cmCut, kbCtrlX, hcNoContext, "Ctrl-X" ) +
            *new TMenuItem( "~C~opy", cmCopy, kbCtrlC, hcNoContext, "Ctrl-C" ) +
            *new TMenuItem( "~P~aste", cmPaste, kbCtrlV, hcNoContext, "Ctrl-V" ) +
            newLine() +
            *new TMenuItem( "~F~ind...", cmFind, kbCtrlF, hcNoContext, "Ctrl-F" ) +
            *new TMenuItem( "~R~eplace...",cmReplace, kbCtrlR, hcNoContext, "Ctrl-R" ) +
            *new TMenuItem( "~G~o to Line...",cmGoToLine, kbCtrlG, hcNoContext, "Ctrl-G" ) +
            *new TMenuItem( "Find ~N~ext", cmSearchAgain, kbF3, hcNoContext, "F3" ) +
            *new TMenuItem( "Find ~P~revious", cmSearchPrev, kbShiftF3, hcNoContext, "Shift-F3" ) +
        *new TSubMenu( "Se~l~ection", kbAltL ) +
            *new TMenuItem( "~T~oggle Comment", cmToggleComment, kbCtrlE, hcNoContext, "Ctrl-E" ) +
            newLine() +
            *new TMenuItem( "~U~ppercase", cmSelUppercase, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~L~owercase", cmSelLowercase, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~C~apitalize", cmSelCapitalize, kbNoKey, hcNoContext ) +
		*new TSubMenu( "~R~un", kbAltR ) +
			*new TMenuItem( "~B~uild", cmBuild, kbF6, hcNoContext, "F6") +
			*new TMenuItem( "Build for ~w~eb", cmBuildWeb, kbNoKey, hcNoContext) +
			*new TMenuItem( "~C~lean", cmClean, kbNoKey, hcNoContext) +
			*new TMenuItem( "~R~un", cmRun, kbF5, hcNoContext, "F5" ) +
			*new TMenuItem( "Run ~W~eb", cmRunWeb, kbF5, hcNoContext) +
			*new TMenuItem( "~D~ebug", cmDebug, kbNoKey, hcNoContext ) +
			*new TMenuItem( "~S~tep into", cmStepInto, kbF7, hcNoContext, "F7" ) +
			*new TMenuItem( "Step ~O~ver", cmStepOver, kbF8, hcNoContext, "F8" ) +
			*new TMenuItem( "Sto~p~", cmStop, kbShiftF5, hcNoContext, "Shift-F5" ) +
			*new TMenuItem( "Toggle ~B~reakpoint", cmToggleBreakpoint, kbF9, hcNoContext, "F9" ) +
        *new TSubMenu( "~W~indows", kbAltW ) +
			*new TMenuItem( "~Z~oom", cmZoom, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~R~esize/move",cmResize, kbCtrlF5, hcNoContext, "Ctrl-F5" ) +
            *new TMenuItem( "~N~ext", cmEditorNext, kbCtrlF6, hcNoContext, "Ctrl-F6" ) +
            *new TMenuItem( "~P~revious", cmEditorPrev, kbShiftF6, hcNoContext, "Shift-F6" ) +
            *new TMenuItem( "~C~lose", cmClose, kbEsc, hcNoContext, "Esc" ) +
            *new TMenuItem( "Previous (in tree)", cmTreePrev, kbAltUp, hcNoContext, "Alt-Up" ) +
            *new TMenuItem( "Next (in tree)", cmTreeNext, kbAltDown, hcNoContext, "Alt-Down" ) +
			*new TMenuItem( "~T~ile", cmTile, kbNoKey, hcNoContext ) +
	        *new TMenuItem( "C~a~scade", cmCascade, kbNoKey, hcNoContext ) +
        *new TSubMenu( "~S~ettings", kbAltS ) +
            *new TMenuItem( "Toggle Line ~N~umbers", cmToggleLineNums, kbNoKey, hcNoContext ) +
            *new TMenuItem( "Toggle Line ~W~rapping", cmToggleWrap, kbNoKey, hcNoContext ) +
            *new TMenuItem( "Toggle Auto ~I~ndent", cmToggleIndent, kbNoKey, hcNoContext ) +
            *new TMenuItem( "Toggle Document ~T~ree View", cmToggleTree, kbNoKey, hcNoContext ) +
            *new TMenuItem( "Toggle Document ~F~older Tree View", cmToggleFolderTree, kbNoKey, hcNoContext ) +
        *new TSubMenu( "~H~elp", kbAltH ) +
            *new TMenuItem( "~K~eyboard shortcuts", cmHelp, kbF1, hcNoContext, "F1" ) +
            *new TMenuItem( "~M~an page...", cmManPage, kbAltF1, hcNoContext, "Alt-F1" ) +
            newLine() +
            *new TMenuItem( "~A~bout...", cmAbout, kbNoKey, hcNoContext )
            );

}

TStatusLine *TurboApp::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( 0, kbAltX, cmQuit ) +
            *new TStatusItem( "~Ctrl-N~ New", kbNoKey, cmNew ) +
            *new TStatusItem( "~Ctrl-O~ Open", kbNoKey, cmOpen ) +
            *new TStatusItem( "~Ctrl-P~ Quick Open", kbCtrlP, cmOpenFileSearch ) +
            *new TStatusItem( "~Ctrl-S~ Save", kbNoKey, cmSave ) +
            *new TStatusItem( "~Esc~ Close", kbEsc, cmClose ) +
            *new TStatusItem( "~Ctrl-F6~ Next", kbCtrlF6, cmEditorNext ) +
            *new TStatusItem( "~F12~ Menu", kbF12, cmMenu ) +
            *new TStatusItem( 0, TKey(kbCtrlZ, kbShift), cmRedo ) +
            *new TStatusItem( 0, kbCtrlX, cmCut ) +
            *new TStatusItem( 0, kbCtrlC, cmCopy ) +
            *new TStatusItem( 0, kbCtrlV, cmPaste ) +
            *new TStatusItem( 0, kbShiftDel, cmCut ) +
            *new TStatusItem( 0, kbCtrlIns, cmCopy ) +
            *new TStatusItem( 0, kbShiftIns, cmPaste ) +
            *new TStatusItem( 0, kbShiftF6, cmEditorPrev ) +
            *new TStatusItem( 0, TKey('/', kbCtrlShift), cmToggleComment ) +
            *new TStatusItem( 0, TKey('_', kbCtrlShift), cmToggleComment ) +
            // *new TStatusItem( 0, kbF5, cmZoom ) +
            *new TStatusItem( 0, kbCtrlF5, cmResize )
            );
}

void TurboApp::shutDown()
{
    docTree = nullptr;
    clock = nullptr;
    TApplication::shutDown();
}

static std::string mi_unquote(const char *s)
{
    std::string res;
    if (*s == '"') s++;
    while (*s && *s != '\r' && *s != '\n') {
        if (*s == '"' && (*(s+1) == '\r' || *(s+1) == '\n' || *(s+1) == '\0')) break;
        if (*s == '\\') {
            s++;
            if (!*s) break;
            switch (*s) {
                case 'n': res += '\n'; break;
                case 'r': res += '\r'; break;
                case 't': res += '\t'; break;
                case '\\': res += '\\'; break;
                case '"': res += '"'; break;
                default: res += *s; break;
            }
        } else {
            res += *s;
        }
        s++;
    }
    return res;
}

void TurboApp::idle()
{
    TApplication::idle();
    if (clock)
	{
        clock->update();
	}

	if(BuildOutput::out != nullptr)
	{
		char buffer[1024];
		int n;
		while ((n = process_poll_output(&BuildOutput::runningProcess, buffer, sizeof(buffer))) > 0)
		{
			if (BuildOutput::isDebugging)
			{
				process_feed(&BuildOutput::runningProcess, buffer, n);
				char line[4096];
				while (process_getline(&BuildOutput::runningProcess, line, sizeof(line)))
				{
					char c = line[0];
					if (c == '@')
					{
						// Target stream output
						if (BuildOutput::progOut)
						{
							(*BuildOutput::progOut) << mi_unquote(line + 1);
							BuildOutput::progOut->flush();
						}
					}
					else if (c == '~' || c == '&')
					{
						// Console or Log stream output
						(*BuildOutput::out) << mi_unquote(line + 1);
						BuildOutput::out->flush();
					}
					else if (c == '^' || c == '*' || c == '+' || c == '=')
					{
						// MI Result or Notification records
						if (strncmp(line, "*stopped", 8) == 0)
						{
							process_locals(&BuildOutput::runningProcess);
						}
						else if (c == '^' && strstr(line, "variables=["))
						{
							if (BuildOutput::localsOut)
							{
								(*BuildOutput::localsOut) << "Local Variables:\n";
								(*BuildOutput::localsOut) << "----------------\n";
								
								const char *p = strstr(line, "variables=[");
								if (p) {
									p += 11; // Skip "variables=["
									while (*p && *p != ']') {
										const char *nameStart = strstr(p, "name=\"");
										const char *valueStart = strstr(p, "value=\"");
										
										if (nameStart && valueStart && (nameStart < valueStart || valueStart < nameStart)) {
											// Extract name
											const char *nS = strstr(p, "name=\"") + 6;
											const char *nE = strchr(nS, '"');
											// Extract value
											const char *vS = strstr(p, "value=\"") + 7;
											const char *vE = strchr(vS, '"');
											
											if (nE && vE) {
												std::string name(nS, nE - nS);
												std::string value(vS, vE - vS);
												(*BuildOutput::localsOut) << name << " = " << value << "\n";
												// Move p past this variable entry
												p = (nE > vE ? nE : vE) + 1;
												// Look for next entry
												const char *nextBrace = strchr(p, '{');
												if (nextBrace) p = nextBrace;
												else break;
											} else break;
										} else break;
									}
								}
								(*BuildOutput::localsOut) << "\n";
								BuildOutput::localsOut->flush();
							}
						}
						(*BuildOutput::out) << line;
						BuildOutput::out->flush();
					}
					else
					{
						// Likely raw program output (not wrapped by GDB)
						if (strncmp(line, "(gdb)", 5) == 0)
							continue;

						if (BuildOutput::progOut)
						{
							(*BuildOutput::progOut) << line;
							BuildOutput::progOut->flush();
						}
						else
						{
							(*BuildOutput::out) << line;
							BuildOutput::out->flush();
						}
					}
				}
			}
			else
			{
				(*BuildOutput::out) << buffer;
				BuildOutput::out->flush();
			}
		}
	}
}

void TurboApp::getEvent(TEvent &event)
{
    if (!argsParsed) {
        argsParsed = true;
        parseArgs();
    }
    TApplication::getEvent(event);
}

void TurboApp::handleEvent(TEvent &event)
{
    TApplication::handleEvent(event);
    bool handled = false;
    if (event.what == evCommand) {
        handled = true;
        switch (event.message.command) {
            case cmNew: fileNew(); break;
            case cmOpen: fileOpen(); break;
            case cmOpenFileSearch: fileSearch(); break;
            case cmOpenFolder: folderOpen(); break;

            case cmEditorNext:
            case cmEditorPrev:
                showEditorList(&event);
                break;
            case cmCloseAll: closeAll(); break;
            case cmToggleTree: toggleTreeView(); break;
            case cmToggleFolderTree: toggleFolderView(); break;
			case cmRefreshFolder:
				folderTree->tree->loadFromFolder(fs::current_path().string());
				break;
            case cmTreeNext:
                if (docTree)
                    docTree->tree->focusNext();
                break;
            case cmTreePrev:
                if (docTree)
                    docTree->tree->focusPrev();
                break;
            case cmAbout:
                TurboHelp::executeAboutDialog(*deskTop);
                break;
            case cmManPage:
                TurboHelp::executeManPageDialog(*deskTop);
                break;
            case cmHelp:
                TurboHelp::showOrFocusHelpWindow(*deskTop, (char*)event.message.infoPtr);
                break;
			case cmFolderTreeClick:
				{
					EditorWindow *editorWindowPtr;
					using FolderNode = FolderTreeView::FolderNode;
					auto node = (FolderNode*)event.message.infoPtr;
					auto ed = node->getEditor();

					if(ed == nullptr)
					{
						fileOpenOrNew(node->fullPath.c_str(), &editorWindowPtr);
						node->data = editorWindowPtr;
					}
					break;
				}
			case cmNewBasicCProject:
				NewBasicCProject();
				break;
			case cmNewBasicSDL3CProject:
				NewBasicSDL3CProject();
				break;
			case cmNewBasicGLFWCProject:
				NewBasicGLFWCProject();
				break;
			case cmNewSDLFrameBufferProject:
				NewSDLFrameBufferProject();
				break;
			case cmBuild:
				BuildOutput::show(*deskTop, ".", event.message.command);
				break;
			case cmBuildWeb:
				BuildOutput::show(*deskTop, ".", event.message.command);
				break;
			case cmClean:
				BuildOutput::show(*deskTop, ".", event.message.command);
				break;
			case cmRun:
				BuildOutput::show(*deskTop, ".", event.message.command);
				break;
			case cmRunWeb:
				runWeb();
				break;
			case cmDebug:
				BuildOutput::show(*deskTop, ".", event.message.command);
				break;
			case cmStepInto:
				if (BuildOutput::out) (*BuildOutput::out) << "Step into...\n";
				process_step_into(&BuildOutput::runningProcess);
				if (BuildOutput::out) BuildOutput::out->flush();
				break;
			case cmStepOver:
				if (BuildOutput::out) (*BuildOutput::out) << "Step over...\n";
				process_step_over(&BuildOutput::runningProcess);
				if (BuildOutput::out) BuildOutput::out->flush();
				break;
			case cmStop:
				if (BuildOutput::out) (*BuildOutput::out) << "Stopping...\n";
				process_kill(&BuildOutput::runningProcess);
				if (BuildOutput::out) BuildOutput::out->flush();
				break;
			case cmToggleBreakpoint:
				if (!MRUlist.empty())
				{
					auto &edWin = *MRUlist.next->self;
					auto &ed = edWin.getEditor();
					auto pos = ed.callScintilla(SCI_GETCURRENTPOS, 0, 0);
					auto line = ed.callScintilla(SCI_LINEFROMPOSITION, pos, 0);
					char loc[MAX_PATH];
					snprintf(loc, sizeof(loc), "%s:%ld", edWin.filePath().c_str(), line + 1);
					
					std::string sloc = loc;
					auto it = std::find(BuildOutput::breakpoints.begin(), BuildOutput::breakpoints.end(), sloc);
					if (it != BuildOutput::breakpoints.end())
					{
						BuildOutput::breakpoints.erase(it);
						// Note: GDB MI doesn't have a simple -break-delete by location string, 
						// usually needs ID. For now we just remove from our list for next run.
					}
					else
					{
						BuildOutput::breakpoints.push_back(sloc);
						if (process_is_running(&BuildOutput::runningProcess))
						{
							process_break(&BuildOutput::runningProcess, loc);
						}
					}
				}
				break;
            default:
                handled = false;
                break;
        }
    }
    if (handled)
        clearEvent(event);
}

void TurboApp::parseArgs()
{
    if (argc && argv) {
        auto *w = new TWindow(TRect(15, 8, 65, 19), "Please Wait...", wnNoNumber);
        w->flags = 0;
        w->options |= ofCentered;
        w->palette = wpGrayWindow;
        w->insert( new TStaticText(TRect(2, 2, 48, 3), "Opening file:"));
        auto *current = new TParamText(TRect(2, 3, 48, 9));
        w->insert(current);
        insert(w);

        std::string dirToLoad = "";
        std::vector<std::string> filesToOpen;

        for (int i = 1; i < argc; ++i) {
            if (fs::is_directory(argv[i])) {
                if (dirToLoad.empty())
                    dirToLoad = fs::absolute(argv[i]).string();
            } else {
                filesToOpen.push_back(fs::absolute(argv[i]).string());
            }
        }

        if (dirToLoad.empty()) {
            dirToLoad = fs::current_path().string();
        }

        current->setText("%s", dirToLoad.c_str());
        TScreen::flushScreen();
        chdir(dirToLoad.c_str());
        folderTree->tree->loadFromFolder(dirToLoad);

        for (const auto& file : filesToOpen) {
            current->setText("%s", file.c_str());
            TScreen::flushScreen();
            fileOpenOrNew(file.c_str());
        }

        remove(w);
        TObject::destroy(w);
    }
}

void TurboApp::fileNew()
{
    addEditor(createScintilla(), "");
}

void TurboApp::NewBasicCProject()
{
	ProjectCreator::create();
	folderTree->tree->loadFromFolder(fs::current_path());
}

void TurboApp::NewBasicSDL3CProject()
{
	ProjectCreator::createSDL();
	folderTree->tree->loadFromFolder(fs::current_path());
}

void TurboApp::NewBasicGLFWCProject()
{
	ProjectCreator::createGLFW();
	folderTree->tree->loadFromFolder(fs::current_path());
}

void TurboApp::NewSDLFrameBufferProject()
{
	ProjectCreator::createSDLFrameBuffer();
	folderTree->tree->loadFromFolder(fs::current_path());
}

void TurboApp::folderOpen()
{
	TView *d = validView( new TChDirDialog( 0, 666 ) );

    if( d != 0 ) {
        deskTop->execView( d );
        destroy(d);
		char path[MAXDIR] = {0}; 
		getcwd(path, MAXDIR);
		if(fs::is_directory(path))
		{
			folderTree->tree->reset();
			folderTree->tree->loadFromFolder(path);
		}
    }
}

void TurboApp::fileOpen()
{
    TurboFileDialogs dlgs {*this};
    turbo::openFile([&] () -> auto& {
        return createScintilla();
    }, [&] (auto &scintilla, auto *path) {
        addEditor(scintilla, path);
    }, dlgs);
}

void TurboApp::fileOpenOrNew(const char *path)
{
    char abspath[MAXPATH];
    strnzcpy(abspath, path, MAXPATH);
    fexpand(abspath);
    
    EditorWindow *ew = nullptr;
    MRUlist.forEach([&](EditorWindow *w) {
        if (w->filePath() == abspath)
            ew = w;
    });

    if (ew) {
        ew->focus();
    } else {
        auto &scintilla = createScintilla();
        if (turbo::readFile(scintilla, abspath, turbo::acceptMissingFilesOnOpen))
            addEditor(scintilla, abspath);
    }
}

void TurboApp::fileOpenOrNew(const char *path, EditorWindow** editorWindow)
{
    char abspath[MAXPATH];
    strnzcpy(abspath, path, MAXPATH);
    fexpand(abspath);

    EditorWindow *ew = nullptr;
    MRUlist.forEach([&](EditorWindow *w) {
        if (w->filePath() == abspath)
            ew = w;
    });

    if (ew) {
        ew->focus();
        if (editorWindow) *editorWindow = ew;
    } else {
        auto &scintilla = createScintilla();
        if (turbo::readFile(scintilla, abspath, turbo::acceptMissingFilesOnOpen))
            addEditor(scintilla, abspath, editorWindow);
    }
}

void TurboApp::closeAll()
{
    while (!MRUlist.empty()) {
        auto *head = MRUlist.next;
        message((EditorWindow *) head->self, evCommand, cmClose, 0);
        TScreen::flushScreen();
        if (MRUlist.next == head) // Not removed
            break;
    }
}

TRect TurboApp::newEditorBounds() const
{
    if (!MRUlist.empty())
        return MRUlist.next->self->getBounds();
    else {
        TRect r = deskTop->getExtent();
        if (docTree && docTree->state & sfVisible) {
            TRect t = docTree->getBounds();
            // Align left.
            if (t.a.x > r.b.x - t.b.x)
                r.b.x = max(t.a.x, EditorWindow::minSize.x);
            // Align right.
            else
                r.a.x = min(t.b.x, r.b.x - EditorWindow::minSize.x);
        }
        return r;
    }
}

turbo::TScintilla &TurboApp::createScintilla() noexcept
{
    return turbo::createScintilla();
}

void TurboApp::addEditor(turbo::TScintilla &scintilla, const char *path)
// Pre: 'path' is an absolute path.
{
    TRect r = newEditorBounds();
    auto &counter = fileCount[TPath::basename(path)];
    auto &editor = *new TurboEditor(scintilla, path);
    EditorWindow &w = *new EditorWindow(r, editor, counter, searchSettings, *this);
    if (docTree)
        docTree->tree->addEditor(&w);
    w.listHead.insert_after(&MRUlist);
    deskTop->insert(&w);
    enableCommands(editorCmds);
}

void TurboApp::addEditor(turbo::TScintilla &scintilla, const char *path, EditorWindow** editorWindow)
// Pre: 'path' is an absolute path.
{
    TRect r = newEditorBounds();
    auto &counter = fileCount[TPath::basename(path)];
    auto &editor = *new TurboEditor(scintilla, path);
    EditorWindow &w = *new EditorWindow(r, editor, counter, searchSettings, *this);
    if (docTree)
        docTree->tree->addEditor(&w);
    w.listHead.insert_after(&MRUlist);
    deskTop->insert(&w);
    enableCommands(editorCmds);
	*editorWindow = &w;
}

void TurboApp::showEditorList(TEvent *ev)
{
    EditorListModel model {MRUlist};
    TRect r {0, 0, 0, 0};
    r.b.x = min(max(ListModel::maxItemCStrLen(model) + 4, 40), deskTop->size.x - 10);
    r.b.y = min(max(model.size() + 2, 6), deskTop->size.y - 4);
    r.move((deskTop->size.x - r.b.x) / 2,
           (deskTop->size.y - r.b.y) / 4);
    ListWindow *lw = &ListWindow::create<EditorListView>(r, "Buffer List", model, lvScrollBars);
    if (ev)
        lw->putEvent(*ev);
    if (deskTop->execView(lw) == cmOK)
        if (auto *wnd = (EditorWindow *) lw->getCurrent())
            wnd->focus();

    destroy(lw);
}

void TurboApp::fileSearch()
{
    FileSearchListModel model;
    TRect r {0, 0, 0, 0};
    r.b.x = min(max(ListModel::maxItemCStrLen(model) + 4, 40), deskTop->size.x - 10);
    r.b.y = min(max(model.size() + 2, 6), deskTop->size.y - 4);
    r.move((deskTop->size.x - r.b.x) / 2,
           (deskTop->size.y - r.b.y) / 4);
    ListWindow *lw = &ListWindow::create(r, "Quick Open", model, lvScrollBars);
    if (deskTop->execView(lw) == cmOK)
    {
        if (auto *path = (std::string *) lw->getCurrent())
            fileOpenOrNew(path->c_str());
    }

    destroy(lw);
}

void TurboApp::toggleFolderView()
{
    if (!folderTree)
        return;
    // Prevent editors from doing draw() on each changeBounds(). We'll draw all
    // the views at the end.
    MRUlist.forEach([&] (auto *win) {
        win->setState(sfExposed, False);
    });
    TRect dr = folderTree->getBounds();
    if (folderTree->state & sfVisible) {
        folderTree->hide();
        MRUlist.forEach([this, dr] (auto *win) {
            TRect r = win->getBounds();
            if (r.a.x >= dr.b.x)
                r.a.x -= folderTree->size.x;
            else if (r.b.x <= dr.a.x)
                r.b.x += folderTree->size.x;
            win->locate(r);
        });
    } else {
        MRUlist.forEach([this, dr] (auto *win) {
            TRect r = win->getBounds();
            if (r.a.x + folderTree->size.x >= dr.b.x)
                r.a.x += folderTree->size.x;
            else if (r.b.x - folderTree->size.x <= dr.a.x)
                r.b.x -= folderTree->size.x;
            win->locate(r);
        });
        folderTree->show();
    }
    MRUlist.forEach([&] (auto *win) {
        win->setState(sfExposed, True);
    });
    deskTop->redraw();
}

void TurboApp::toggleTreeView()
{
    if (!docTree)
        return;
    // Prevent editors from doing draw() on each changeBounds(). We'll draw all
    // the views at the end.
    MRUlist.forEach([&] (auto *win) {
        win->setState(sfExposed, False);
    });
    TRect dr = docTree->getBounds();
    if (docTree->state & sfVisible) {
        docTree->hide();
        MRUlist.forEach([this, dr] (auto *win) {
            TRect r = win->getBounds();
            if (r.a.x >= dr.b.x)
                r.a.x -= docTree->size.x;
            else if (r.b.x <= dr.a.x)
                r.b.x += docTree->size.x;
            win->locate(r);
        });
    } else {
        MRUlist.forEach([this, dr] (auto *win) {
            TRect r = win->getBounds();
            if (r.a.x + docTree->size.x >= dr.b.x)
                r.a.x += docTree->size.x;
            else if (r.b.x - docTree->size.x <= dr.a.x)
                r.b.x -= docTree->size.x;
            win->locate(r);
        });
        docTree->show();
    }
    MRUlist.forEach([&] (auto *win) {
        win->setState(sfExposed, True);
    });
    deskTop->redraw();
}

void TurboApp::runWeb()
{
    const char *command = "make run_web";
    char buf[1024];
#ifdef _WIN32
    snprintf(buf, sizeof(buf), "start cmd /k \"%s\"", command);
#else
    snprintf(buf, sizeof(buf),
             "(x-terminal-emulator -e \"%s\" || "
             "gnome-terminal -- %s || "
             "konsole -e %s || "
             "xfce4-terminal -e \"%s\" || "
             "xterm -e \"%s\") &",
             command, command, command, command, command);
#endif
    int r = system(buf);
    (void) r;
}

void TurboApp::handleFocus(EditorWindow &w) noexcept
{
    // w has been focused, so it becomes the first of our MRU list.
    w.listHead.insert_after(&MRUlist);
    if (docTree)
        docTree->tree->focusEditor(&w);
    // We keep track of the most recent directory for file dialogs.
    if (!w.filePath().empty())
        mostRecentDir = TPath::dirname(w.filePath());
}

void TurboApp::handleTitleChange(EditorWindow &w) noexcept
{
    auto &counter = fileCount[TPath::basename(w.filePath())];
    if (&counter != w.fileNumber.counter)
    {
        w.fileNumber = {counter};
        if (docTree)
        {
            docTree->tree->removeEditor(&w);
            docTree->tree->addEditor(&w);
            docTree->tree->focusEditor(&w);
        }
    }
    if (!w.filePath().empty() && w.state & sfActive)
        mostRecentDir = TPath::dirname(w.filePath());
}

void TurboApp::removeEditor(EditorWindow &w) noexcept
{
    w.listHead.remove();
    if (MRUlist.empty())
        disableCommands(editorCmds);
    if (docTree)
    {
		folderTree->tree->focusEditor(&w);
		folderTree->tree->removeEditor(&w);
        docTree->tree->removeEditor(&w);
        // Removing the editor causes the focus to stay on the same position
        // but maybe not on the right element.
        if (!MRUlist.empty())
            docTree->tree->focusEditor(MRUlist.next->self);
    }
}

const char *TurboApp::getFileDialogDir() noexcept
{
    return mostRecentDir.c_str();
}
