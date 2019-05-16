png2pcd_batch   -   simple command line utility to convert depth and rgb frames
from png format to PCL pointcloud.



All dataset related parameters are incapsulated in Intr structure ( intrinsics ).
There are: width, height, fx, fy, cx, cy, scale_factor.
Usually depth data is saved as unsigned short ( 16 bit ), 
but in pcl::PointXYZ you have to re-scale it to float - metric measurment.

Appropriate intrinsics should be written to file cam_params.cfg otherwise
default values will be used ( which may lead to invalid output data ).

There are exist two opportunity for compiling:

using classical make:
edit WORK_DIR in Makefile to point in directory contained pcl-trunk & opencv
make
it produce more lightweighted version by avoiding linkage with unnecessary libraries

or using cmake:

mkdir build; cd build
cmake ..
make

NOTE 1: There are only two dependencies:
PCL and OpenCV.

NOTE 2: in case of builded-but-not-properly-installed OpenCV libraries you have to 
manually create symlink to ipp lib:
sudo ln -s /path-to-opencv/3rdparty/ippicv/unpack/ippicv_lnx/lib/intel64/libippicv.a /usr/lib/libippicv.a
