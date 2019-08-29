# Gnuplot script file for plotting data in file "force.dat"
# This file is called test.p

set multiplot layout 2,2
set grid
do for [name in "10000Hz  2000Hz  300Hz  5000Hz"] {
        filename = name . ".dat"
        i = strstrt(name, "Hz")
        freq = substr(name, 0, i+1)
        period = 10*44100/freq
        set xrange [0:period]
        # outputps = name . ".ps"
        # set output outputps
        set title name
        plot filename using 1 title 'source' with lines, \
                filename using 2 title 'isolated' with lines, \
                filename using 3 title 'amplified' with lines;
}
unset xrange
unset multiplot
