# Project diary

## 28-11-205

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


