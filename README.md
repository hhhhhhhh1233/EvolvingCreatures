# Evolving Creatures Overview
This is a program that can generate 3D creatures using genetic algorithms. 
They can only be trained for horizontal movement at the moment.
Inspired by Karl Sims' [Evolved Virtual Creatures](https://www.youtube.com/watch?v=RZtZia4ZkX8)

# Examples 

Snake Like Creature | Waggling Creature
:-------------------------:|:-------------------------:
![Creature6-Short](https://github.com/user-attachments/assets/70ea9abf-12ed-40a3-a0fe-005a4554861d) | ![Creature2](https://github.com/user-attachments/assets/1edca0c8-d462-4b2f-95c6-05c94714df91)

These creatures were only run for a few generations, but they display some interesting locomotive capabilites.


# Running the Program

[Grab the latest release from here](https://github.com/hhhhhhhh1233/EvolvingCreatures/releases) and unzip it. Then just run the EvolvingCreatures.exe.

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

Make sure you're positioned in the vcpkg directory then run

### Windows

```console
./vcpkg.exe install physx
```

### Linux

```console
./vcpkg install physx
```

## Cloning project

Then you can clone the project and run 

```console
git clone https://github.com/hhhhhhhh1233/EvolvingCreatures.git
cd EvolvingCreatures
mkdir build && cd build
cmake ..
```
If there are any issues running cmake then make sure that both vcpkg and physx are installed in the proper location, or edit the cmake file to point to where they are on your system.


