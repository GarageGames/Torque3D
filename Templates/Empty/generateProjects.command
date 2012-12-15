#!/bin/sh

cd "`dirname "$0"`"

OS=`uname`

if [ "$OS" = "Darwin" ]; then
	/usr/bin/php ../../Tools/projectGenerator/projectGenerator.php buildFiles/config/project.mac.conf
else
	/usr/bin/php ../../Tools/projectGenerator/projectGenerator.php buildFiles/config/project.linux.conf
	/usr/bin/php ../../Tools/projectGenerator/projectGenerator.php buildFiles/config/project.linux_ded.conf
fi
