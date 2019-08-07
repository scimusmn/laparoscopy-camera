# laparoscopy-camera
Exhibit kiosk software for demonstrating laparoscopic surgery

## Installation instructions

There is an easy and a hard way to install the software. The easy way is to grab a pre-made disk image (a link to which is forthcoming), which you can simply burn onto an SD card, plug in to your pi, and go.

The hard way is to compile OpenCV yourself and install everything from scratch. Here's how to do that:

#### Step One

Obtain an SD card with the 2019-07-10 release of [Raspbian Buster Lite](https://downloads.raspberrypi.org/raspbian_lite_latest). There are many different ways to burn such an image; [this tutorial](https://xmodulo.com/write-raspberry-pi-image-sd-card.html) goes through many of them.

#### Step Two

Boot and connect your Raspberry Pi to the internet. [This tutorial](https://www.raspberrypi.org/documentation/configuration/wireless/wireless-cli.md) describes how to do this from the command line.

#### Step Three

Download this git repository, either by using the "Clone or download" button in the upper right of this page, or by running 

```bash
sudo apt update && sudo apt upgrade
sudo apt install git
git clone https://github.com/scimusmn/laparoscopy-camera.git
```

#### Step Four

Download and build using the included setup scripts by running

```bash
cd laparoscopy-camera
./install-deps.sh
./download-opencv.sh
./build-opencv.sh
```
This *will* take a while, so be patient!

To actually install the libraries you just built:

```bash
cd
cd opencv/opencv-4.1.0/build
sudo make install
```

#### Step Five

Build the exhibit software itself! Type:

```bash
cd
cd laparoscopy-camera
cmake .
make
```

#### Step Six

You now need to configure the software so that it runs on startup. The first step is to disable the screensaver. To do this, run `startx xscreensaver` and pick from the drop-down menu "Disable screen saver." Go to "File > Quit" to exit the program.

Next, run

```bash
cd
nano .profile
```

to open the `.profile` file in the nano text editor. Add to the bottom of the file the line

```
startx /home/pi/laparoscopy-camera/init.sh
```

> **Important note:** If you installed the repository somewhere 
> other than the path above, you should supply that path instead. Additionally, 
> you should edit `init.sh` to reflect that alternative path as well.

Press `Ctrl-O` to save the file and `Ctrl-X` to quit.

Finally, run `sudo raspi-config` and select 
"Boot Options" > "Desktop/CLI." Use the arrow keys to select 
"Console Autologin" and press enter. Select "yes" to reboot,
and ensure that the program is working properly.

**Done!**
