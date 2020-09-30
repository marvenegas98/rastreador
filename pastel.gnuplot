#!/usr/bin/gnuplot -persist
reset

dataname = 'tabla.csv'
set datafile separator ','

# Get STATS_sum (sum of column 2) and STATS_records 
stats dataname u 2 noout

# Define angles and percentages
ang(x)=x*360.0/STATS_sum        # get angle (grades)
perc(x)=x*100.0/STATS_sum       # get percentage

# Set Output 
set terminal png
set output "piechart.png"
set size square

# Print the Title of the Chart
set title "Frecuencia de Syscalls"
#set label 1 "\n               Frecuencia de Syscalls\n" at graph 0,1.125 left font 'Arial-Bold,18'

#set terminal wxt
unset key
set key off

set xrange [-2.5:2.5]
set yrange [-4.0:4.0]
set style fill solid 1

unset border
unset tics
unset colorbox

# some parameters 
Ai = 5.0;                     # Initial angle
mid = 0.0;                    # Mid angle
set palette rgb 33,13,10;

plot for [i=1:STATS_records] dataname u (0):(0):(1):(Ai):(Ai=Ai+ang($2)):(i) every ::i::i with circle linecolor palette,\
         dataname u (mid=(Ai+ang($2)), Ai=2*mid-Ai, mid=mid*pi/360.0, -0.5*cos(mid)):(-0.5*sin(mid)):(sprintf('%.2f\%', $2, perc($2))) w labels center font ',16',\
         for [i=1:STATS_records]dataname u (1.45):((i*0.21)-2.0):(sprintf('%s:%i calls',stringcolumn(1),$2)) every ::i::i with labels left font 'Arial-Bold,10',\
         for [i=1:STATS_records] '+' u (1.3):((i*0.21)-2.0):(i) pt 5 ps 4 linecolor palette 
# for [i=0:rowf-rowi] filename u (xpos):(ypos(i)):(sprintf('%s: %05.2f%% calls',stringcolumn(1),percentage($2))) every ::i+rowi::i+rowi w labels left offset 3,0

# ***************************************************   +--------------------         shape code=
#     for [i=1:STATS_records] '+' u (1.3):(i*0.25):(i) pt 8 ps 4 linecolor palette          # empty triangle
#     for [i=1:STATS_records] '+' u (1.3):(i*0.25):(i) pt 9 ps 4 linecolor palette          # solid triangle
#     for [i=1:STATS_records] '+' u (1.3):(i*0.25):(i) pt 7 ps 4 linecolor palette          # solid circle
#     for [i=1:STATS_records] '+' u (1.3):(i*0.25):(i) pt 6 ps 4 linecolor palette          # empty circle


# first line plot semicircles: (x):(y):(radius):(init angle):(final angle):(color)
# second line places percentages: (x):(y):(percentage)
# third line places the color labels
# fourth line places the color symbols

