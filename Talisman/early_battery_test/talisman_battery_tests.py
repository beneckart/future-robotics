import pylab as pl
import numpy as np

f = open('output_latlon_timing_delanies.txt')

t = []
v = []
i = 0
for line in f.readlines():
    tokens = line.split(',')
    time = float(tokens[0])
    volt = float(tokens[3])
    lat = float(tokens[1])
    lon = float(tokens[2])
    if i > 275 and volt < 6 and volt > 0 and lat < 100 and lat > 0:
        t.append(time/(60.*60.))
        v.append(volt)
        print str(lat) + ", " + str(lon)
    i += 1
    if i > 293:
        break

x1, y1 = 0, 4.2
x2, y2 = 8.0, 3.7

m = (y2-y1)/(x2-x1)
b = 4.2

batt_percentage = (np.array(v)-3.7)/(4.2-3.7)

fig = pl.figure()
ax = pl.gca()    

pl.plot(t[:4685], v[:4685], '*-')
pl.plot(t[:4685], m*np.array(t[:4685])+b)
pl.plot(t[:4685], 3.7*np.ones(len(t[:4685])))
pl.xlabel('Running Time (hours)')
pl.ylabel('Voltage of Talisman')
pl.title('Voltage Drop Over Time\n2500mAh lipoly, 1W LoRa, 1 LED lit, 5 second update rate')
pl.legend(['Battery Voltage', 'Linear Approximation', 'Software Failsafe Cutoff'])
ax.set_ylim([3.0, 4.4])

ax2 = ax.twinx()
pl.plot(t[:4685], batt_percentage[:4685], 'r', alpha=0.0)
ax2.set_ylim([(3.0-3.7)/(4.2-3.7), (4.4-3.7)/(4.2-3.7)])

vals = ax2.get_yticks()
ax2.set_yticklabels(['{:3.1f}%'.format(x*100) for x in vals])

pl.show()

# 4.2 to 3.75 is pretty linear (over the course of 8 hours)
# 

# (-4.2/3.75)*x + 4.2 = 
