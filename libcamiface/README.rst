.. _libcamiface:

************************************
libcamiface - camera interface C API
************************************

Overview
========

libcamiface ("camera interface") is a C API that provides a camera and OS
independent image acquisition framework.

There is also a Python wrapper (:mod:`cam_iface`) of the libcamiface
libraries.

.. _libcamiface-supported-cameras:

What cameras are supported?
===========================

Camera support is determined by a libcamiface "backend". Each backend
is a piece of code that creates the libcamiface interface for each
particular camera driver.

As seen below, the supported camera drivers are currently libdc1394,
the Prosilia Gigabit Ethernet SDK, Point Grey Research FlyCapture2,
and the QuickTime SequenceGrabber. For a list of cameras supporting
the libdc1394 software, see
http://damien.douxchamps.net/ieee1394/cameras/. For the Prosilica
Gigabit cameras, see http://www.prosilica.com/. For the Point Grey
cameras, see http://www.ptgrey.com. The QuickTime SequenceGrabber
supports any camera available through QuickTime. This includes the
built-in cameras on Mac laptops and desktops.

.. _libcamiface-supported-rates:

What frame rates, image sizes, bit depths are possible?
=======================================================

This depends primarily on the camera and camera interface. A primary
goal of the Motmot suite is not to limit the capabilities
possible. The fastest frame rates that we routinely use are 500 fps
with a Basler A602f camera using a small region of interest and 200
fps on a Prosilica GE 680 at full frame resolution. The largest image
size we routinely use is 1600x1200 on a Pt. Grey Scorpion camera. The
largest bit depths we routinely use are 12 bits/pixel on a Basler
A622f camera.

Backend status
==============

A number of backends are supported.

.. list-table::
  :header-rows: 1

  * - Backend
    - GNU/Linux i386
    - GNU/Linux x86_64 (amd64)
    - win32 (XP)
    - Mac OS X
  * - libdc1394_
    - |works|
    - |works|
    - |orange| newest library version supports Windows, but untested with libcamiface
    - |mostly works| triggering options disabled
  * - `Prosilica GigE Vision`_
    - |works|
    - |works|
    - |works|
    - |works|
  * - `Point Grey Research FlyCapture2 <http://www.ptgrey.com/products/pgrflycapture/index.asp>`_
    - |orange| untested
    - |orange| untested
    - |works|
    - |NA|
  * - `ImperX`_
    - |NA|
    - |NA|
    - |orange| rudiments present in git 'cruft' branch
    - |NA|
  * - `Basler BCAM 1.8`_
    - |NA|
    - |NA|
    - |orange| rudiments present in git 'cruft' branch, frequent BSOD
    - |NA|
  * - `QuickTime SequenceGrabber`_
    - |NA|
    - |NA|
    - |NA| (supported by driver?)
    - |mostly works| basic functionality works

Key to the above symbols:

* |works| Works well, no known bugs or missing features
* |mostly works| Mostly works, some missing features
* |orange| Not working, although with some effort, could presumably be
   made to work
* |NA| The upstream driver does not support this configuration

.. _libdc1394: http://damien.douxchamps.net/ieee1394/libdc1394/
.. _Prosilica GigE Vision: http://www.prosilica.com
.. _ImperX: http://www.imperx.com/
.. _Basler BCAM 1.8: http://www.baslerweb.com/indizes/beitrag_index_en_21486.html
.. _QuickTime SequenceGrabber: http://developer.apple.com/quicktime/

.. |works| image:: _static/greenlight.png
  :alt: works
  :width: 22
  :height: 22
.. |mostly works| image:: _static/yellowgreenlight.png
  :alt: mostly works
  :width: 22
  :height: 22
.. |orange| image:: _static/redlight.png
  :alt: caution
  :width: 22
  :height: 22
.. |NA| replace:: NA


Download
========

.. Also keep motmot/doc/source/download.rst in sync with download page.

Download official releases from `the download page`__.

__ http://code.astraw.com/libcamiface

Install from binary releases
============================

Mac OS X
--------

The Mac installer is called ``libcamiface-x.y.z-Darwin.dmg``.

This will install the files::

  /usr/include/cam_iface.h
  /usr/bin/liveview-glut-*
  /usr/bin/ ( other demos )
  /usr/lib/libcam_iface_*

To run a demo program, open ``/usr/bin/liveview-glut-mega``.

Windows
-------

The Windows installer is called ``libcamiface-x.y.z-win32.exe``.

This will install the files::

  C:\Program Files\libcamiface x.y.z\bin\simple-prosilica_gige.exe
  C:\Program Files\libcamiface x.y.z\bin\liveview-glut-prosilica_gige.exe
  C:\Program Files\libcamiface x.y.z\include\cam_iface.h
  C:\Program Files\libcamiface x.y.z\lib\cam_iface_prosilica_gige.lib
  C:\Program Files\libcamiface x.y.z\bin\cam_iface_prosilica_gige.dll

To run a demo program, open ``C:\Program Files\libcamiface x.y.z\bin\liveview-glut-prosilica_gige.exe``.

Compile from source
===================

On all platforms, you need to install cmake. cmake is available from
http://www.cmake.org/

You will also need the libraries for any camera software. Cmake should
automatically find these if they are installed in the default
locations.

linux
-----

::

  mkdir build
  cd build
  cmake ..
  make
  make install

To build with debug symbols, include the argument
``-DCMAKE_BUILD_TYPE=Debug`` in your call to cmake. To install in
/usr, include ``-DCMAKE_INSTALL_PREFIX=/usr``. To make verbose
makefiles, include ``-DCMAKE_VERBOSE_MAKEFILE=1``.

To cut a source release::

  VERSION="0.5.9"
  git archive --prefix=libcamiface-$VERSION/ release/$VERSION | gzip -9 > ../libcamiface-$VERSION.tar.gz
  git archive --prefix=libcamiface-$VERSION/ --format=zip release/$VERSION > ../libcamiface-$VERSION.zip

To make a Debian source package::

  VERSION="0.5.9"
  ln ../libcamiface-$VERSION.tar.gz ../libcamiface_$VERSION.orig.tar.gz
  rm -rf ../libcamiface_*.orig.tar.gz.tmp-nest
  git-buildpackage --git-debian-branch=debian --git-upstream-branch=master --git-no-create-orig --git-tarball-dir=.. --git-ignore-new --git-verbose -rfakeroot -S

Mac OS X
--------

Download and install Apple's XCode. This requires signing up (free) as
an Apple ADC member.

::

  mkdir build
  cd build
  cmake ..
  make
  cpack

To build with debug symbols, include the argument
``-DCMAKE_BUILD_TYPE=Debug`` in your call to cmake.

In fact, I use the following commands to set various environment
variables prior to my call to cmake.::

  # You will doubtless need to change these to match your system
  export PROSILICA_CMAKE_DEBUG=1
  export PROSILICA_TEST_LIB_PATHS=/Prosilica\ GigE\ SDK/lib-pc/x86/4.0
  export GLEW_ROOT="/Users/astraw/other-peoples-src/glew/glew-1.5.1"

This will build a Mac installer, called ``libcamiface-x.y.z-Darwin.dmg``.

To build an Xcode project, run cmake with the argument
``-DCMAKE_GENERATOR=Xcode``.

Windows
-------

Windows XP 32bit with CMake 2.6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Install Microsoft's Visual Studio 2008. (Tested with Express Edition.)
Install CMake.

Open a Visual Studio Command Prompt from Start Menu->All
Programs->Microsoft Visual C++ 2008 Express Edition->Visual Studio
Tools->Visual Studio 2008 Command Prompt. Change directories into the
libcamiface source directory.

::

  cmakesetup
  rem  In the cmakesetup GUI, set your source and build directories.
  rem  Click "configure".
  rem  In the "Select Generator" menu that pops up, press "NMake Makefiles".
  rem  After it's done configuring, click "configure" again.
  rem  Finally, click "OK".

  rem Now change into your build directory.
  cd build
  nmake

  rem Now, to build an NSIS .exe Windows installer.
  cpack

This will build a Windows installer, called
``libcamiface-x.y.z-win32.exe``.

Windows 7 64bit with CMake 2.8 to make 32 bit libcamiface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Install Microsoft's Visual Studio 2008. Install CMake.

Open a Visual Studio Command Prompt from Start Menu->All
Programs->Microsoft Visual Studio 2008->Visual Studio
Tools->Visual Studio 2008 Command Prompt. Change directories into the
libcamiface source directory.

::

  cd build
  "C:\Program Files (x86)\CMake 2.8\bin\cmake.exe" .. -G "NMake Makefiles"
  nmake

  rem Now, to build an NSIS .exe Windows installer.
  cpack

This will build a Windows installer, called
``libcamiface-x.y.z-win32.exe``.


Backend notes
=============

prosilica_gige
--------------

Here is an example of setting attributes on the camera using
Prosilica's command line tools::

  export CAM_IP=192.168.1.63
  CamAttr -i $CAM_IP -s StreamBytesPerSecond 123963084
  CamAttr -i $CAM_IP -s PacketSize 1500

Environment variables:

  * *PROSILICA_BACKEND_DEBUG* print various debuggin information.

libdc1394
---------

Environment variables:

 * *DC1394_BACKEND_DEBUG* print libdc1394 error messages. (You may
   also be interested in libdc1394's own *DC1394_DEBUG* environment
   variable, which prints debug messages.)

 * *DC1394_BACKEND_1394B* attempt to force use of firewire
    800. (Otherwise defaults to 400.)

 * *DC1394_BACKEND_AUTO_DEBAYER* use dc1394 to de-Bayer the images,
    resulting in RGB8 images (rather than MONO8 Bayer images).

Git source code repository
==========================

The `development version of libcamiface`__ may be downloaded via git::

  git clone git://github.com/motmot/libcamiface.git

__ http://github.com/motmot/libcamiface

License
=======

libcamiface is licensed under the BSD license. See the LICENSE.txt file
for the full description.
