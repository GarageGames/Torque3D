# Make sure package listings are up to date.
apt-get update -y

# Install Unity desktop for GUI purposes.
apt-get install -y --no-install-recommends ubuntu-desktop

# Install software for development.
apt-get install \
	terminal \
	git \
	-y

# Install Torque-specific binaries.
sudo apt-get install \
	build-essential \
	nasm \
	xorg-dev \
	ninja-build \
	gcc-multilib \
	g++-multilib \
	cmake \
	cmake-qt-gui \
	--ignore-missing -y

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
	--ignore-missing -y
