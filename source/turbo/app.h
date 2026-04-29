#ifndef TURBO_APP_H
#define TURBO_APP_H

#define Uses_TApplication
#define Uses_TFileDialog
#include <tvision/tv.h>

#include <turbo/editstates.h>
#include "doctree.h"
#include "foldertree.h"
#include "apputils.h"
#include "editwindow.h"
#include "cmds.h"

struct EditorWindow;
class TClockView;

struct TurboApp : public TApplication, EditorWindowParent
{

    FileCounter fileCount;
    list_head<EditorWindow> MRUlist;
    TClockView *clock;
    DocumentTreeWindow *docTree;
    FolderTreeWindow *folderTree;
    TCommandSet editorCmds;
    bool argsParsed {false};
    int argc;
    const char **argv;
    turbo::SearchSettings searchSettings;
    std::string mostRecentDir;

    TurboApp(int argc, const char **argv) noexcept;
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);

    void shutDown() override;
    void idle() override;
    void getEvent(TEvent &event) override;
    void handleEvent(TEvent& event) override;
    void parseArgs();

    void fileNew();
    void fileOpen();
	void folderOpen();
    void fileOpenOrNew(const char *path);
	void fileOpenOrNew(const char *path, EditorWindow** editorWindow);
    void closeAll();
	void NewBasicCProject();
	void NewBasicSDL3CProject();
	void NewBasicGLFWCProject();
	void NewSDLFrameBufferProject();
    TRect newEditorBounds() const;
    turbo::TScintilla &createScintilla() noexcept;
    void addEditor(turbo::TScintilla &, const char *path);
    void addEditor(turbo::TScintilla &, const char *path, EditorWindow** w);
    void showEditorList(TEvent *ev);
    void toggleTreeView();
    void toggleFolderView();

    void handleFocus(EditorWindow &w) noexcept override;
    void handleTitleChange(EditorWindow &w) noexcept override;
    void removeEditor(EditorWindow &w) noexcept override;
    const char *getFileDialogDir() noexcept override;
};

#endif
