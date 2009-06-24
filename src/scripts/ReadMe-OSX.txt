Hi!

To create OSX bundles that can be shipped as disk images (.dmg), the script mk-osx-bundle.sh may help.

The best way when you are in the source directory (called diplom) do:

# cd ..
# mkdir build
# ccmake ../diplom
do the configuration you need
# make
edit some of the paths in mk-osx-bundle, such as where wx-widgets and GLEW is lying around, then:
# . ../diplom/scripts/mk-osx-bundle.sh

you should have a self-containing NAME.dmg lying around now that contains all the required frameworks.
