# -*- coding: utf-8 -*-
"""
Created on Sun Nov 12 09:37:47 2017

@author: beckart

RANDOM NOTES FROM BM 2018:

  else if(currTime - last_log >= LOG_UPDATE_DELAY_MS)
  {
    logfile.print(month); logfile.print(" "); // Hour (0-23) (u8)
    logfile.print(day); logfile.print(" "); // Hour (0-23) (u8)
    logfile.print(hour); logfile.print(" "); // Hour (0-23) (u8)
    logfile.print(minute); logfile.print(" ");// Minute (0-59) (u8)
    logfile.print(second); logfile.print(" "); // Second (0-59) (u8)
    logfile.print(DEVICE_ID); logfile.print(" "); 
    logfile.print(lat, 6); logfile.print(" ");
    logfile.print(lon, 6); logfile.print(" ");
    logfile.print(numSat); logfile.print(" ");
    //logfile.print(spd); logfile.print(" ");
    logfile.print(batPerc); logfile.print(" ");
    logfile.println(millis()); //f.print(" ");
    
    last_log = millis();
  }

Shervin (10)
Device ('08D9C53A7D80', 4):    49 good entries
Device ('08D9C53A7D80', 5):   412 good entries
Device ('08D9C53A7D80', 9): 14074 good entries
Device ('08D9C53A7D80', 15):   987 good entries

Dad #2
Device ('54D8C53A7D80', 2): 17643 good entries

Jimmy (was 6)
Device ('60D8C53A7D80', 3):    57 good entries
Device ('60D8C53A7D80', 5): 12236 good entries
Device ('60D8C53A7D80', 9):    44 good entries

JP (verified by ID), but was 8 on the box... might have been Ben on wednesday as ID 11. was Ash during marathon
Device ('7CD8C53A7D80', 11): 17131 good entries 


Device ('7CD9C53A7D80', 0):    16 good entries
Device ('7CD9C53A7D80', 1):  1132 good entries
Device ('7CD9C53A7D80', 3):   757 good entries
Device ('7CD9C53A7D80', 5):    62 good entries

Ben after Thursday (?) -- 11(2), or Elliot?
Device ('94D8C53A7D80', 1):   384 good entries
Device ('94D8C53A7D80', 2):  2304 good entries
Device ('94D8C53A7D80', 11):  9898 good entries

Luca (7)
Device ('98D7C53A7D80', 5):  1990 good entries
Device ('98D7C53A7D80', 6): 14443 good entries

Unknown0 -- Elliot? 9(2)
Device ('E0D8C53A7D80', 0):  2698 good entries
Device ('E0D8C53A7D80', 4): 13242 good entries


Notes from BM:
// 0-5 Dad
// *6 - Jimmy (was JP on Wednesday?) -- gave to him
// *7 - Luca -- gave to him
// *8 - JP (was Ben on Wednesday?) -- left by bed
// *9(2) - Elliot
// 10 - Shervin
// 11 - Ben

// NEED 11, 10 -- and need to charge them! (Elliot/Shervin?

// made an 11(2), 9(2), but 11(2) has bad usb port so ditching it

// gave Talisman 6 to Jimmy (6) -- are the channels right?
// flashed 11(2) for me (Ben)
// wrote JP on 8
// gave 7 to Luca


keymap = {}

#0
#1
keymap['54D8C53A7D80'] = 2 # dad #2
#3
keymap['E0D8C53A7D80'] = 4 # elliot
keymap['60D8C53A7D80'] = 5 # jimmy
keymap['98D7C53A7D80'] = 6 # luca
keymap['7CD9C53A7D80'] = 7 # ??
#8
keymap['08D9C53A7D80'] = 9 # shervin
keymap['94D8C53A7D80'] = 10 # ben
keymap['7CD8C53A7D80'] = 11 # ben/jp

namemap = {}
namemap['08D9C53A7D80'] = 'shervin'
namemap['54D8C53A7D80'] = 'dad #2'
namemap['60D8C53A7D80'] = 'jimmy'
namemap['7CD8C53A7D80'] = 'jp'
namemap['94D8C53A7D80'] = 'ben'
namemap['98D7C53A7D80'] = 'luca'
namemap['E0D8C53A7D80'] = 'elliot'
namemap['7CD9C53A7D80'] = '??'

"""

import sys
from collections import defaultdict
from glob import glob
import pickle

def makegoogleheatmap(ifname, ofname='googleheatmap.html'):
    
    f = open(ifname)
    of = open(ofname,'w')

    print 'writing heatmap to %s' % ofname

    html_top = '''
    <!DOCTYPE html>
    <html>
      <head>
        <meta charset="utf-8">
        <title>Heatmaps</title>
        <style>
          /* Always set the map height explicitly to define the size of the div
           * element that contains the map. */
          #map {
            height: 100%;
          }
          /* Optional: Makes the sample page fill the window. */
          html, body {
            height: 100%;
            margin: 0;
            padding: 0;
          }
          #floating-panel {
            position: absolute;
            top: 10px;
            left: 25%;
            z-index: 5;
            background-color: #fff;
            padding: 5px;
            border: 1px solid #999;
            text-align: center;
            font-family: 'Roboto','sans-serif';
            line-height: 30px;
            padding-left: 10px;
          }
          #floating-panel {
            background-color: #fff;
            border: 1px solid #999;
            left: 25%;
            padding: 5px;
            position: absolute;
            top: 10px;
            z-index: 5;
          }
        </style>
      </head>

      <body>
        <div id="floating-panel">
          <button onclick="toggleHeatmap()">Toggle Heatmap</button>
          <button onclick="changeGradient()">Change gradient</button>
          <button onclick="changeRadius()">Change radius</button>
          <button onclick="changeOpacity()">Change opacity</button>
        </div>
        <div id="map"></div>
        <script>

          // This example requires the Visualization library. Include the libraries=visualization
          // parameter when you first load the API. For example:
          //<script src="https://maps.googleapis.com/maps/api/js?key=AIzaSyB2-p5mEPvn78N7j7iRXLDmRdvEBhZRjzk&libraries=visualization">

          var map, heatmap;

          function initMap() {
            map = new google.maps.Map(document.getElementById('map'), {
              zoom: 15,
              center: {lat: 40.7864, lng: -119.2065}, 
              mapTypeId: 'hybrid' 
            });

            heatmap = new google.maps.visualization.HeatmapLayer({
              data: getPoints(),
              map: map
            });
          }

          function toggleHeatmap() {
            heatmap.setMap(heatmap.getMap() ? null : map);
          }

          function changeGradient() {
            var gradient = ['''
            
    gradient_old = """
              'rgba(0, 255, 255, 0)',
              'rgba(0, 255, 255, 1)',
              'rgba(0, 191, 255, 1)',
              'rgba(0, 127, 255, 1)',
              'rgba(0, 63, 255, 1)',
              'rgba(0, 0, 255, 1)',
              'rgba(0, 0, 223, 1)',
              'rgba(0, 0, 191, 1)',
              'rgba(0, 0, 159, 1)',
              'rgba(0, 0, 127, 1)',
              'rgba(63, 0, 91, 1)',
              'rgba(127, 0, 63, 1)',
              'rgba(191, 0, 31, 1)',
              'rgba(255, 0, 0, 1)'"""
            
    import numpy as np
    grad_lst = ["'rgba(0, 255, 255, 0)'"]
    for i in np.linspace(64,0,4):
        grad_lst.append("'rgba(0, %d, 255, 1)'" % int(i))
    for i in np.linspace(255,127,64):
        grad_lst.append("'rgba(0, 0, %d, 1)'" % int(i))    
    for i, j in zip(np.linspace(0,255,512), np.linspace(127,0,512)):
        grad_lst.append("'rgba(%d, 0, %d, 1)'" % (int(i), int(j)))
    #for i in range(512):
    #    grad_lst.append("'rgba(255, 0, 0, 1)'") 
            
    gradient = ',\n'.join(grad_lst)
              
    html_after_gradient = '''
            ]
            heatmap.set('gradient', heatmap.get('gradient') ? null : gradient);
          }

          function changeRadius() {
            heatmap.set('radius', heatmap.get('radius') ? null : 20);
          }

          function changeOpacity() {
            heatmap.set('opacity', heatmap.get('opacity') ? null : 0.7);
          }

          // Heatmap data: 500 Points
          function getPoints() {
            return [
              '''

    #new google.maps.LatLng(37.782551, -122.445368),
    #new google.maps.LatLng(37.751266, -122.403355)

    html_bottom = '''
            ];
          }
        </script>
        <script async defer
            src="https://maps.googleapis.com/maps/api/js?key=AIzaSyB2-p5mEPvn78N7j7iRXLDmRdvEBhZRjzk&libraries=visualization&callback=initMap">
        </script>
      </body>
    </html>
    '''

    prototype = 'new google.maps.LatLng(%s)'

    build_str = []


    for line in f:
        build_str.append(prototype % line.strip())
        
    html_str = ',\n'.join(build_str)

    html = html_top + gradient + html_after_gradient + html_str + html_bottom



    of.write(html)
    of.close()
    
def clean_logs(outputfilename, dirs=['.']):
    ofName_cleaned = outputfilename + "_cleaned.txt"
    ofName_pickle = outputfilename + ".pickle"
    
    of = open(ofName_cleaned,"w")

    num_good = 0
    num_start_log = 0
    num_end_log = 0

    num_malformed_bad_token = 0
    num_malformed_num_tokens = 0
    num_dupes = 0
    
    bm_ll = [41, -119]
    
    oob_dict = {
    'clarksville':[36.545088, -87.315514],
    'pittsburgh':[40.424187, -79.982342],
    'san fransisco':[37, -122],
    'zero lat lon':[0, 0]}
    num_malformed_oob = defaultdict(int)

    data = defaultdict(list)

    #inputfiles = ['latlon_dev%02d_try00.txt' % i for i in range(8)]
    totalFiles = []
    for diri in dirs: 
        print 'Looking for: %s' % diri + "/*.*"
        totalFiles.extend(glob(diri + "/*.*"))
    totalFiles = sorted(totalFiles)
    for i, fname in enumerate(totalFiles):
        print i, fname
    
    inp = raw_input('pick files to use (ex. 0 1 2 5 7): ')
    inputfiles = [totalFiles[int(i)] for i in inp.split()]


    
    dupe_detect = {}

    for fname in inputfiles:
        f = open(fname,"r")
        firstline = True
        for line in f:
            if firstline: 
                devID_unique = line.split()[-1]
                print 'Reading device %s' % devID_unique
                firstline = False
        
            if 'START' in line or 'start' in line: num_start_log += 1
            if 'END' in line or 'end' in line: num_end_log += 1 
            tokens = line.split(" ")  
            if len(tokens) != 12:
                print 'MALFORMED (# tokens=%d): %s' % (len(tokens), line)
                num_malformed_num_tokens += 1
            else:
                try:
                    month = int(tokens[0])
                    day = int(tokens[1])
                    hour = int(tokens[2])
                    minute = int(tokens[3])
                    second = int(tokens[4])
                    dev_id = int(tokens[5])
                    lat = float(tokens[6])
                    lon = float(tokens[7])
                    num_sat = int(tokens[8])
                    spd = float(tokens[9])
                    bat_perc = int(tokens[10])
                    time_since_boot = int(tokens[11])

                    if abs(lat - bm_ll[0]) > 2 or abs(lon - bm_ll[1]) > 2:    
                        for place, oob_ll in oob_dict.iteritems():
                            if abs(lat - oob_ll[0]) < 5 and abs(lon - oob_ll[1]) < 5:
                                print 'MALFORMED (latlon out of bounds -- %s): ' % place + line
                                num_malformed_oob[place] += 1
                    else:
                        if (lat, lon) in dupe_detect:
                            num_dupes += 1
                            #print "DUPE! " + line
                        else:
                            of.write("%f, %f\n" % (lat, lon))
                            num_good += 1
                            data[(devID_unique, dev_id)].append((month,day,hour,minute,second,lat,lon,num_sat,spd,bat_perc,time_since_boot))
                            dupe_detect[(lat, lon)] = 1
                except Exception, e:
                    print str(e)
                    print 'MALFORMED (malformed token): ' + line
                    num_malformed_bad_token += 1    
                    #raw_input()
        f.close()
    of.close()

    print 'using files:'
    for f in inputfiles: print f
    
    for dev_id, datalines in sorted(data.iteritems()):
        print 'Device %s: %5d good entries'  % (str(dev_id), len(datalines))
    
    print "num good entries: %d" % num_good
    print "num bad entries (# tokens): %d" % num_malformed_num_tokens
    print "num bad entries (bad token): %d" % num_malformed_bad_token 
    for place, numoob in num_malformed_oob.iteritems():
        print "num lat lon out of bounds %s: %d" % (place, numoob) 
    print "num dupes: %d" % num_dupes
    print "percentage good: %f" % (100.0*num_good/(num_good+num_malformed_num_tokens+num_malformed_bad_token))
    print "sanity check: %d log starts, %d log ends" % (num_start_log, num_end_log)
    print
    
    pickle.dump(data, open(ofName_pickle,'w'))
    
    print 'cleaned aggregate data log: %s' % ofName_cleaned
    print 'pickle file for playback: %s' % ofName_pickle
    
    return ofName_cleaned, ofName_pickle

def process_datum(key, datum):
    #data[(devID_unique, dev_id)].append((month,day,hour,minute,second,lat,lon,num_sat,spd,bat_perc,time_since_boot))
    dev_id = key[0] #keymap[key[0]]
    reported_id = key[1]
    month,day,hour,minute,second,lat,lon,num_sat,spd,bat_perc,time_since_boot = datum
    
    # note: this mainly prevents date=0 data from before gps latches on correctly
    if month <= 8 and day <= 23: 
        return 0, 0
    if month >= 9 and day >= 4: 
        return 0, 0
    time_in_s = second + minute*60 + hour*60*60 + day*24*60*60 + month*31*24*60*60
    return time_in_s, [dev_id, reported_id, lat, lon, num_sat, spd, bat_perc, time_since_boot, month, day, hour, minute, second]
    
def gen_playback_log(f_pickle, ofName='playback.txt'):
    playback_data = []
    
    dataset = pickle.load(open(f_pickle))
    for key, data in dataset.iteritems():
        for datum in data:
            time_in_s, metadata = process_datum(key, datum)
            if time_in_s > 0: playback_data.append([time_in_s, metadata])
            
    playback_data = sorted(playback_data)
    time0 = playback_data[0][0]
    for i in range(len(playback_data)):
        playback_data[i][0] -= time0
        
    #print 
    #print playback_data[:10]
    
    f = open(ofName,'w')
    for time_in_s, metadata in playback_data:
        dev_id, reported_id, lat, lon, num_sat, spd, bat_perc, time_since_boot, month, day, hour, minute, second = metadata
        f.write("%d %s %d %f %f %d %f %f %d %d %d %d %d %d\n" % (time_in_s, dev_id, reported_id, lat, lon, num_sat, spd, bat_perc, time_since_boot, month, day, hour, minute, second))
    f.close()
    
    print 'wrote playback file to %s' % ofName

if __name__ == '__main__':
    print 'usage: "python parse_logs.py outputfilename"'
    
    outputfilename = sys.argv[1]
    
    of_cleaned, of_pickle = clean_logs(outputfilename, dirs=['./logs_2017','./logs_2018'])
    
    print 'making google heatmap...'
    makegoogleheatmap(of_cleaned)
    
    print 'making playback file...'
    gen_playback_log(of_pickle, 'playback2018.txt')
    
    print 'done!'