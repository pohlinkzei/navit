#!/bin/bash
set -e

COVERITY_VERSION="2017.07"
BUILD_PATH="linux"

cmake_opts="-Dgraphics/qt_qpainter:BOOL=FALSE -Dgui/qml:BOOL=FALSE -DSVG2PNG:BOOL=FALSE -DSAMPLE_MAP=n -Dgraphics/gtk_drawing_area:BOOL=TRUE"

[ -d $BUILD_PATH ] || mkdir -p $BUILD_PATH
pushd $BUILD_PATH

# Build everything
    echo "Building..."
cmake ${cmake_opts} ../
make -j $(nproc --all)
make package


if [[ "${CIRCLE_BRANCH}" == "audio_framework" ]]; then
	sudo apt-get install libasound2-dev libasound2

	# Test the mpd audio plugin build
	sudo apt-get install mpc mpd
	mkdir ~/linux_audio_mpd && pushd ~/linux_audio_mpd
	cmake ~/navit/ ${cmake_opts}
	make
	echo "Checking if the libaudio_player-mpd.so was built"
	[ -f navit/audio/player-mpd/.libs/libaudio_player-mpd.so ] || exit -1
	echo "SUCCESS"
	popd


	# Test the spotify audio plugin build
        wget https://developer.spotify.com/download/libspotify/libspotify-12.1.51-Linux-x86_64-release.tar.gz
        tar xfz libspotify-12.1.51-Linux-x86_64-release.tar.gz
        pushd libspotify-12.1.51-Linux-x86_64-release
        sudo make install prefix=/usr/local
        popd

	mkdir ~/linux_audio_spotify && pushd ~/linux_audio_spotify
	cmake ~/navit/ ${cmake_opts}
	make
	echo "Checking if the libaudio_player-spotify.so was built"
	[ -f navit/audio/player-spotify/.libs/libaudio_player-spotify.so ] || exit -1
	echo "SUCCESS"
	popd

if [[ "$CIRCLE_ARTIFACTS" != "" ]]; then
	echo "Copying icons to artifacts..."
	cp -r navit/icons $CIRCLE_ARTIFACTS

fi
