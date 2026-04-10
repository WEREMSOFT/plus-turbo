#include "help.h"
#include <tvision/help.h>

#define Uses_MsgBox
#define Uses_TButton
#define Uses_TDialog
#define Uses_TGroup
#define Uses_TStaticText
#include <tvision/tv.h>

#include "cmds.h"
#include <sstream>

#include <array>
#include "editwindow.h"
static constexpr TStringView aboutDialogText =
    "\003+Turbo"
#ifdef TURBO_VERSION_STRING
    " (build " TURBO_VERSION_STRING ")"
#endif
    "\n\n"
    "\003A text editor based on Turbo, Scintilla and Turbo Vision\n\n";

// Since we do not need cross-references, we can easily create a THelpFile
// on-the-fly and define topics manually instead of using TVHC.

static constexpr TStringView helpParagraphs[] =
{
    "  Keyboard shortcuts ▄\n"
    " ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀",
    "This table shows +Turbo's commands and their associated keyboard shortcuts."
    "\n\n",
    "Some commands can be triggered by more than one shortcut. Some keyboard "
    "shortcuts may not be supported by the console.\n\n",
    " ┌─────────────┬────────────────────────┬────────────────────┐\n"
    " │  Category   │        Command         │     Shortcuts      │\n"
    " ╞═════════════╪════════════════════════╪════════════════════╡\n"
    " │ Application │ Focus menu             │ F12                │\n"
    " │ control     │ Exit                   │ ┬─ Ctrl+Q          │\n"
    " │             │                        │ └─ Alt+X           │\n"
    " ├─────────────┼────────────────────────┼────────────────────┤\n"
    " │ File        │ New                    │ Ctrl+N             │\n"
    " │ management  │ Open                   │ Ctrl+O             │\n"
    " │             │ Save                   │ Ctrl+S             │\n"
    " │             │ Close                  │ Ctrl+W             │\n"
    " │             │ Rename                 │ F2                 │\n"
    " ├─────────────┼────────────────────────┼────────────────────┤\n"
    " │ Editing     │ Copy                   │ ┬─ Ctrl+C          │\n"
    " │             │                        │ └─ Ctrl+Ins        │\n"
    " │             │ Paste                  │ ┬─ Ctrl+V          │\n"
    " │             │                        │ └─ Shift+Ins       │\n"
    " │             │ Cut                    │ ┬─ Ctrl+X          │\n"
    " │             │                        │ └─ Shift+Del       │\n"
    " │             │ Undo                   │ Ctrl+Z             │\n"
    " │             │ Redo                   │ ┬─ Ctrl+Y          │\n"
    " │             │                        │ └─ Ctrl+Shift+Z    │\n"
    " │             │ Indent                 │ Tab                │\n"
    " │             │ Unindent               │ Shift+Tab          │\n"
    " │             │ Toggle comment         │ ┬─ Ctrl+E          │\n"
    " │             │                        │ ├─ Ctrl+/          │\n"
    " │             │                        │ └─ Ctrl+_          │\n"
    " │             │ Select all             │ Ctrl+A             │\n"
    " │             │ Duplicate selection    │ Ctrl+D             │\n"
    " │             │ Delete current line    │ Ctrl+K             │\n"
    " │             │ Cut current line       │ Ctrl+L             │\n"
    " │             │ Word left              │ ┬─ Ctrl+Left       │\n"
    " │             │                        │ └─ Alt+Left        │\n"
    " │             │ Word right             │ ┬─ Ctrl+Right      │\n"
    " │             │                        │ └─ Alt+Right       │\n"
    " │             │ Erase word left        │ ┬─ Ctrl+Back       │\n"
    " │             │                        │ └─ Alt+Back        │\n"
    " │             │ Erase word right       │ ┬─ Ctrl+Del        │\n"
    " │             │                        │ └─ Alt+Del         │\n"
    " │             │ Move lines up          │ ┬─ Ctrl+Shift+Up   │\n"
    " │             │                        │ └─ Alt+Shift+Up    │\n"
    " │             │ Move lines down        │ ┬─ Ctrl+Shift+Down │\n"
    " │             │                        │ └─ Alt+Shift+Down  │\n"
    " │             │ Scroll up one line     │ Ctrl+Up            │\n"
    " │             │ Scroll down one line   │ Ctrl+Down          │\n"
    " │             │ 'Find' panel           │ Ctrl+F             │\n"
    " │             │ Find next              │ F3                 │\n"
    " │             │ Find previous          │ Shift+F3           │\n"
    " │             │ 'Replace' panel        │ Ctrl+R             │\n"
    " │             │ 'Go To Line' panel     │ Ctrl+G             │\n"
    " ├─────────────┼────────────────────────┼────────────────────┤\n"
    " │ Window      │ Next editor (MRU)      │ ┬─ F6              │\n"
    " │ management  │                        │ ├─ Ctrl+Tab        │\n"
    " │             │                        │ └─ Alt+Tab         │\n"
    " │             │ Previous editor (MRU)  │ ┬─ Shift+F6        │\n"
    " │             │                        │ ├─ Ctrl+Shift+Tab  │\n"
    " │             │                        │ └─ Alt+Shift+Tab   │\n"
    " │             │ Next editor (Tree)     │ Alt+Down           │\n"
    " │             │ Previous editor (Tree) │ Alt+Up             │\n"
    " └─────────────┴────────────────────────┴────────────────────┘",
};

void TurboHelp::executeAboutDialog(TGroup &owner) noexcept
{
    TDialog *aboutBox = new TDialog(TRect(0, 0, 39, 12), "About");

    aboutBox->insert(
        new TStaticText(TRect(2, 2, 37, 8), aboutDialogText)
    );

    aboutBox->insert(
        new TButton(TRect(14, 9, 26, 11), "OK", cmOK, bfDefault)
    );

    aboutBox->options |= ofCentered;

    owner.execView(aboutBox);

    TObject::destroy(aboutBox);
}

// Use a stringbuf to store the help file contents. We use inheritance to
// ensure that the stringbuf's lifetime exceeds that of the THelpFile.
class InMemoryHelpFile : private std::stringbuf, public THelpFile
{
public:

    InMemoryHelpFile() noexcept;
};

InMemoryHelpFile::InMemoryHelpFile() noexcept :
    THelpFile(*new iopstream(this))
{
}

// Inherit THelpWindow to be able to handle the cmFindHelpWindow command.
class TurboHelpWindow : public THelpWindow
{
public:

    TurboHelpWindow(THelpFile &helpFile) noexcept;

    void handleEvent(TEvent &ev) override;
};

TurboHelpWindow::TurboHelpWindow(THelpFile &helpFile) noexcept :
    TWindowInit(&initFrame),
    THelpWindow(&helpFile, hcNoContext)
{
    state &= ~sfShadow;
}

void TurboHelpWindow::handleEvent(TEvent &ev)
{
    THelpWindow::handleEvent(ev);

    if (ev.what == evBroadcast && ev.message.command == cmFindHelpWindow)
        clearEvent(ev);
}

static THelpFile &createInMemoryHelpFile(TSpan<const TStringView> paragraphs) noexcept
{
    auto &helpTopic = *new THelpTopic;
    for (TStringView paragraphText : paragraphs)
    {
        paragraphText = paragraphText.substr(0, USHRT_MAX);
        auto &paragraph = *new TParagraph;
        paragraph.text = newStr(paragraphText);
        paragraph.size = (ushort) paragraphText.size();
        paragraph.wrap = (paragraphText.size() > 0 && paragraphText[0] != ' ');
        paragraph.next = nullptr;
        helpTopic.addParagraph(&paragraph);
    }

    auto &helpFile = *new InMemoryHelpFile;
    helpFile.recordPositionInIndex(hcNoContext);
    helpFile.putTopic(&helpTopic);

    return helpFile;
}

void TurboHelp::showOrFocusHelpWindow(TGroup &owner, char* selectedText) noexcept
{
    auto *helpWindow =
        (TurboHelpWindow *) message(&owner, evBroadcast, cmFindHelpWindow, nullptr);

    if (helpWindow == 0)
    {
        std::string result;
        bool found = false;

        // Try sections 3, 2, and then no section
        if (selectedText != nullptr && strlen(selectedText) != 0)
        {
            const char* sections[] = {"3", "2", ""};
            for (const char* section : sections)
            {
                char cmd[128] = {0};
                if (strlen(section) > 0)
                    snprintf(cmd, sizeof(cmd), "man %s %s 2>/dev/null", section, selectedText);
                else
                    snprintf(cmd, sizeof(cmd), "man %s 2>/dev/null", selectedText);

                FILE* pipe = popen(cmd, "r");
                if (pipe)
                {
                    std::array<char, 256> manPage;
                    std::string output;
                    while (fgets(manPage.data(), manPage.size(), pipe) != nullptr)
                    {
                        output += manPage.data();
                    }
                    pclose(pipe);

                    // Check if we actually got output (man page found)
                    if (!output.empty())
                    {
                        result = std::move(output);
                        found = true;
                        break;
                    }
                }
            }
        }

        // Fallback logic
        if (!found)
        {
            // If no text was selected, show general help
            if (selectedText == nullptr || strlen(selectedText) == 0)
            {
                for (const auto &p : helpParagraphs)
                {
                    result += p.data();
                }
            }
            else
            {
                // If text was selected but no man page was found in any section
                result = "No man page found for \"";
                result += selectedText;
                result += "\"";
            }
        }

        TStringView sv{result.data(), result.size()};
        THelpFile &helpFile = createInMemoryHelpFile(TSpan<const TStringView>(&sv, 1));

        helpWindow = new TurboHelpWindow(helpFile);

        // Resize the Help Window so that:
        // - It fits into the owner view.
        // - It is wide enough for the topic contents to fit, if possible.
        THelpViewer *helpViewer = (THelpViewer *) helpWindow->first();
        THelpTopic *topic = helpViewer->topic;
        int topicWidth = max(helpViewer->size.x, topic->longestLineWidth());
        int marginWidth = helpWindow->size.x - helpViewer->size.x;
        int windowWidth = topicWidth + marginWidth;
        TRect bounds = owner.getExtent();
        bounds.b.x = min(bounds.b.x, windowWidth);
        // THelpWindow has 'ofCentered' set, so it will be centered automatically.
        helpWindow->changeBounds(bounds);

        owner.insert(helpWindow);
    }

    helpWindow->focus();
}
