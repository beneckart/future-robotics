pitch = 5.0/25.0
rows = 20
cols = 25

xoffset = 2.5
yoffset = 2.5*rows/cols

pixels = []

xpos, ypos = 0, 0

for i, r in enumerate(range(rows)):
    for j, c in enumerate(range(cols)):
        fwd = i % 2 == 0
        if fwd:
            xpos = c*pitch
            ypos = r*pitch
        else:
            xpos = (cols-c-1)*pitch
            ypos = r*pitch
        pixels.append({"point":[xpos-xoffset, 0, ypos-yoffset]})

f = open('marquee.json','w')
f.write(str(pixels))
f.close()