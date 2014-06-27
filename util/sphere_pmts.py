'''Create a PMTINFO RATDB table with positions of PMTs arranged on a
sphere in horizontal bands.

Note that PMTs are arranged on grids and so not all "target" numbers are
possible. Tweak this parameter until you get close to what you want.

Andy Mastbaum <mastbaum@hep.upenn.edu>, June 2014
'''

import argparse
import numpy as np

def sphere(npmts, radius):
    A = lambda z: np.pi * radius**2 * np.sqrt(1 - np.abs(z/radius))
    nz = int(np.sqrt(npmts) / np.pi * 2)
    theta = np.linspace(0, np.pi, nz)
    z = radius * np.cos(theta)
    equator_pmts = int(np.sqrt(npmts) * np.pi / 2)
    pos = []
    for i in range(len(z)):
        n = A(z[i]) / (np.pi * radius**2) * equator_pmts
        phi = np.linspace(-np.pi, np.pi, n)
        for j in range(len(phi)):
            xx = radius * np.sin(np.arccos(z[i]/radius)) * np.cos(phi[j])
            yy = radius * np.sin(np.arccos(z[i]/radius)) * np.sin(phi[j])
            zz = z[i]
            pos.append([xx, yy, zz])
    return pos
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Spherical detector PMT placement')
    parser.add_argument('--radius', '-r', type=float, default=8500,
                        help='Sphere radius (mm)')
    parser.add_argument('--npmts', '-n', type=int, default=1000,
                        help='Target number of PMTs')
    parser.add_argument('--output', '-o', default='PMTINFO.ratdb',
                        help='Ouput RATDB filename')
    args = parser.parse_args()

    points = np.array(sphere(2 * args.npmts, args.radius))

    print 'Positioned', len(points), 'PMTs'

    smap = lambda x: '%1.6f' % x

    x = '[' + ', '.join(map(smap, points[:,0])) + ']'
    y = '[' + ', '.join(map(smap, points[:,1])) + ']'
    z = '[' + ', '.join(map(smap, points[:,2])) + ']'
    t = '[' + ', '.join(map(str, np.ones_like(points[:,0], dtype=int))) + ']'

    ratdb_template = '''{
"name": "PMTINFO",
"valid_begin": [0, 0],
"valid_end": [0, 0],
"x": %s,
"y": %s,
"z": %s,
"type": %s
}

'''

with open(args.output, 'w') as f:
    f.write(ratdb_template % (x, y, z, t))
    print 'Wrote positions to', args.output

