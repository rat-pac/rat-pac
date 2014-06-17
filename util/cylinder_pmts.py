'''Create a PMTINFO RATDB table with positions of PMTs arranged on a
cylinder.

Note that PMTs are arranged on grids and so not all "target" numbers are
possible. Tweak this parameter until you get close to what you want.

Andy Mastbaum <mastbaum@hep.upenn.edu>, June 2014
'''

import argparse
import numpy as np

def disk(npmts, radius, holdoff=0, z=0, orient=1):
    npmts = int(npmts * (2*radius)**2 / (np.pi * radius**2))
    x = np.linspace(-radius, radius, int(np.sqrt(npmts)))
    y = np.linspace(-radius, radius, int(np.sqrt(npmts)))
    pos = []
    for i in range(len(x)):
        for j in range(len(y)):
            if x[i]**2 + y[j]**2 <= (radius - holdoff)**2:
                pos.append([x[i], y[j], z, 0, 0, 1.0 * np.sign(orient)])
    return pos

def tube(npmts, radius, height, holdoff=0):
    z = np.linspace(-height/2+holdoff, height/2-holdoff, int(np.sqrt(npmts)/2))
    phi = np.linspace(-np.pi, np.pi, int(np.sqrt(npmts)*2))
    pos = []
    for i in range(len(z)):
        for j in range(len(phi)):
            xx = radius * np.cos(phi[j])
            yy = radius * np.sin(phi[j])
            zz = z[i]
            uu = np.cos(np.pi - phi[j])
            vv = np.cos(np.pi / 2 + phi[j])
            ww = np.cos(np.pi/2)
            pos.append([xx, yy, zz, uu, vv, ww])
    return pos

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Cylindrical detector PMT placement')
    parser.add_argument('--radius', '-r', type=float, default=5000,
                        help='Cylinder radius (mm)')
    parser.add_argument('--height', '-z', type=float, default=10000,
                        help='Cylinder height (mm)')
    parser.add_argument('--npmts', '-n', type=int, default=1000,
                        help='Target number of PMTs')
    parser.add_argument('--holdoff', '-d', type=float, default=250,
                        help='Dead distance at plane/tube corners (mm)')
    parser.add_argument('--output', '-o', default='PMTINFO.ratdb',
                        help='Ouput RATDB filename')
    args = parser.parse_args()

    disk_area = np.pi * args.radius**2
    tube_area = 2 * np.pi * args.radius * args.height

    disk_fraction = disk_area / (8 * disk_area + tube_area)
    tube_fraction = tube_area / (8 * disk_area + tube_area)

    top =    disk(int(args.npmts * disk_fraction),
                  args.radius, args.holdoff, z=args.height/2, orient=-1)
    bottom = disk(int(args.npmts * disk_fraction),
                  args.radius, args.holdoff, z=-args.height/2, orient=1)
    sides =  tube(args.npmts * tube_fraction,
                  args.radius, args.height, args.holdoff)

    everything = np.array(top + bottom + sides)

    print 'Positioned', len(everything), 'PMTs'

    smap = lambda x: '%1.6f' % x

    x = '[' + ', '.join(map(smap, everything[:,0])) + ']'
    y = '[' + ', '.join(map(smap, everything[:,1])) + ']'
    z = '[' + ', '.join(map(smap, everything[:,2])) + ']'
    u = '[' + ', '.join(map(smap, everything[:,3])) + ']'
    v = '[' + ', '.join(map(smap, everything[:,4])) + ']'
    w = '[' + ', '.join(map(smap, everything[:,5])) + ']'
    t = '[' + ', '.join(map(str, np.ones_like(everything[:,0], dtype=int))) + ']'

    ratdb_template = '''{
"name": "PMTINFO",
"valid_begin": [0, 0],
"valid_end": [0, 0],
"x": %s,
"y": %s,
"z": %s,
"dir_x": %s,
"dir_y": %s,
"dir_z": %s,
"type": %s
}

'''

with open(args.output, 'w') as f:
    f.write(ratdb_template % (x, y, z, u, v, w, t))
    print 'Wrote positions to', args.output

