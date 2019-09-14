# Introduction
This project was born to accomplish a school project for the Real-Time Systems class
> FFT. Develop a program that acquires an audio signal (either from file or from microphone) and display both the amplitude and the frequency spectrum (computed by FFT) as a function of time.The spectrum should be optionally visualized as a waveform or a 2D (time-freq.) color map. The program must provide an graphical interface for a band equalizer.

# Requirements
The only need to build the program is **make**

# Dependencies
The project depends on some libraries

## Needed libraries
- fftw3
- liballegro
## Additional 
- criterion
- gnuplot
These are needed only if you are interested to compile tests.
## Documentation
To produce the documentation starting from the code we need:
- Doxygen

# Building
After the dependencies has been installed in your system, just run in the main directory. 
> make 

Make will produce the **player** executable file.

## Test
To compile the test, run:
> make test

## Clean
To clean the directory from object and binary files:
> make clean

# Run
Execute the player with the **sudo** command. It is needed in order to access the real-time feature of your system.
> sudo ./player <input_audio_file>

# Test
As already said above, to compile the test digit:
> make test

This will produce the **player_test** executable.
The **player_test** need to be run with the sudo command as well.

> sudo ./player_test

## Gnuplot
player_test will produce some files under */tmp/github.com/stiflerGit/AudioPlayer* directory. Here we can copy the *test.p* file contained in the test directory and run it with gnuplot to graphically see results of some tests.

> cp test/test.p /tmp/github.com/stiflerGit/AudioPlayer/test/player
> 
> cd /tmp/github.com/stiflerGit/AudioPlayer/test/player
> 
> gnuplot

Now, gnuplot will open its own cli. Here we can type
> load "test.p"
> 
And magically a plot will appear.

# Documentation
To produce the documentation under the *doc* directory:
> doxygen doxygen.conf

