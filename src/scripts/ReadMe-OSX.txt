Hi!

To create OSX bundles that can be shipped as disk images (.dmg), the script mk-osx-bundle.sh may help.

To use it, make sure you have a compiled version of the Fibernavigator, then go to your build directory, then call the script.

The script needs at least two parameters, which are:
$1: the path to the Fibernavigator binary (typically: bin/fibernavigator)
$2: the path to the wxWidgets build that was used to compile the Fibernavigator. It should be the same path as the one that was set in the CMake configuration.

An example call on a developer system, when in the build directory, would look like
# . ../src/scripts/mk-osx-bundle.sh bin/fibernavigator /path/to/wxWidgets/root/directory/build-release/

There is an additional optional parameter that, when set to 1, tells to script to use a more verbose output.

If all works, you should have a self-containing Fibernavigator.dmg disk image that contains all the required frameworks.