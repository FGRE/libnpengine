libnpengine
===========

libnpengine is a free implementation of Nitroplus visual novel game engine.
It can currently run [Steins;Gate][2].

If you need help getting this to compile and run or simply want to chat, join #FGRE @ irc.freenode.net

Building
--------

It is the choice of the Steins Gate if you can build from master branch today:
[![Travis Build Status](https://travis-ci.org/FGRE/libnpengine.svg?branch=master)][1]

    $ git clone https://github.com/FGRE/libnpengine.git
    $ cd libnpengine

optionally, check out latest tag, if master branch status is red:

    $ git checkout `git describe --tags --abbrev=0`

now compile:

    $ cmake .
    $ make

[1]: https://travis-ci.org/FGRE/libnpengine
[2]: https://github.com/FGRE/steins-gate
