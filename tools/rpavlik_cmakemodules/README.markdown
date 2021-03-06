VRAC CMake Modules
==================

Ryan Pavlik

<rpavlik@iastate.edu> <abiryan@ryand.net>
<http://academic.cleardefinition.com>

Iowa State University HCI Graduate Program/VRAC

Introduction
------------

This is a collection of CMake modules that I've produced during the course
of a variety of software development.  There are a number of find modules,
especially for virtual reality and physical simulation packages, some utility
modules of more general interest, and some patches or workarounds for
CMake itself.

Each module is generally documented, and depending on how busy I was
when I created it, the documentation can be fairly complete.

How to Integrate
----------------

These modules are probably best placed wholesale into a "cmake" subdirectory
of your project source.

If you use Git, try installing [git-subtree][1] (included by default on
Git for Windows and perhaps for your Linux distro), so you can easily
use this repository for subtree merges, updating simply.

For the initial checkout:

	cd projectdir

	git subtree add --squash --prefix=cmake git://github.com/rpavlik/cmake-modules.git master

For updates:

	cd projectdir

	git subtree pull --squash --prefix=cmake git://github.com/rpavlik/cmake-modules.git master

If you use some other version control, you can export a copy of this directory
without the git metadata by calling:

    ./export-to-directory.sh yourprojectdir/cmake

You might also consider exporting to a temp directory and merging changes, since
this will not overwrite by default.  You can pass -f to overwrite existing files.

How to Use
----------

At the minimum, all you have to do is add a line like this near the top
of your root CMakeLists.txt file (but not before your project() call):

	list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

You might also want the extra automatic features/fixes included with the
modules, for that, just add another line following the first one:

	include(UseBackportedModules)

Look at module-help.html/txt (generated by update-help.sh on a unix-like shell)
either in this directory or online at <http://github.com/rpavlik/cmake-modules/blob/master/module-help.txt>
for more information on individual modules.


Licenses
--------

The modules that I wrote myself are all subject to this license:

> Copyright Iowa State University 2009-2013
>
> Distributed under the Boost Software License, Version 1.0.
>
> (See accompanying file `LICENSE_1_0.txt` or copy at
> <http://www.boost.org/LICENSE_1_0.txt>)

Modules based on those included with CMake are under the OSI-approved
BSD license, which is included in each of those modules.  A few other modules
are modified from other sources - when in doubt, look at the .cmake.

Important License Note!
-----------------------

If you find this file inside of another project, rather at the top-level
directory, you're in a separate project that is making use of these modules.
That separate project can (and probably does) have its own license specifics.




[1]: http://github.com/apenwarr/git-subtree  "Git Subtree master"
