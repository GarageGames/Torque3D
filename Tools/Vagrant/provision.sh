# Make sure package listings are up to date.
apt-get update -y

# Install software for development.
sudo apt-get install \
	git \
	build-essential \
	nasm \
	cmake \
	cmake-qt-gui \
	xorg-dev \
	ninja-build \
	gcc-multilib \
	g++-multilib \
	-y

# Install libraries.
sudo apt-get install \
	libogg-dev \
	libxft-dev \
	libx11-dev \
	libxxf86vm-dev \
	libopenal-dev \
	libfreetype6-dev \
	libxcursor-dev \
	libxinerama-dev \
	libxi-dev \
	libxrandr-dev \
	libxss-dev \
	libglu1-mesa-dev \
	-y
