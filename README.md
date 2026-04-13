
![PLUSTURB](https://github.com/user-attachments/assets/53164835-0415-4921-b471-4be50ad76ec0)


# +Turbo IDE

+Turbo is a fork from the original [Turbo Editor](https://github.com/magiblot/turbo) project. Its a mixture of 3 projects: [TurboVision App framework](https://github.com/magiblot/tvision), [Scintilla](https://www.scintilla.org/index.html) (for sintax highlighting), and , as I said, [Turbo Editor](https://github.com/magiblot/turbo) for the already implemented multi document editor funcitonality and sintax highlighting.

It's an IDE. Its not an editor. It has integration with debugger, help for specific functions (only in C trough man pages), and project management.

The reason to make a custom fork was to have more freedom to change stuff and don't bother the repo owner with pull requests that can make the development cycle take longer.

It adds the following functionality that I wanted for myself:

- [x] Remove the Line numbers by default (can be re enabled).
- [x] Default color scheme matching the old Borland Turbo C++.
- [x] File browser shows the contents of the current folder. Not only the open files.
- [ ] Integration with GDB.
- [x] Integration with (and only with)Makefiles. Including saving project preferences in special comments.
- [x] Project templates, allows to create C projects that uses Opengl, SDL3 or GLFW and compiles to binary or to a browser using emscripten.
- [ ] Installing libraries needed for the project templates.

# But why?

I wanted an IDE that doesn't bother you with 

* Popups asking for updates.
* Pry to sell you something.
* Recomends you plugins.
* Has autocomplete.
* NOT having a proper integration with a debugger (there are many editors out there, but not much IDE's).
* Displays a help when pressing F1 over a function name (through man pages).
* Is written in C/C++.
* It's just one file.
* Doesn't have dependencies (actually, all dependencies are included as part of the code).

# FAQ

## Can I contribute with the project?

No, Pull Requests are closed.

## What if I found a bug and I can enumerate the steps to reproduce it?

Don't use the program that way.

## I like the project, what if I want to use it but I need a functionality?

Fork it and change it. It should be easy enough to modify the code and customize it to your needs or liking.

## What's the license for this software?

I don't know.

## Building

Clone the repo.

Then, make sure the following dependencies are installed:

* CMake.
* A compiler supporting C++17.
* `libncursesw` (note the 'w') (Unix only).

Additionally, you may also want to install these optional dependencies:

* `libmagic` for better recognition of file types (Unix only).
* `libgpm` for mouse support on the linux console (Linux only).
* `xsel`, `xclip` and/or `wl-clipboard` for system clipboard integration (Unix only, except macOS).

Turbo can be built with the following commands:

```sh
cmake . -DCMAKE_BUILD_TYPE=Release && # Or 'RelWithDebInfo', or 'MinSizeRel', or 'Debug'.
cmake --build .
```

The above will generate the `turbo` binary.

<details>
<summary><b>Detailed build instructions for Ubuntu 20.04</b></summary>

```sh
sudo apt update && sudo apt upgrade
sudo apt install build-essential cmake gettext-base git libgpm-dev libmagic-dev libncursesw5-dev xsel
git clone --recursive https://github.com/magiblot/turbo.git
cd turbo
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j$(nproc) # Build Turbo.
sudo cp turbo /usr/local/bin/ # Install (optional).
```
</details>
<details>
<summary><b>Detailed build instructions for Ubuntu 18.04</b></summary>

```sh
sudo apt update && sudo apt upgrade
sudo apt install build-essential cmake g++-8 gettext-base git libgpm-dev libmagic-dev libncursesw5-dev xsel
git clone --recursive https://github.com/magiblot/turbo.git
cd turbo
CXX=g++-8 cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j$(nproc) # Build Turbo.
sudo cp turbo /usr/local/bin/ # Install (optional).
```
</details>

## Usage

### From the command line

* **turbo** [*file*...]

In order to open several files in a directory tree you should use wildcards or subcommands, if they are supported by your command shell. Also, you can open a folder using the same logic. For example, in Unix:

```sh
# Open all .c and .h files in the current directory and its subdirectories
turbo `find . -type f -name '*.c' -o -name '*.h'`
# Open all files in the current directory and its subdirectories, excluding executables and hidden files or directories
turbo `find . -type f \! -executable \! -path '*/.*'`
# Open the current folder.
turbo .
```

### In-app

As said earlier, Turbo has been designed to be intuitive. So you probably already know how to use it!

Some keybindings are:

* `Ctrl+C`/`Ctrl+Ins`: copy.
* `Ctrl+V`/`Shift+Ins`: paste.
* `Ctrl+X`/`Shift+Del`: cut.
* `Ctrl+Z`, `Ctrl+Y`: undo/redo.
* `Tab`, `Shift+Tab`: indent/unindent.
* `Ctrl+E`: toggle comment.
* `Ctrl+A`: select all.
* `Shift+Arrow`: extend selection.
* `Ctrl+F`: find.
* `Ctrl+R`: replace.
* `Ctrl+G`: go to line.
* `Ctrl+Back`/`Alt+Back`, `Ctrl+Del`: erase one word left/right.
* `Ctrl+Left`/`Alt+Left`, `Ctrl+Right`/`Alt+Right`: move one word left/right.
* `Ctrl+Shift+Up`/`Alt+Shift+Up`, `Ctrl+Shift+Down`/`Alt+Shift+Down`: move selected lines up/down.
* `Ctrl+N`: create new document.
* `Ctrl+O`: "open file" dialog.
* `Ctrl+S`: save document.
* `Ctrl+W`: close focused document.
* `F6`, `Shift+F6`: next/previous document (in MRU order).
* `Ctrl+Q`/`Alt+X`: exit the application.

In environments with extended keyboard support (e.g. the Linux console, Windows or Kitty ≥ 0.20.0), the following key shortcuts may also work:

* `Ctrl+Shift+Z`: redo.
* `Ctrl+Tab`/`Alt+Tab`, `Ctrl+Shift+Tab`/`Alt+Shift+Tab`: next/previous document (in MRU order).
* `Shift+Enter`: find previous (in the "find" text box).
* `Ctrl+/`/`Ctrl+_`: toggle comment.

Support for these key combinations may vary among terminal applications, but any issue on this should be reported to [Turbo Vision](https://github.com/magiblot/tvision/issues) instead.

### Clipboard support

See the Turbo Vision [documentation](https://github.com/magiblot/tvision#clipboard).

## Features

Scintilla has [lots of features](https://www.scintilla.org/ScintillaDoc.html), of which Turbo only offers a few. Making more of them available is just a matter of time, so contributions are welcome.

Below is a TO-DO list of features I would like to implement in Turbo:

- [x] Several files open at the same time.
- [x] Line numbers.
- [x] Word wrap.
- [x] Suspend to shell.
- [x] Unicode in documents (in particular, UTF-8).
- [x] Double-width characters.
- [x] Opening binary files without freaking out.
- [x] List of open documents in MRU order.
- [x] Tree view of open documents.
- [x] Tree view sorted alphabetically.
- [x] Case-insensitive search.
- [x] Find as you type.
- [x] Replace.
- [x] Go to line.
- [ ] List of recently opened files.
- [x] Remove trailing whitespaces on save.
- [x] Ensure newline at end of file.
- [ ] Detect open files modified on disk.
- [ ] Persistent configuration.
- [ ] Keybinding customization.
- [ ] Color scheme customization.
- [x] Syntax highlighting for some languages (C/C++, Rust, Python, JavaScript, Make, Bash, Ruby, JSON, YAML, HTML, INI, Go, PHP).
- [ ] Syntax highlighting for the rest of [languages supported by Scintilla](https://github.com/ScintillaOrg/lexilla/blob/master/include/SciLexer.h).
- [x] Comment toggling.
- [x] Brace match highlighting.
- [ ] VIM input mode.
- [ ] Localization.
- [x] Integration with the system clipboard.
- [x] Unicode in dialog text boxes (this depends on Turbo Vision).
- [ ] True Color support (this depends on Turbo Vision).
