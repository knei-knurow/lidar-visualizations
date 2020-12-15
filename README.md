# LIDAR Visualizations

This utility takes advantage of real-time and/or preloaded LIDAR data. It is the first time I'm messing around which such a device, so I've decided that the first step would be creating my own data visualization software which also offers some features related to saving and reading time series of point clouds and some experimental calculations. There is a quite large potential in LIDAR devices and this technology can be found in various applications like self-driven vehicles, robotics, terrain mapping etc. 

This project uses **Slamtec RPLIDAR A3M1** device, about which you can learn below.

## Table of contents
- [Gallery](#gallery)
- [About RPLIDAR](#about-rplidar)
    - [References](#references)
- [Binaries](#binaries)
- [Compilation](#compilation)
    - [Windows](#windows)
- [Usage](#usage)
    - [Options](#options)
    - [Scenarios](#senarios)
    - [GUIs](#guis)
    - [RPLIDAR Modes](#rplidar-modes)
- [Datasets](#datasets)
    - [Point cloud](#point-cloud)
    - [Point cloud series](#point-cloud-series)
- [RPLIDAR with STM32](#rplidar-with-stm32)
- [Thanks](#thanks)

## Gallery
Indoor scanning:

![doc/screenshots/garden.gif](doc/screenshots/room.gif)

Outdoor, observing riding cars:

![doc/screenshots/cars.gif](doc/screenshots/cars.gif)

Outdoor, lots of trees and shrubs around:

![doc/screenshots/garden.gif](doc/screenshots/garden.gif)

## About RPLIDAR

This project has been created with the low-cost **Slamtec RPLIDAR A3M1** and SDK provided by its manufacturers. We haven't tested it, but should be compatible with other related Slamtec models. A3M1 supports two important scanning modes depending on whether scans indoor or outdoor (more information in *RPLIDAR User Manual*).

|Property|A3M1|
|:-|-|
|Distance Range|up to 25 m|
|Sample Rate|up to 16 kHz|
|Scan Rate| 5 Hz - 20 Hz|
|Angular Resolution|up to 0.225°|
|Communication Interface|TTL UART|
|Communication Speed|256000 bps|

The complete lidar specification is available 

- [A3M1 website](https://www.slamtec.com/en/Lidar/A3) 
- [SDK GitHub](https://github.com/Slamtec/rplidar_sdk)
- [RPLIDAR A3 User Manual](https://download.kamami.pl/p573426-LM310_SLAMTEC_rplidarkit_usermanual_A3M1_v1.0_en.pdf)
- [RPLIDAR A3 Introduction and Datasheet](https://download.kamami.pl/p573426-LD310_SLAMTEC_rplidar_datasheet_A3M1_v1.3_en.pdf)

<img src="doc/imgs/a3m1.jpg" width=400><img src="doc/imgs/lidar.jpg" width=400>

## Binaries

At the moment, ready-to-run binaries are only available for Windows. Have a look at the GitHub *Releases* tab.

## Compilation 

### Linux, MacOS

TODO

### Windows

TODO

### Windows (Visual Studio)

The project is developed using Visual Studio on Windows. There are some additional dependencies which should be satisfied (actually, you can compile without them but there will be no visualization nor rendering features*).

1. Collect all requirements:
    - Visual Studio with C++ toolchain.
    - SFML 2.5.1 graphical library - [https://www.sfml-dev.org/download/sfml/2.5.1/](https://www.sfml-dev.org/download/sfml/2.5.1/).
    - RPLIDAR SDK - [https://github.com/Slamtec/rplidar_sdk](https://github.com/Slamtec/rplidar_sdk).
2. Prepare RPLIDAR SDK:
    - Move the directory containing RPLIDAR SDK to the path of this repository. Rename it to `rplidar-sdk`.
    - Install CP2102 driver which allows communicating via USB and UART: `rplidar_sdk/tools/cp2102_driver`.
    - Open VS solution with SDK:  `rplidar_sdk/sdk/workspaces/vc**/sdk_and_demo.sln`.
    - Go to *Solution Explorer,* select `rplidar_driver` and build it in *Debug* and *Release* mode. This should produce two `.lib` files which will be used in our final app. You can find them here: `rplidar_sdk\sdk\output`. These files will be automatically found by the LV project, so don't move them.
    - That's it. SDK is ready and you can close the VS project.
3. Prepare SFML 2.5.1.:
    - nah, compilation required
4. Build:
    - Open the VS solution of *lidar-visualizations* - `lidar/lidar.sln`.
    - Compile in *Debug* or *Release* mode (in *Debug* mode SFML is linked dynamically, so you have to provide necessary DLL's aside your output executable - you will find them in `SFML-2.5.1\bin`; in *Release* mode SFML is linked statically).

**\*** `lidar.sln` uses several project property files (*rplidar.props*, *sfml-debug.props*, *sfml-release.props*) which consist of relative include and library paths to RPLIDAR SDK and SFML, and define macros enabling sections of code that requires the specified dependencies (`USING_RPLIDAR`, `USING_SFML`). For example, if you remove SFML property files from the project, a compiler won't be looking for SFML and finally it build the program without the SFML GUI. You can do the same with RPLIDAR, so you won't be able to receive data from it.

## Usage
If successfully downloaded or compiled LV, you are able to start some scanning and visualizing. The program can be controlled via command line in such a way:

```
lidar [options]
```

### Options

```
-h  --help                     Display help.

-f  --file [filename]          Input cloud filename.
-fs --file-series [filename]   Input cloud series filename.
-p  --port [portname]          Connect to the RPLIDAR port.

-m  --mode [portname]          Connect to the RPLIDAR port.
-s  --scenario [id]            Specify scenario (default: 0).
-g  --gui [id]                 Specity GUI (default: 1).
-o  --rplidar-mode [id]        Specify the RPLIDAR scanning mode (default: 4).
```

**Note**: RPLIDAR options are unavailable, if they haven't been compiled into the project (disabled `USING_RPLIDAR` macro).

### Scenarios

Scenarios are sets of actions which are executed just after grabbing the cloud data, and just before its visualization by the GUI. They allow to add more advanced mechanics in the future.

```
0    Do nothing, just grab a cloud and visualize (default).
1    Save each cloud as a part of cloud series.
2    Save each cloud as a new screenshot (extremely unoptimized).
```

### GUIs
GUIs are responsible for the visual layer of the application and interacting with user.

```
0    Terminal GUI - prints data as a list of points on stdout.
1    SFML GUI - default, the most beautiful one from the gallery.
```

**Note**: SFML GUI is unavailable, if it hasn't been compiled into the project (disabled `USING_SFML` macro).

### RPLIDAR Modes

RPLIDAR supports several scanning modes which differs by its applications (indoor/outdoor), distance range, and sample rate. More details can be found in the documentation of the product. You should consider only the option 3 and 4, because the first three options are here due to compatibility reasons and don't produce spectacular results.

```
0    Standard
1    Express
2    Boost
3    Sensitivity (default)
4    Stability
```

## Datasets
As mentioned earlier, LV allows you to save and load point cloud data from your local disk instead of grabbing it from the RPLIDAR driver. The format of input and output datasets is very straightforward and can be modified with a basic text editor. We can distinguish two variants of data - **point cloud** and **point cloud series**.

### Point cloud
Files contain data of a single point cloud (e.g. a full 360° scan, multiple combined ones). Each line (except comments which must start with `#`) represents a single point which consists of an **angle value [°]** and a **distance value [mm]**. Both may be a floating point number, and have to be separated by any kind of white characters.

**Example**:

```
# A comment
# Angle [°]   Distance [mm]
90.0    42.0
180.0   1000
270.0   1920.11
360.0   2002.0
```

**Preview**: 

```
lidar -f datasets/example.txt
```
![doc/screenshots/example.gif](doc/screenshots/example.jpg)

### Point cloud series
Files contain a list of point clouds. This variant can be used to record a series of grabbed clouds. The rules are the same as in the previous paragraph, but there are some special lines starting with `!` which separate two point clouds. Each line marked with `!` should consists of the **ID number of the following point cloud** and **number of milliseconds elapsed from grabbing the previous one**. Clouds should be sorted by their ID number.

Example:
```
# A comment
# ! ID Number   Elapsed time [ms]
# Angle [°]   Distance [mm]
! 1 0
120  100
240  100
360  100
! 2 500
120  200
240  200
360  200
! 3 500
120  300
240  300
360  300
```

Preview: 
```
lidar -fs datasets/example-series.txt
```
![doc/screenshots/example.gif](doc/screenshots/example-series.gif)

## RPLIDAR with STM32

I also encourage you to follow my friend's project in which he combined lidar technology with a portable STM32 microcontroller, and created a similar visualization software but on a completely different low-level platform.

**GitHub**: [https://github.com/knei-knurow/lidar-stm32](https://github.com/knei-knurow/lidar-stm32)

<img src="doc/imgs/stm32.jpg" width=500>

## Thanks

This project was developed under Electronics and Computer Science club in Knurów (KNEI for short) where lots of amazing projects and ideas come from. Have a look at our website - [https://knei.pl/](https://knei.pl/) - unfortunately, at the moment, only available in Polish. Check out our GitHub too - https://github.com/knei-knurow.

Numerous packages with colourful electronic gadgets like RPLIDAR have been granted to us by our friends from [KAMAMI.pl](http://kamami.pl).
