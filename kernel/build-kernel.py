#!/usr/bin/env python3

import os
from statistics import median
import time
import sys

from datetime import datetime
from shutil import copytree
from shutil import rmtree

import paramiko
from getpass import getpass

import argparse

# Parameters to configure
TEMPORARY_DIRECTORY     = '~/tmp/rpi-kernel'
BUILD_DIRECTORY         = '/rt-kernel'

GET_SOURCES_CMDS        = ['git clone https://github.com/raspberrypi/linux.git',
                           'git clone https://github.com/raspberrypi/tools.git --depth 3',
                           'wget http://xenomai.org/downloads/xenomai/stable/xenomai-3.0.7.tar.bz2',
                           'tar jxf xenomai-3.0.7.tar.bz2',
                           'cp %s/ipipe-core-4.14.37-rpi.patch .',
                           'cp %s/config-4.14.37-xenomai .']

CONFIGURE_CMDS          = ['cd ./linux',
                           'git reset --hard 29653ef5475124316b9284adb6cbfc97e9cae48f',
                           'cd ..',
                           'xenomai-3.0.7/scripts/prepare-kernel.sh --linux=linux/ --arch=arm --ipipe=ipipe-core-4.14.37-rpi.patch --verbose',
                           'cp config-4.14.37-xenomai ./linux/.config']

BUILD_CMDS              = ['cd ./linux',
                           'make %s zImage',
                           'make %s modules',
                           'make %s dtbs', 
                           'make %s modules_install', 
                           'make %s dtbs_install',
                           'mkdir $INSTALL_MOD_PATH/boot',
                           'cp ./arch/arm/boot/zImage $INSTALL_MOD_PATH/boot/$KERNEL.img',
                           'cd ..']

XENOMAI_BUILD_CMDS      = ['cd xenomai-3.0.7',
                           './scripts/bootstrap --with-core=cobalt –disable-debug',
                           'mkdir ./build',
                           'cd ./build',
                           '../configure CFLAGS="-march=armv7-a -mfpu=vfp3" LDFLAGS="-mtune=cortex-a53" --build=i686-pc-linux-gnu --host=arm-linux-gnueabihf --with-core=cobalt --enable-smp CC=${CROSS_COMPILE}gcc LD=${CROSS_COMPILE}ld',
                           'make -j4 install'
                           ]
PATH_TO_IMAGE           = '/media/dimercur'
 
# 
# Nothing to configure or modify under this line
#

def getSources(tmp_dir, script_dir, sourcesCmd):
    for cmd in sourcesCmd:
        if "%s" in cmd:
            os.system (cmd % script_dir)
        else:
            os.system (cmd)

def configureKernel(tmp_dir, build_dir, configureCmd):
    for cmd in configureCmd:
        if 'cd ' in cmd:
            os.chdir(cmd.split()[1])
        else:
            os.system (cmd)

def buildKernel(tmp_dir, build_dir, buildCmd, nb_threads=4):
    # Set environment variables
    os.environ["ARCH"] = "arm"
    os.environ["CROSS_COMPILE"] = tmp_dir+"/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-"
    os.environ["INSTALL_MOD_PATH"] = build_dir
    os.environ["INSTALL_DTBS_PATH"] = build_dir
    os.environ["KERNEL"] = "kernel7"
    
    for cmd in buildCmd:
        if 'cd ' in cmd:
            os.chdir(cmd.split()[1])
        else:
            if '%s' in cmd:
                threadOption = '-j'+str(nb_threads)
                print (cmd%threadOption)
                os.system(cmd%threadOption)
            else:
                os.system (cmd)
                
    os.chdir(build_dir)
    os.system('tar czf ../xenomai-kernel.tgz *')
    os.chdir(tmp_dir)
    
def installKernel(tmp_dir, build_dir, image_path):
    os.chdir(tmp_dir)
    print (".... Install *.dtb")
    os.system('sudo cp %s/*.dtb %s/boot/'%(build_dir,image_path))
    
    print (".... Install kernel")
    os.system('sudo cp -rd %s/boot/* %s/boot/'%(build_dir,image_path))
    
    print (".... Install libraries (modules)")
    os.system('sudo cp -rd %s/lib/* %s/rootfs/lib/'%(build_dir,image_path))
    
    print (".... Install overlays")
    os.system('sudo cp -d %s/overlays/* %s/boot/overlays/'%(build_dir,image_path))
    
    print (".... Install bcm* files")
    os.system('sudo cp -d %s/bcm** %s/boot/'%(build_dir,image_path))
    
    print (".... Update config.txt file")
    file = open("%s/boot/config.txt"%image_path, "a")  # append mode
    file.write("kernel=${zImage name}\ndevice_tree=bcm2710-rpi-3-b.dtb\n")
    file.close()
    
    print (".... Update cmdline.txt file")
    file = open("%s/boot/cmdline.txt"%image_path, "a")  # append mode
    file.write(" dwc_otg.fiq_enable=0 dwc_otg.fiq_fsm_enable=0 dwc_otg.nak_holdoff=0")
    file.close()

def buildXenomai(tmp_dir, build_dir, buildCmd):
    os.chdir(tmp_dir+"/xenomai-3.0.7")
    
    #for cmd in buildCmd:
    #    if 'cd ' in cmd:
    #        os.chdir(cmd.split()[1])
    #    else:
    #        os.system (cmd)
     # Set environment variables
    os.environ["ARCH"] = "arm"
    os.environ["CROSS_COMPILE"] = tmp_dir+"/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-"
    os.environ["INSTALL_MOD_PATH"] = build_dir
    os.environ["INSTALL_DTBS_PATH"] = build_dir
    os.environ["KERNEL"] = "kernel7"
           
    print (".... Bootstrap")
    os.system("./scripts/bootstrap --with-core=cobalt –disable-debug")
    
    print (".... Configure")
    os.system ("mkdir ./build")
    os.chdir("./build")
    os.system('../configure CFLAGS="-march=armv7-a -mfpu=vfp3" LDFLAGS="-mtune=cortex-a53" --build=i686-pc-linux-gnu --host=arm-linux-gnueabihf --with-core=cobalt --enable-smp CC=${CROSS_COMPILE}gcc LD=${CROSS_COMPILE}ld')
    
    print (".... Building")
    os.system('make -j4 install DESTDIR=${PWD}/target')

def installXenomai(tmp_dir, build_dir, image_path ):
    os.chdir(tmp_dir+"/xenomai-3.0.7/build/target")
    
    os.system('sudo cp -a dev/* %s/rootfs/dev/'%image_path)
    os.system('sudo cp -a usr/* %s/rootfs/usr/'%image_path)

def main(build_only=False):
    # prepare build environment
    temp_dir = os.path.expanduser (TEMPORARY_DIRECTORY)
    build_dir = temp_dir + BUILD_DIRECTORY
    script_dir=""
    
    for arg in sys.argv:
        if os.path.basename(__file__) in arg:
            script_dir = os.path.dirname(arg)
            break
    
    if script_dir == "":
        print ("Can't find directory of script (%s). Exit"%os.path.basename(__file__))
        return 2
    
    print ("Temp directory : " + temp_dir)
    print ("Build directory : " + build_dir)
    print ("Script directory : " + script_dir)
    print ("Build only: " + str(build_only))
    print()
    
    if not os.path.exists(temp_dir):
        print ('Create temp directory "%s"'%temp_dir)
        os.makedirs(build_dir)
    else:
        val=input('Directory "%s" already exists. Ok to remove all contents ? [Y/n/c] '%temp_dir)
        if val=='n' or val=='N':
            return 1
        elif val=='c' or val=='C':
            pass
        else:
            print ('Purge temp directory "%s"'%temp_dir)
            rmtree(temp_dir, ignore_errors=True)
            os.makedirs(build_dir)

    currentdir = os.getcwd()
    os.chdir(temp_dir)
    
    print ("Retrieve sources")
    getSources(temp_dir,script_dir,GET_SOURCES_CMDS)
    
    print ("\nConfigure kernel")
    configureKernel(temp_dir, build_dir, CONFIGURE_CMDS)
    
    print ("\nBuild kernel")
    buildKernel(temp_dir,build_dir,BUILD_CMDS,nb_threads=4)
    
    print ('\nBuild Xenomai libraries')
    buildXenomai(temp_dir, build_dir, [])
    
    if not build_only:
        print ('\nInstalling kernel')
        installKernel(temp_dir, build_dir, PATH_TO_IMAGE)
    
        print ('\nInstall Xenomai libraries')
        installXenomai(temp_dir, build_dir, PATH_TO_IMAGE)
    
    os.chdir(currentdir)
    
def parseCommandLine() -> str:
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-only', action='store_true', help='Build only')
    
    args = parser.parse_args()
    return args.build_only

if __name__ == '__main__':
    build_only = parseCommandLine()
    if build_only == None or build_only == False:
        val=main(False)
    else:
        val=main(True)
        
    if val ==None:
        val =0
        
    print ("\nBye bye")
    exit(val)