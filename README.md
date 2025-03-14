# Evolving Creatures Overview
This is a program that can generate 3D creatures that can move about on their own using genetic algorithms. 
They can only be trained for horizontal movement at the moment.
Any interesting creatures that you generate can also be saved to file and loaded in later, they'll be saved to the Creatures directory.

Inspired by Karl Sims' [Evolved Virtual Creatures](https://www.youtube.com/watch?v=RZtZia4ZkX8)

# Table of Contents
 - [Examples](#examples)
 - [Running the Program](#running-the-program)
   - [Controls](#controls)
 - [Setup Project](#setup-project)
   - [vcpkg installation](#vcpkg-installation)
   - [Installing PhysX](#installing-physx)
   - [Cloning project](#cloning-project)

# Examples 

Here are some example creatures I generated that exhibit some ability to move around on their own. 
These creatures were only run for a few generations, but they display some interesting locomotive capabilites.

Snake Like Creature | Waggling Creature
:-------------------------:|:-------------------------:
![Creature6-Short](https://github.com/user-attachments/assets/70ea9abf-12ed-40a3-a0fe-005a4554861d) | ![Creature2](https://github.com/user-attachments/assets/1edca0c8-d462-4b2f-95c6-05c94714df91)

# Running the Program

To run the program all you have to do is [grab the latest release from here](https://github.com/hhhhhhhh1233/EvolvingCreatures/releases) and unzip it, then just run the EvolvingCreatures.exe.

> [!NOTE]
> There aren't any specific dependencies as far I know, but I've only tested on windows computers running NVIDIA GPUs.

## Controls
Hold right click to move the camera around and WASD to move around. Q and E will move the camera up and down. The ImGui window has all of the options for creature management, including loading in saved creatures or starting a new run to create new creatures.

Menu Option | Description
----------- | -----------
Population Size | How many creatures are in each generation
Generation Survivors | How many creatures are kept at the end of the evaluation period into the next generation
Mutation Chance | The odds of each mutation event happening to the creature at the end of the evaluation period
Mutation Severity | How much a value can mutate by in percent, e.g. 0.5 would be ±50%
Number of Generations | How many generations it will run for before it's done
Evaluation Duration | How long each generation will last in seconds, a longer period gives the creatures more of a chance to prove themselves but will take longer

# Setup Project
To set up the project files locally you will need to have PhysX installed, below I showcase my method of getting it to work but if you have your own method just edit the cmake files to point at your PhysX install instead. It should work just fine on Linux too but it hasn't been tested much, since PhysX wouldn't compile properly on my own setup.

## vcpkg installation
First make sure to install vcpkg, if you don't have it first make sure you're in your home directory then run the below based on your system

> [!NOTE]
> The project files expect vcpkg to be installed in the __home directory__, if you want it to be somewhere else edit line 11 and line 22 in [root CMakeLists.txt](./CMakeLists.txt) to point to your vcpkg directory and PhysX.

### Windows

```console
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
```

### Linux

```console
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
```

## Installing PhysX

To install physx make sure you're positioned in the vcpkg directory and run:
### Windows

```console
./vcpkg.exe install physx
```

### Linux

```console
./vcpkg install physx
```

## Cloning project

Finally to setup the project locally follow these steps:

```console
git clone https://github.com/hhhhhhhh1233/EvolvingCreatures.git
cd EvolvingCreatures
mkdir build && cd build
cmake ..
```

> [!NOTE]
> If there are any issues running cmake then make sure that both vcpkg and physx are installed in the proper location, or edit the cmake file to point to where they are on your system.
