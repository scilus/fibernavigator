#!/bin/bash

#
# This script creates a max osx bundle and a disk image (.dmg) file
# out of a compiled Fibernavigator version.
#
# This script considers that you installed GLEW using MacPorts, and that it was put in "/opt/local/lib".
#
# $1 Path to the application executable.
# $2 Path to the custom wxWidgets compilation, should point to the build directory

echo

if [[ $# < 2 ]]
then
    echo "Missing some arguments."
    return 1
fi

verbose=false
if [[ $# == 3 && $3 == "1" ]]
then
    verbose=true
    echo "Verbose"
fi

#echo $1
#
# change these defines to what you want to build a bundle from
export EXE=FiberNavigator
export BUNDLEDIR=$EXE.app
export EXEC_DIR=$BUNDLEDIR/Contents/MacOS
export FRAMEWORK_DIR=$BUNDLEDIR/Contents/Frameworks
export RESOURCE_DIR=$BUNDLEDIR/Contents/Resources
export SOURCE=../src/

# make required directories
mkdir -p $EXEC_DIR
mkdir -p $FRAMEWORK_DIR

# Copy the executable to the EXEC_DIR
echo "Copying $1 to the app directory..."
cp $1 $EXEC_DIR/$EXE

WXWIDGETSROOT=$2

# copy additional libraries
EXTLIBS=("${WXWIDGETSROOT}/lib/libwx_base_carbon-2.8.0.dylib" "${WXWIDGETSROOT}/lib/libwx_base_carbon_net-2.8.0.dylib" "${WXWIDGETSROOT}/lib/libwx_base_carbon_xml-2.8.0.dylib" 
         "${WXWIDGETSROOT}/lib/libwx_mac_adv-2.8.0.dylib" "${WXWIDGETSROOT}/lib/libwx_mac_core-2.8.0.dylib" "${WXWIDGETSROOT}/lib/libwx_mac_gl-2.8.0.dylib" "/opt/local/lib/libGLEW.1.6.0.dylib")

EXTLIBN=("libwx_base_carbon-2.8.0.dylib" "libwx_base_carbon_net-2.8.0.dylib" "libwx_base_carbon_xml-2.8.0.dylib" "libwx_mac_adv-2.8.0.dylib" "libwx_mac_core-2.8.0.dylib" 
        "libwx_mac_gl-2.8.0.dylib" "libGLEW.1.6.0.dylib")

echo "Copying external libraries to the Frameworks folder..."
    
for i in ${EXTLIBS[@]}
do
    if [[ $verbose == true ]]
    then
        echo "  " `basename $i`
    fi
    
    cp $i $FRAMEWORK_DIR/
done

# relink executable to its external libraries that we
# ship with the bundle
echo "Relinking binary to external libs.."
libcount=${#EXTLIBS[@]} 
for (( j = 0 ; j < libcount ; j++ )) 
do
  mlib=${EXTLIBS[$j]} 
  mbase=${EXTLIBN[$j]}
  
  if [[ $verbose == true ]]
  then
    echo "  processing $mlib..."
  fi

  install_name_tool \
  -id @executable_path/../Frameworks/$mbase \
  $FRAMEWORK_DIR/$mbase  #was $i

  install_name_tool \
  -change $mlib \
  @executable_path/../Frameworks/$mbase \
  $EXEC_DIR/$EXE

  # some path still end up in /usr/lib don't know why,
  # but we circumvent this...
  install_name_tool \
  -change /usr/lib/$mbase \
  @executable_path/../Frameworks/$mbase \
  $EXEC_DIR/$EXE

done

# relink libraries and binary
echo "Relinking all libraries..."
for i in `ls -1 $FRAMEWORK_DIR`
do
  if [[ $verbose == true ]]
  then
    echo "  relinking library: $i"
  fi
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
	
	if [[ $verbose == true ]]
    then
        echo "    processing $mlib..."
    fi
	
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
done


# add a readme
#cp $SOURCE/README ${EXE}-img/ReadMe.txt
echo "Copying resources to the Resources folder..."
path=`pwd`
cd $SOURCE/icons/
mkdir -p $path/${RESOURCE_DIR}/icons
for i in *.png ; do cp $i $path/${RESOURCE_DIR}/icons/; done
cd $path
cp $SOURCE/icons/main.icns $RESOURCE_DIR/

mkdir -p ${RESOURCE_DIR}/GLSL
cp -R $SOURCE/GLSL/*.vs $RESOURCE_DIR/GLSL/
cp -R $SOURCE/GLSL/*.fs $RESOURCE_DIR/GLSL/

# copy everything to a subfolder
rm -Rf ${EXE}-img
mkdir ${EXE}-img
cp -R $EXE.app ${EXE}-img/

cd $SOURCE;
#svn info > $path/$EXE-img/svn-info.txt
cd $path


# create a disk image from this directory
echo "Creating a disk image for the application..."
rm -f ${EXE}.dmg
hdiutil create -srcfolder ${EXE}-img -format UDBZ -volname ${EXE} ${EXE}.dmg

# Remove temporary files.
rm -Rf ${EXE}-img
