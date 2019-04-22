import serial, sys

if __name__ == '__main__':
    print 'usage: "python read_logs_from_device.py COM10 output0.txt"'

    print 'using port: %s' % sys.argv[1]
    print 'writing to file: %s' % sys.argv[2]
    
    ser = serial.Serial(port=sys.argv[1], baudrate=9600)
    
    print 'connected to port!'
    print 'waiting to capture input from serial line'

    f = open(sys.argv[2],"w")

    i = 0
    while True:
        line = ser.readline()
        #print line
        print 'reading line %5d' % i
        i += 1
        f.write(line)
        f.flush()
    f.close()

    print 'done!'