#!/bin/bash
set -e

COVERITY_VERSION="2017.07"
BUILD_PATH="linux"

cmake_opts="-Dgraphics/qt_qpainter:BOOL=FALSE -Dgui/qml:BOOL=FALSE -DSVG2PNG:BOOL=FALSE -DSAMPLE_MAP=n -Dgraphics/gtk_drawing_area:BOOL=TRUE"

[ -d $BUILD_PATH ] || mkdir -p $BUILD_PATH
pushd $BUILD_PATH

if [[ "${CIRCLE_PROJECT_USERNAME}" == "navit-gps" && "${CIRCLE_BRANCH}" == "trunk" ]]; then
	# If we are building the official trunk code, push an update to coverity
	wget -nv -c -O /tmp/cov-analysis-linux64-${COVERITY_VERSION}.tar.gz http://sd-55475.dedibox.fr/cov-analysis-linux64-${COVERITY_VERSION}.tar.gz
	tar xfz /tmp/cov-analysis-linux64-${COVERITY_VERSION}.tar.gz --no-same-owner -C /usr/local/share/
	export PATH=/usr/local/share/cov-analysis-linux64-${COVERITY_VERSION}/bin:$PATH
	
	cov-build --dir cov-int cmake ${cmake_opts} ../
	cov-build --dir cov-int make -j $(nproc --all) || exit -1
	tar czvf navit.tgz cov-int
	
	curl --form token=$COVERITY_TOKEN \
  --form email=$COVERITY_EMAIL \
  --form file=@navit.tgz \
  --form version="${CIRCLE_BRANCH}-$CIRCLE_SHA1" \
  --form description="${CIRCLE_BRANCH}-$CIRCLE_SHA1" \
  https://scan.coverity.com/builds?project=$CIRCLE_PROJECT_USERNAME

	# Then update the translation template on launchpad
	sed -i '/INTEGER/d' po/navit.pot
	cp po/navit.pot $CIRCLE_ARTIFACTS/
	curl "https://translations.launchpad.net/navit/${CIRCLE_BRANCH}/+translations-upload" -H "$lp_cookie" -H "Referer: https://translations.launchpad.net/navit/${CIRCLE_BRANCH}/+translations-upload" -F file=@po/navit.pot | grep title
	popd

else
#<<<<<<< HEAD
#	mkdir ~/linux-bin && cd ~/linux-bin
#	cmake ~/${CIRCLE_PROJECT_REPONAME}/ ${cmake_opts} -DSAMPLE_MAP=n -DUSE_I2C=1 || exit -1
#	make  || exit -1
#fi

#if [[ "${CIRCLE_BRANCH}" == "audio_framework" ]]; then
#	sudo apt-get install libasound2-dev libasound2

	# Test the mpd audio plugin build
#	sudo apt-get install mpc mpd
#	mkdir ~/linux_audio_mpd && pushd ~/linux_audio_mpd
#	cmake ~/navit/ ${cmake_opts}
#	make
#	echo "Checking if the libaudio_player-mpd.so was built"
#	[ -f navit/audio/player-mpd/.libs/libaudio_player-mpd.so ] || exit -1
#	echo "SUCCESS"
#	popd


	# Test the spotify audio plugin build
 #       wget https://developer.spotify.com/download/libspotify/libspotify-12.1.51-Linux-x86_64-release.tar.gz
 #       tar xfz libspotify-12.1.51-Linux-x86_64-release.tar.gz
 #       pushd libspotify-12.1.51-Linux-x86_64-release
 #       sudo make install prefix=/usr/local
 #       popd

#	mkdir ~/linux_audio_spotify && pushd ~/linux_audio_spotify
#	cmake ~/navit/ ${cmake_opts}
#	make
#	echo "Checking if the libaudio_player-spotify.so was built"
#	[ -f navit/audio/player-spotify/.libs/libaudio_player-spotify.so ] || exit -1
#	echo "SUCCESS"
#	popd
#=======
	cmake ${cmake_opts} ../ || exit -1
	make -j $(nproc --all) || exit -1
fi

if [[ "$CIRCLE_ARTIFACTS" != "" ]]; then
	cp -r navit/icons $CIRCLE_ARTIFACTS
#>>>>>>> navit/trunk
fi
