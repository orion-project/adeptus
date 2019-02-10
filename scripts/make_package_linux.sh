#! /bin/bash
#
# Create redistributable AppImage package.
#

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${SCRIPT_DIR}/..

# Create out dir if none
if [ ! -d out ]; then mkdir out; fi
cd out

# Download linuxdeplyqt if none (https://github.com/probonopd/linuxdeployqt)
# NOTE: It've broken compatibility with newer versions forsing to stick at Ubuntu 14 LTS
# See discussion here: https://github.com/probonopd/linuxdeployqt/issues/340
# But I have nor a machine running Trusty and wish to stick at Qt 5.5 
# (the last supported for Trusty) so have to use a more relaxed 5th version of the tool.
#LINUXDEPLOYQT=linuxdeployqt-continuous-x86_64.AppImage
#LINUXDEPLOYQT_URL=https://github.com/probonopd/linuxdeployqt/releases/download/continuous/${LINUXDEPLOYQT}
#LINUXDEPLOYQT=linuxdeployqt-5-x86_64.AppImage
#LINUXDEPLOYQT_URL=https://github.com/probonopd/linuxdeployqt/releases/download/5/${LINUXDEPLOYQT}

LINUXDEPLOY=linuxdeploy-x86_64.AppImage
LINUXDEPLOY_URL=https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/${LINUXDEPLOY}

if [ ! -f ${LINUXDEPLOY} ]; then
  echo
  echo "Downloading ${LINUXDEPLOY}..."
  wget -c ${LINUXDEPLOY_URL}
  chmod a+x ${LINUXDEPLOY}
  if [ "${?}" != "0" ]; then exit 1; fi
fi

## Create AppDir structure
#echo
#echo "Creating AppDir structure..."
#if [ -d AppDir ]; then rm -rdf AppDir; fi
#mkdir -p AppDir/usr/bin
#mkdir -p AppDir/usr/lib
#mkdir -p AppDir/usr/share/applications
#mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps
#cp ../bin/adeptus AppDir/usr/bin
#cp ../release/adeptus.desktop AppDir/usr/share/applications
#cp ../icon/main_256.png AppDir/usr/share/icons/hicolor/256x256/apps/adeptus.png
#if [ "${?}" != "0" ]; then exit 1; fi

# Run linuxdeplyqt on the AppDir
echo
echo "Creating AppImage..."

# qmake must be in PATH, we can try to extract its path from RPATH of app binary
#LD_PATH=/home/kolyan/Qt/5.10.0/gcc_64/lib/;${LD_PATH}
LD_LIBRARY_PATH=/home/kolyan/Qt/5.10.0/gcc_64/lib/:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH
echo ${LD_LIBRARY_PATH=}
echo ";;;;;"

./${LINUXDEPLOY} \
   --executable=../bin/adeptus \
   --desktop-file=../release/adeptus.desktop \
   --icon-file=../icon/adeptus.png \
   --appdir=AppDir \
   --output=appimage

#./${LINUXDEPLOYQT} AppDir/usr/share/applications/adeptus.desktop -appimage -no-translations -no-plugins \
#  -extra-plugins=sqldrivers/libqsqlite.so,iconengines/libqsvgicon.so,imageformats,platforminputcontexts,platforms,xcbglintegrations
if [ "${?}" != "0" ]; then exit 1; fi

# Rename resulting file to contain version
#if [ -f ../release/version.txt ]; then
#  VERSION="$(cat ../release/version.txt)"
#  cp Adeptus-x86_64.AppImage adeptus-${VERSION}-x86_64.AppImage
#else
#  echo
#  echo "Warning: Unknown release version."
#  echo "Run release/make_version.py script to generate version number."
#fi

