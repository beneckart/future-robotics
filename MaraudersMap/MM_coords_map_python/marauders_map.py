from numpy import sin, cos, sqrt
from math import atan2

## production - burning man
#/*
MAN_LAT = 40786400
MAN_LON = -119206500
PLAYA_ELEV = 1190.  # m
SCALE = 1.
#*/

##/ PLAYA COORDINATES CODE ##/

DEG_PER_RAD = (180. / 3.1415926535)
CLOCK_MINUTES = (12 * 60)
METERS_PER_DEGREE = (40030230. / 360.)
# Direction of north in clock units
NORTH = 10.5  # hours
NUM_RINGS = 13  # Esplanade through L
ESPLANADE_RADIUS = (2500 * .3048)  # m
FIRST_BLOCK_DEPTH = (440 * .3048)  # m
BLOCK_DEPTH = (240 * .3048)  # m
# How far in from Esplanade to show distance relative to Esplanade rather than the man
ESPLANADE_INNER_BUFFER = (250 * .3048)  # m
# Radial size on either side of 12 w/ no city streets
RADIAL_GAP = 2.  # hours
# How far radially from edge of city to show distance relative to city streets
RADIAL_BUFFER = .25  # hours

# 0=man, 1=espl, 2=A, 3=B, ...
def ringRadius(n):
    if (n == 0):
        return 0
    elif (n == 1):
        return ESPLANADE_RADIUS
    elif (n == 2):
        return ESPLANADE_RADIUS + FIRST_BLOCK_DEPTH
    else:
        return ESPLANADE_RADIUS + FIRST_BLOCK_DEPTH + (n - 2) * BLOCK_DEPTH
  


# Distance inward from ring 'n' to show distance relative to n vs. n-1
def ringInnerBuffer(n):
    if (n == 0):
        return 0
    elif (n == 1):
        return ESPLANADE_INNER_BUFFER
    elif (n == 2):
        return .5 * FIRST_BLOCK_DEPTH
    else:
        return .5 * BLOCK_DEPTH
  


def getReferenceRing(dist):
    for n in range(NUM_RINGS, 0, -1): # (int n = NUM_RINGS n > 0 n--):
        #print str(n) + ":" + str(ringRadius(n)) + " " + str(ringInnerBuffer(n))
        if (ringRadius(n) - ringInnerBuffer(n) <= dist):
            return n
    return 0


def getRefDisp(n):
    if (n == 0):
        return ")("
    elif (n == 1):
        return "Espl"
    else:
        return str(chr(ord('A') + n - 2))
  


def playaStr(lat, lon, accurate):
    # Safe conversion to float w/o precision loss.
    dlat = 1e-6 * (lat - MAN_LAT)
    dlon = 1e-6 * (lon - MAN_LON)

    m_dx = dlon * METERS_PER_DEGREE * cos(1e-6 * MAN_LAT / DEG_PER_RAD)
    m_dy = dlat * METERS_PER_DEGREE

    dist = SCALE * sqrt(m_dx * m_dx + m_dy * m_dy)
    bearing = DEG_PER_RAD * atan2(m_dx, m_dy)

    clock_hours = (bearing / 360. * 12. + NORTH)
    clock_minutes = (int)(clock_hours * 60 + .5)
    # Force into the range [0, CLOCK_MINUTES)
    clock_minutes = ((clock_minutes % CLOCK_MINUTES) + CLOCK_MINUTES) % CLOCK_MINUTES

    hour = clock_minutes / 60
    minute = clock_minutes % 60
    clock_disp = str(hour) + ":" + ("0" if minute < 10 else "") + str(minute)

    if (6 - abs(clock_minutes/60. - 6) < RADIAL_GAP - RADIAL_BUFFER):
        refRing = 0
    else:
        refRing = getReferenceRing(dist)

    refDelta = dist - ringRadius(refRing)
    refDeltaRounded = (refDelta + .5)

    return clock_disp + " & " + getRefDisp(refRing) + ("+" if refDeltaRounded >= 0 else "-") + \
         str(-refDeltaRounded if refDeltaRounded < 0 else refDeltaRounded) + "m" + \
         ("" if accurate else "-ish")
         
def playa2latlon(hour=3, minute=16, ring='J', espDiv=1.0):
    
    if ring == '1':
        n = 1
    elif ring == '0':
        n = 0
    else:
        n = ord(ring) - ord('A') + 2
    distToMan = ringRadius(n)*espDiv
    
    clock_hours = hour + minute/60.
    
    bearing = (((clock_hours - NORTH)/12.)*360.)
    
    m_dy = distToMan * cos(bearing/DEG_PER_RAD) 
    m_dx = distToMan * sin(bearing/DEG_PER_RAD)
    
    dlon = m_dx/(METERS_PER_DEGREE*cos(1e-6 * MAN_LAT / DEG_PER_RAD))
    dlat = m_dy/METERS_PER_DEGREE
    
    lat = (dlat/1e-6 + MAN_LAT)*1e-6
    lon = (dlon/1e-6 + MAN_LON)*1e-6
    return lat, lon

def makeGoogleMap(latlon):
    url = 'https://www.google.com/maps/dir/'
    for lat, lon in latlon:
        url += '%3.6f, %3.6f/' % (lat, lon)
    url += '@40.786400,-119.206500,15z/'
    return url

def getHrMin(clock=10.25):
    hr = int(clock)
    minute = int((clock - int(clock))*60)
    return hr, minute

def createLEDtable():
    strand = [[] for i in range(8)] # trash fence handled separately
    
    espDiv = 1.0
    
    #tmpStrand = []
    
    # strand #0
    for i in range(33):
        ring = 'K' if i % 2 == 1 else 'L'
        clock = 10 - i*0.25
        hr, minute = getHrMin(clock)
        strand[0].append((hr, minute, ring, espDiv))
        #tmpStrand.append((hr, minute, ring, espDiv))
        
    for i in range(33, 50):
        ring = 'J' if i % 2 == 1 else 'I'
        clock = 2 + (i-33)*0.25
        hr, minute = getHrMin(clock)
        strand[0].append((hr, minute, ring, espDiv))
        #tmpStrand.append((hr, minute, ring, espDiv))
        
    #strand[0].extend(tmpStrand[::-1])    
        
    # strand #1
    for i in range(0, 16):
        ring = 'J' if i % 2 == 1 else 'I'
        clock = 6.25 + i*0.25
        hr, minute = getHrMin(clock)
        strand[1].append((hr, minute, ring, espDiv))
        
    for i in range(16, 49):
        ring = 'G' if i % 2 == 1 else 'H'
        clock = 10 - (i-16)*0.25
        hr, minute = getHrMin(clock)
        strand[1].append((hr, minute, ring, espDiv))        
    
    strand[1].append((2, 0, 'F', espDiv))
    
    #strand #2
    for i in range(32):
        ring = 'F' if i % 2 == 1 else 'E'
        clock = 2.25 + i*0.25
        hr, minute = getHrMin(clock)
        strand[2].append((hr, minute, ring, espDiv))    
        
    for i in range(32, 50):
        ring = 'C' if i % 2 == 1 else 'D'
        clock = 10 - (i-32)*0.25
        hr, minute = getHrMin(clock)
        strand[2].append((hr, minute, ring, espDiv))   
        
    #strand #3
    for i in range(15):
        ring = 'C' if i % 2 == 1 else 'D'
        clock = 5.5 - i*0.25
        hr, minute = getHrMin(clock)
        strand[3].append((hr, minute, ring, espDiv))    
        
    for i in range(15, 48):
        ring = 'B' if i % 2 == 1 else 'A'
        clock = 2 + (i-15)*0.25
        hr, minute = getHrMin(clock)
        strand[3].append((hr, minute, ring, espDiv))  
        
    #strand #4
    for i in range(24):
        ring = '1'
        clock = 10 - i*0.5
        hr, minute = getHrMin(clock)
        strand[4].append((hr, minute, ring, espDiv))   
        
    for i in range(24, 48):
        ring = '1'
        espDiv = 0.8
        clock = 10.5 - (i-24)*0.5
        hr, minute = getHrMin(clock)
        strand[4].append((hr, minute, ring, espDiv))    
        
    strand[4].append((10, 30, '1', 0.6))

    #strand #5
    for i in range(11):
        ring = '1'
        espDiv = 0.6
        clock = 9.5 - i
        hr, minute = getHrMin(clock)
        strand[5].append((hr, minute, ring, espDiv))  
        
    for i in range(11, 17):
        ring = '1'
        espDiv = 0.4
        clock = 11 - (i-11)*2.0
        hr, minute = getHrMin(clock)
        strand[5].append((hr, minute, ring, espDiv)) 
        
    for i in range(17, 21):
        ring = '1'
        espDiv = 0.2
        clock = 1.5 + (i-17)*3
        hr, minute = getHrMin(clock)
        strand[5].append((hr, minute, ring, espDiv)) 
        
    strand[5].append((0, 0, '0', 0.0)) # the man! )( 22nd LED
    
    # road east wing
    strand[5].append((3,0,'1',1-0.9)) #23
    strand[5].append((3,0,'1',1-0.5)) #24
    strand[5].append((3,0,'1',1-0.3)) #25
    strand[5].append((3,0,'1',1-0.1)) #26
    strand[5].append((3,0,'1',1-0.4)) #27
    strand[5].append((3,0,'1',1-0.7)) #28
    
    # road south wing
    strand[5].append((6,0,'1',1-0.9)) #29
    strand[5].append((6,0,'1',1-0.5)) 
    strand[5].append((6,0,'1',1-0.3)) 
    strand[5].append((6,0,'1',1-0.1)) 
    strand[5].append((6,0,'1',1-0.4)) 
    strand[5].append((6,0,'1',1-0.6)) 
    strand[5].append((6,0,'1',1-0.7)) #35
    
    # road west wing
    strand[5].append((9,0,'1',1-0.9)) #36
    strand[5].append((9,0,'1',1-0.5)) 
    strand[5].append((9,0,'1',1-0.3)) 
    strand[5].append((9,0,'1',1-0.1)) 
    strand[5].append((9,0,'1',1-0.4)) 
    strand[5].append((9,0,'1',1-0.7)) #41
    
    # road north wing
    strand[5].append((0,0,'1',1-0.9)) #42
    strand[5].append((0,0,'1',1-0.5)) #43
    strand[5].append((0,0,'1',1-0.3)) 
    strand[5].append((0,0,'1',1-0.1)) 
    strand[5].append((0,0,'1',1-0.4)) 
    strand[5].append((0,0,'1',1-0.6)) #47
    strand[5].append((0,0,'1',1-0.7)) #48
    
    # DEEP PLAYA!!
    # put on roads A, C, E, H, J, L, N, Q
    for ring in ['L', 'J', 'H', 'E', 'C', 'A']:
        strand[6].append((1, 30, ring, 1.0))
 
    for ring in ['A', 'C', 'E', 'H', 'J', 'L']:
        strand[6].append((1, 0, ring, 1.0))
        
    strand[6].append((1, 0, 'Z', 1.0)) # BURN
    
    for ring in ['L', 'J', 'H', 'E', 'C', 'A']:
        strand[6].append((12, 30, ring, 1.0))   
        
    for ring in ['A', 'C', 'E', 'H', 'J', 'L']:
        strand[6].append((12, 00, ring, 1.0))  
        
    strand[6].append((12, 15, 'N', 1.0))
    strand[6].append((12, 00, 'Q', 1.0)) # END OF THE WORLD
    strand[6].append((11, 45, 'N', 1.0))
    
    
    
    for ring in ['L', 'J', 'H', 'E', 'C', 'A']:
        strand[6].append((11, 30, ring, 1.0))  
        
    for ring in ['A', 'C', 'E', 'H', 'J', 'L']:
        strand[6].append((11, 00, ring, 1.0))   
        
    strand[6].append((11, 00, 'Z', 1.0)) # BURN
        
    for ring in ['L', 'J', 'H', 'E', 'C', 'A']:
        strand[6].append((10, 30, ring, 1.0))  

    #for i in range(8):
    #    printStrand(strand[i])
        
    # TRASH FENCE
    # top of trash fence is 8100 feet from man
    # esplanade is 2500 feet
    # distance to trash fence is therefore 8100/2500 = 3.24 esplanades
    
    def radians(tmp):
        return np.pi*tmp/180.0
    
    trashFence = []
    import numpy as np
 
    for theta in np.arange(0, 360, 7.2):
        a = 72.0 #360.0/n
        b = 36.0 #360.0/(2*n)
        gamma = abs((theta%a) - b)
        P = np.cos(radians(b))/np.cos(radians(b)-radians(gamma))
        
        clock = (theta/360.0*12.0) - 6 # shift coords of theta=0
        hr, minute = getHrMin(clock)
        strand[7].append((hr, minute, '1', P*3.24))
   
    for i in range(8):
        strand[i] = strand[i][::-1]

    #for i in range(8):
    #    printStrand(strand[i])
        
    lat_all = []
    lon_all = []
    for s in strand:
        for hr, minute, ring, espDiv in s:
            lat, lon = playa2latlon(hour=hr, minute=minute, ring=ring, espDiv=espDiv)
            lat_all.append(lat)
            lon_all.append(lon)
    
    minLat, maxLat = np.amin(lat_all), np.amax(lat_all)
    minLon, maxLon = np.amin(lon_all), np.amax(lon_all)
    
    width_lat = maxLat - minLat
    width_lon = maxLon - minLon
    
    res = 100
    
    def rotate(x, y, theta):
        xr = x*np.cos(theta) - y*np.sin(theta)
        yr = x*np.sin(theta) + y*np.cos(theta)
        return xr, yr
    
    def to_pixel(lat, lon, theta=0):
        x = (lat - minLat) / (maxLat - minLat) * res
        y = (lon - minLon) / (maxLon - minLon) * res
        
        return rotate(y, x, theta) # NOTE THAT THIS IS SWITCHED
        
    def recenter(xs, ys):
        xmin, xmax = np.amin(xs), np.amax(xs)
        ymin, ymax = np.amin(ys), np.amax(ys)
        xs = np.array(xs) - xmin
        ys = np.array(ys) - ymin
        return xs, ys
    
    xs, ys, ids, stripNums = [], [], [], []
    for i, s in enumerate(strand):
        for j, loc in enumerate(s):
            hr, minute, ring, espDiv = loc
            lat, lon = playa2latlon(hour=hr, minute=minute, ring=ring, espDiv=espDiv)
            x, y = to_pixel(lat, lon, theta=np.radians(-NORTH/12.0*360.0))
            xs.append(x)
            ys.append(y)
            ids.append((64*i + j))
            stripNums.append(i)
            print '%d %f %f' % ((64*i+j, lat, lon))
            
    xs, ys = recenter(xs, ys)
    
    pixel_coords = [[] for i in range(8)]
    for idx, x, y, i in zip(ids, xs, ys, stripNums):
        pixel_coords[i].append((idx, x, y))
    
    for pix in pixel_coords:
        for i, j, k in pix:
            #print 'pixel[%d] = (%f, %f)' % (i, j, k)
            pass #print '%d %f %f' % (i, j, k)
    
    # NOTE THIS IS REVERSED!!
    import pylab as pl
    colors = 'bgrcmykb'
    for i, pix in enumerate(pixel_coords):
        pix = np.array(pix)
        pl.plot(100-pix[:,1], pix[:,2], '%s*-' % colors[i], lw=3, ms=10)
        pl.plot([100-pix[0,1]], np.array([pix[0,2]]), '%so' % colors[i], ms=15)
    pl.axis('equal')
    pl.show()
        
def printStrand(strand):
    for hr, minute, ring, espDiv in strand:
        lat, lon = playa2latlon(hour=hr, minute=minute, ring=ring, espDiv=espDiv)
        print '%3.8f, %3.8f' % (lat, lon)        

def test():    
    #print playaStr(MAN_LAT, MAN_LON, accurate=True)
    print '%3.6f, %3.6f' % ((MAN_LAT-6000)*1e-6, (MAN_LON+6000)*1e-6)
    print playaStr(MAN_LAT-6000, MAN_LON+6000, accurate=True)
    
    latlon = []
    for ring in '01ABCDEFGHIJKL':
        for hour in range(1, 13):
            for minute in [0, 15, 30, 45]:
                lat, lon = playa2latlon(hour=hour, minute=minute, ring=ring)
                print '%3.8f, %3.8f' % (lat, lon)
                latlon.append((lat, lon))
            
    #print makeGoogleMap(latlon)

if __name__ == '__main__':
    createLEDtable()
