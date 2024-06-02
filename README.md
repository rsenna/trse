# TRSE
![TRSE Logo](resources/images/trse_optic.png)

## What is Turbo Rascal Syntax error, “;” expected but “BEGIN”?

In a nutshell, Turbo Rascal Syntax error, “;” expected but “BEGIN” is a complete suite for developing games and demos for older computer systems. TRSE is created with Qt (C++), and runs as a stand-alone application that contains various tools for developing and deploying projects for these processors. 

Read more at [Turbo Racal SE Hompage](http://www.turborascal.com).

## Repository
Turbo Rascal Syntax Error full repo 
- C++
- Qt

## Prepare
First, clone this repo to a TRSE directory.

### Linux

#### Debian/Ubuntu
```bash
sudo apt install qt5-default qt5-qmake qtdeclarative5-dev mesa-common-dev
```

#### Fedora 40
```bash
# Using Qt 6
sudo dnf install qt-devel qt6-qtbase-devel qt6-qt3d qt6-qtdeclarative-devel mesa-libGL-devel 
sudo dnf install qt-creator           # C++ ide for QT, optional
sudo dnf install libasan libubsan     # AddressSanitizer
```

#### ARM chromebook/ARM computers (Debian based)
```bash
sudo apt install qtbase5-dev qt5-qmake qtbase5-dev-tools qtdeclarative5-dev
```

On windows/macos, you need to download and install the qt libraries, msvc, xcode etc:

### Windows
- install MSVC 2019
- download the Qt framework from https://www.qt.io/download. Install the latest framework of Qt6 (desktop application).

### MacOS
- install Xcode 
- download the Qt framework from https://www.qt.io/download. Install the latest framework of Qt6 (desktop application).

## Compiling
```bash
qmake TRSE.pro
make -j8 
```

### On Qt Creator project settings
Select "Release", and under the qt project/build make sure you set the build directory to be `Release/bin`

## After first compile:
TRSE uses a couple of directories that needs to be linked with symlinks:
- Copy the directory "themes" in **TRSE/Publish/source/** to the **TRSE/Release** build directory 
- Make a symbolic link called "tutorials from your build directory to point to Publish/tutorials to access tutorial projects from the front page 
- Make a symbolic link called "units from your build directory to point to TRSE/Units to access the TRSE library 
- Make a symbolic link "project_templates" from your build directory to point to Publish/project_templates in order to access the "New Project" templates


# Source code information
A compiler UML diagram can be found here: https://github.com/leuat/TRSE/blob/master/uml/compiler.png

