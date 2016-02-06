# cpped
An efficient C++ editor

## The goal
The goal of the project is to create a fast, efficient, productive C++ editor.

## 0.1 (Alpha preview 1) release notes
The first release is nothing more than a preview. 
The features are but a few, the interface is rough and minimalistic, the performance is not what I plan it to be.

### Known limitations
* Doesn't work with PuTTY, PuTTY doesn't send correct sequences for some fancy keyboard shotcurs. Use another SSH client. mobaXterm works pretty well, just diasable 'Backspace sends ^H'. 
* Works only with ASCII files with UNIX line endings

### Requirements
* To build, clang 3.7 lib is needed. Binary package can be downloaded from http://llvm.org/releases/
* To get the C++ parsing fully working, clang++-3.7 must be in your path when the editor is run.

## Planned features

* Rich, terminal interface, using unicode chars, 256 colors and mouse
* Responisve user interface
* CMake project support
* Smooth learing curve, good feature discoverability
* Clang-based C++ parsing, code completion and as-you-type diagnostics
* Advanced code navigation
* Integrated bulding and running
* Debugger integration
* Spell checking
* Vim mode

## Features that I'm not planning to implement

* UML diagram genertor
* Plugins
* Python/Lua API
* VCS integration
* Integrated web broser, mail client and chess program
* Qt/Gtk/Web UI
* Port to exotic OSes
