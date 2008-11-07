#!/bin/bash

#
# This script creates a max osx bundle and a disk image (.dmg) file
# out of a compiled FAnToM version
#


#echo $1
#
# change these defines to what you want to build a bundle from
export EXE=main
export BUNDLEDIR=$EXE.app
export EXEC_DIR=$BUNDLEDIR/Contents/MacOS
export FRAMEWORK_DIR=$BUNDLEDIR/Contents/Frameworks
export RESOURCE_DIR=$BUNDLEDIR/Contents/Resources
export SOURCE=../diplom/

# make required directories
mkdir -p $EXEC_DIR
mkdir -p $FRAMEWORK_DIR

# copy libraries into framework directory (raw files, we do not bundle them, yet)
#cp -Rf lib/* $FRAMEWORK_DIR

# all main libraries go here, we need to relink everything
#LIBS="libcommon.dylib libmath.dylib libanalysis.dylib libalgo.dylib libvisKernel.dylib libnogui.dylib libgui.dylib libFge.dylib libdataSet.dylib libqtgui-qt4.dylib libFgeQT4.dylib libqtwidgets.dylib libvisualization.dylib"

#EXTLIBS="/sw/lib/libboost_python.dylib"
#EXTLIBS2="libboost_python.dylib"

# copy additional libraries
EXTLIBS=("/usr/lib/libwx_macud-2.8.0.dylib" "/usr/lib/libwx_macud_gl-2.8.0.dylib" "/usr/lib/libGLEW.1.5.0.dylib")
EXTLIBN=("libwx_macud-2.8.0.dylib" "libwx_macud_gl-2.8.0.dylib" "libGLEW.1.5.0.dylib")

echo "==========================>> $EXTLIBS"
for i in ${EXTLIBS[@]}
do
  echo ===============================
  echo ===============================
  echo `basename $i`
  cp $i $FRAMEWORK_DIR/
done

# relink executable to its external libraries that we
# ship with the bundle
echo "+== relinking binary to external libs"
libcount=${#EXTLIBS[@]} 
for (( j = 0 ; j < libcount ; j++ )) 
do
  mlib=${EXTLIBS[$j]} 
  mbase=${EXTLIBN[$j]}
  echo "  processing $mlib..."

  install_name_tool \
  -id @executable_path/../Frameworks/$mbase \
  $FRAMEWORK_DIR/$mbase  #was $i

  install_name_tool \
  -change $mlib \
  @executable_path/../Frameworks/$mbase \
  $EXEC_DIR/$EXE
done



# relink libraries and binary
echo "+== relinking all libraries"
for i in `ls -1 $FRAMEWORK_DIR`
do
  echo " == relinking library: $i"
  #set name=`basename $i`
  # echo "name: $name"
  #echo $EXEC_DIR/$EXE

  # all maindir relative libraries
  for lib in $LIBS
  do
	install_name_tool \
	-change `pwd`/lib/$lib \
	@executable_path/../Frameworks/$lib \
	$FRAMEWORK_DIR/$i
  done

  # do the same for external libraries copied into FAnToM
  libcount=${#EXTLIBS[@]} 
  for (( j = 0 ; j < libcount ; j++ )) 
  do
	mlib=${EXTLIBS[$j]} 
	mbase=${EXTLIBN[$j]}
	echo "  processing $lib..."
	
	install_name_tool \
	-change $mlib \
	@executable_path/../Frameworks/$mbase \
	$FRAMEWORK_DIR/$i
  done

  install_name_tool \
  -id @executable_path/../Frameworks/$i \
  $FRAMEWORK_DIR/$i

  install_name_tool \
  -change `pwd`/lib/$i \
  @executable_path/../Frameworks/$i \
  $EXEC_DIR/$EXE

  # echo "app: $EXEC_DIR/$EXE"
done

# copy everything to a subfolder
rm -Rf ${EXE}-img
mkdir ${EXE}-img
cp -R $EXE.app ${EXE}-img/

# we do not need QT, so skip it
if false 
then
  # check what we need from qt
  declare -a NEEDED_LIBS=( "QtCore" "QtGui" "QtXml" "QtSql" "QtNetwork" "QtOpenGL" "QtSvg")
  # additional files you'd like to get copied to the final dmg inside DMG not bundle! 
  declare -a ADD_FILES=() #"bin/ticket.pdf" "bin/copying.txt") 

  if [ -z $QTDIR ]; then 
	echo "\$QTDIR environment variable not found... exiting." 
	exit 1 
  fi 


  # canonicalize QtDir, unfortunately this bash has no realpath() implementation 
  # so we need to use perl for this 
  QTDIR=`perl -e "use Cwd 'realpath'; print realpath('$QTDIR');"` 

  if [ ! -d "$bundle_dir" ]; then 
	echo "Application bundle not found in bin... exiting." 
	exit 1 
  fi 

  echo "Creating Frameworks directory in application bundle..." 
  mkdir -p "$framework_dir" 
  mkdir -p $plugin_dir 
  mkdir -p $locale_dir 

  libcount=${#NEEDED_LIBS[@]} 
  for (( i = 0 ; i < libcount ; i++ )) 
  do 
	lib=${NEEDED_LIBS[$i]} 
	echo "Processing $lib..." 

	if [ ! -d "$QTDIR/lib/$lib.framework" ]; then 
	  echo "Couldn't find $lib.framework in $QTDIR." 
	  exit 1 
	fi 

	rm -rf "$framework_dir/$lib.framework" 
	cp -fR "$QTDIR/lib/$lib.framework" "$framework_dir" 
	echo "...$lib copied." 

	install_name_tool \ 
	-id "@executable_path/../Frameworks/$lib.framework/Versions/4/$lib" \ 
	"$framework_dir/$lib.framework/Versions/4/$lib" 

	# other Qt libs depend at least on QtCore 
	if [ "$lib" != "QtCore" ]; then 
	  install_name_tool -change "$QTDIR/lib/QtCore.framework/Versions/4/QtCore" \ 
	  "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" \ 
	  "$framework_dir/$lib.framework/Versions/Current/$lib" 
	fi 

	install_name_tool -change "$QTDIR/lib/$lib.framework/Versions/4/$lib" \ 
	"@executable_path/../Frameworks/$lib.framework/Versions/4/$lib" \ 
	"$bundle_bin" 

	echo "...$lib done." 
  done 

  echo "Removing any debug libraries and headers..." 
  find "$framework_dir" | egrep "debug|Headers" | xargs rm -rf 

  echo "Preparing image directory..." 
  tempdir="/tmp/`basename $0`.$$" 
  mkdir $tempdir 
  cp -R $bundle_dir $tempdir 
  echo "...Bundle copied" 
  fcount=${#ADD_FILES[@]} 
  for (( i = 0 ; i < fcount ; i++ )) do 
	file=${ADD_FILES[$i]}    
	if [ ! -f "$file" ]; then 
	  echo "WARNING: $file not found!" 
	else 
	  cp "$file" $tempdir 
	  echo "...$file copied" 
	fi 
  done 

fi


# add a readme
#cp $SOURCE/README ${EXE}-img/ReadMe.txt
path=`pwd`
cd $SOURCE/icons/
mkdir -p $path/${RESOURCE_DIR}/icons
for i in *.png ; do cp $i $path/${RESOURCE_DIR}/icons/; done
cd $path
cp $SOURCE/icons/main.icns $RESOURCE_DIR/

mkdir -p ${RESOURCE_DIR}/GLSL
cp -R $SOURCE/GLSL/*.vs $RESOURCE_DIR/GLSL/
cp -R $SOURCE/GLSL/*.fs $RESOURCE_DIR/GLSL/

# create a disk image from this directory
rm main.dmg
hdiutil create -srcfolder ${EXE}-img -format UDBZ -volname ${EXE} ${EXE}.dmg

