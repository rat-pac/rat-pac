# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                                                   #
#   Simple script to generator a macro file for reactor ES in       #
#   WATCHMAN. Can take in any isotopics, power level, standoff,     #
#   acquisition time, target water volume size, and antineutrino    #
#   direction. Will set up visualization, if requested, and         #
#   calculate the number of events to run.                          #
#                                                                   #
#   To run, simply enter $ python ReactorESMacroGenerator.py        #
#                                                                   #
#        ~ Created by Daniel Hellfeld (07/2015) ~                   #
#                                                                   #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Imports
import math as math
import ROOT as ROOT

# Get the isotopic fission fractions. Make sure they sum to 1
while True:
	print '\nEnter the isotopic fission fractions for U235, U238, Pu239, and Pu241'
	U235concentration  = float(raw_input('\nU235: '))
	U238concentration  = float(raw_input('U238: '))
	Pu239concentration = float(raw_input('Pu239: '))
	Pu241concentration = float(raw_input('Pu241: '))
	if (U235concentration + U238concentration + Pu239concentration + Pu241concentration != 1.0):
		print '\n*** Those fission fractions do not add up to one, please try again ***\n'
	else:
		break

# Get some needed paramters
power            = float(raw_input('\nEnter the power level of the reactor (in GWth): '))
energyperfission = float(raw_input('\nEnter the energy released per fission in MeV (typically we use 200 MeV): '))
standoff         = float(raw_input('\nEnter the reactor-detector distance (in km): '))
time             = float(raw_input('\nEnter the acquisition time (in years): '))
watervolume      = float(raw_input('\nEnter water volume (in kilotons): '))

# Construct antineutrino spectra for each isotope. Fold with scattering cross-section
U235foldedspectrum  = ROOT.TF1('U235spectrum', '(exp(0.870+(-0.160*x)+(-0.0910*x*x)))*(7.8*pow(10,-45)*0.511*x)',0,8)
U238foldedspectrum  = ROOT.TF1('U238spectrum', '(exp(0.976+(-0.162*x)+(-0.0790*x*x)))*(7.8*pow(10,-45)*0.511*x)',0,8)
Pu239foldedspectrum = ROOT.TF1('Pu239spectrum','(exp(0.896+(-0.239*x)+(-0.0981*x*x)))*(7.8*pow(10,-45)*0.511*x)',0,8)
Pu241foldedspectrum = ROOT.TF1('Pu241spectrum','(exp(0.793+(-0.080*x)+(-0.1085*x*x)))*(7.8*pow(10,-45)*0.511*x)',0,8)

# Set some parameters for the functions above
U235foldedspectrum. SetNpx(10000)
U238foldedspectrum. SetNpx(10000)
Pu239foldedspectrum.SetNpx(10000)
Pu241foldedspectrum.SetNpx(10000)

# Calculate the total fission rate based off the power and energy released per fission
fissionrate = (power*(1e9)) * (6.241509*(1e12)) * (1./energyperfission)

# Break up the fission rate into the 4 isotopes
U235fissionrate  = fissionrate * U235concentration
U238fissionrate  = fissionrate * U238concentration
Pu239fissionrate = fissionrate * Pu239concentration
Pu241fissionrate = fissionrate * Pu241concentration

# Calculate the total number of availeble electrons in our water volume
num_electrons = (watervolume*(1e9)) * (1./18.01528) * (6.022*(1e23)) * 10.

# Integrate the folded spectra over the entire energy range
U235_integral  = U235foldedspectrum. Integral(0,8)
U238_integral  = U238foldedspectrum. Integral(0,8)
Pu239_integral = Pu239foldedspectrum.Integral(0,8)
Pu241_integral = Pu241foldedspectrum.Integral(0,8)

# Calculate the number of expected interactions
result = (num_electrons/(4. * 3.14159 * ((standoff*(1e5))**2))) * ((U235fissionrate*U235_integral) + (U238fissionrate*U238_integral) + (Pu239fissionrate*Pu239_integral) + (Pu241fissionrate*Pu241_integral)) * (time*3600.*24.*365.)

# Start process of printing information to macro file
print '\n------------------------------------------------\n'
print '\n\nI will now write the reactor ES macro file for WATCHMAN and put it in the macro folder...\n\n'

# Do you want to include the visualization commands?
choice = raw_input('Would you like visualization included? (y/n): ')
if (choice == 'y' or choice == 'Y' or choice == 'Yes' or choice == 'yes'):
	visualization = True
elif (choice == 'n' or choice == 'N' or choice == 'No' or choice == 'no'):
	visualization = False
else:
	print 'I do not recognize that command, so you do not get visualization...\n'
	visualization = False

# Get incident antineutrino direction vector
print '\nEnter the incident direction direction vector (x,y,z) (dont worry, I will normalize it)\n'
x = float(raw_input('x: '))
y = float(raw_input('y: '))
z = float(raw_input('z: '))

# Open up macro file (** will delete prior contents **)
myfile = open('mac/watchman_reactor_es.mac','w')

myfile.write('#Set the detector parameters\n')
myfile.write('/rat/db/set DETECTOR experiment \"Watchman\"\n')
myfile.write('/rat/db/set DETECTOR geo_file \"Watchman/Watchman.geo\"\n\n')
myfile.write('/run/initialize\n')
myfile.write('/process/activate Cerenkov\n\n')
    
if (visualization):
	myfile.write('/vis/open OGLSX 1000x1000\n')
	myfile.write('#/vis/open VRML2FILE\n')
	myfile.write('/vis/scene/create\n')
	myfile.write('/vis/scene/add/volume\n')
	myfile.write('/vis/sceneHandler/attach\n')
	myfile.write('/vis/scene/add/trajectories smooth\n')
	myfile.write('/vis/modeling/trajectories/create/drawByCharge\n')
	myfile.write('/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true\n')
	myfile.write('/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 2\n')
	myfile.write('/vis/scene/add/trajectories\n')
	myfile.write('/vis/scene/add/hits\n')
	myfile.write('/vis/viewer/set/viewpointVector 1 0.5 0.5\n')
	myfile.write('/vis/modeling/trajectories/create/drawByParticleID\n')
	myfile.write('/vis/modeling/trajectories/drawByParticleID-0/set e- blue\n')
	myfile.write('/vis/modeling/trajectories/drawByParticleID-0/set geantino green\n')
	myfile.write('/vis/modeling/trajectories/drawByParticleID-0/set opticalphoton yellow\n')
	myfile.write('/vis/viewer/set/autoRefresh true\n')
	myfile.write('#/vis/scene/endOfEventAction accumulate\n')
	myfile.write('/tracking/FillPointCont true\n\n')
	myfile.write('/tracking/storeTrajectory 1\n')
	myfile.write('/vis/viewer/refresh\n')
	myfile.write('/vis/viewer/flush\n\n')
    
myfile.write('# BEGIN EVENT LOOP\n')
myfile.write('/rat/proc simpledaq\n')
myfile.write('#/rat/proc fitbonsai\n')
myfile.write('/rat/proc count\n')
myfile.write('/rat/procset update 10\n\n')
myfile.write('/rat/proclast outroot\n')
myfile.write('/rat/procset file \"watchman.root\"\n')
myfile.write('#END EVENT LOOP\n\n')

myfile.write('/generator/add combo reactor_es:point\n')
myfile.write('/generator/vtx/set %5.2f %5.2f %5.2f\n' %(x/(math.fabs(x+y+z)), y/(math.fabs(x+y+z)), z/(math.fabs(x+y+z))))
myfile.write('/generator/pos/set 0 0 0\n')
myfile.write('/generator/reactor_es/U235 %5.3f\n'  %U235concentration)
myfile.write('/generator/reactor_es/U238 %5.3f\n'  %U238concentration)
myfile.write('/generator/reactor_es/Pu239 %5.3f\n' %Pu239concentration)
myfile.write('/generator/reactor_es/Pu241 %5.3f\n' %Pu241concentration)

myfile.write('\n/run/beamOn %d\n\n' %result)

myfile.close()

print '\n\n'

