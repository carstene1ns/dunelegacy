### Instructions for building and packaging on Ubuntu 25.04:
- Install the prerequisites:
	```bash
	sudo apt install cmake libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev
	```

- Obtain the source code:
	- Clone the repository.
		```bash
		git clone git://dunelegacy.git.sourceforge.net/gitroot/dunelegacy/dunelegacy -b BRANCHNAME
		```
		or
		```bash
		git clone ssh://USERNAME@git.code.sf.net/p/dunelegacy/code dunelegacy-code -b BRANCHNAME
		```
	- Obtain the .PAK files and place them in the `dunelegacy/data` folder.

- Building:
	```bash
	cd dunelegacy
	cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Release . && cmake --build build
	```

- Packaging:
	```bash
	cd build
	cpack -G DEB -D CPACK_DEBIAN_PACKAGE_MAINTAINER="User Name" -D CPACK_PACKAGING_INSTALL_PREFIX=/usr/local
	cpack -G RPM -D CPACK_PACKAGING_INSTALL_PREFIX=/usr/local
	cpack -G TGZ -D CPACK_PACKAGING_INSTALL_PREFIX=/usr/local
	```
