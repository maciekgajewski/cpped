# Project diary

## 28-11-2015

Everyone seems to be obesessed with 'frameworks'. Every IDE or rick editor I know is trying to be a 'framework' for 'plugins' providing additional 'features'.
As a result, we get this:
* Extensible product with many features, most of them barely working,
* Big, heavy framework whihc takes ages to do the most simple thing.

QtCreator used to be lightweight, but since 3.3 it's unusably slow. I'm tryuing to read the code, and now I knonw why it is so: 95% of the code is boilerplate, plumbing, framework. The remaining 5% of actual business logic is lost in there.
Vim is fast and powerful, but makes for poor IDE. Any C++-related pluing provides only part of the functionality.
KDevelop suffered from feature overblow, I haven't looked at the new version yet (5.x); judging by the release announcement, things have imporved.

So, sins to avoid:
* too much extensibility and flexibility. This causes the boilerplate part of the code overshadow the business logic
* too many features; being jack of al trades prevents from being master of one.

I was working on first ncurses interface, and I had to resist the urge to make it flexible, to reduce exposure to ncurses. I must force myself to let go, to go full-ncurses, full clang.

## 30-11-2015

I struck gold thi weekend: libclang. Simple, stable API to parse C++ and provide facilities for syntax highlighting and code completion. Much easier than playing with the mess of clang internas.
Unfortunately, it's C. I've found a nice C++ wrapper in clang-tags project, but it was not suitable for my needs. I'm writing my own. C++ wrapper ofr C APi for C++ code. ZeroMQ again.

## 01-12-2105

The editor supports only ASCII. There is no trivial way to provide support for unicode. I can't convert the data to wchar_t* on loading, as I need undisturbed char* buffer for parsingg with libclang.
I came up with an idea of container wrapping char utf-8 data, ut it would be a non-random-access container. Should be fine, as I manipualte one line at the time. I'll implement it one day.

## 20-12-2015

YouCompleteMe proven to be a great source of insipirartion. Why very little (if any) of the actual code is re-usable for me, it provides great examples on how to deal with liblcang quirks. Most importantly, it valdiates libclang as the tool to use for C++ parsing.

## 23-12-2015

I decided loggign is the easiest way to see what going on in the backend process. I was contemplating some UI feature, but not now, not now.
I was trying to use Boost.Logging, but it turned out to be such a pint in the ass: unpleasent to use, unpleasant to build.

# 16-01-2016

So, the biggest blocket for achieving grateeditor responsivness was the factm that parsing a file from scratch takes between 2-4 seconds, and it stalled the backend process.
I just had an ides: spawn on, or even few more processes. The only goal for the parins of the _entire_ project, is to build to code model. This can be done in yet another worker, 
and the result can be transferred back.
Disk caching or buildign of PCH (if this makes sense) still may be requiired, to not go trough the same chores whenver the project is opened.

# 06-02-2016 

Having goodcode structure is paying off. At some point I was thinking: "dude, you spent way too much time refactoring; push for features", but now, with a solid code structure, adding new features is a breeze!