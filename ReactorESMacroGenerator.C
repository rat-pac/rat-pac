{
    double U235concentration, U238concentration, Pu239concentration, Pu241concentration;
    bool condition, visualization;
    double power, energyperfission, standoff, time, watervolume;
    double x, y, z;
    
    string choice;
    
    do {
        cout << "\nEnter the isotopic fission fractions for U235, U238, Pu239, and Pu241";
        cout << "\nU235: ";  cin >> U235concentration;
        cout << "U238: ";  cin >> U238concentration;
        cout << "Pu239: "; cin >> Pu239concentration;
        cout << "Pu241: "; cin >> Pu241concentration;
        
        if (U235concentration + U238concentration + Pu239concentration + Pu241concentration != 1.0){
            cout << "\n*** Those fission fractions do not add up to one, please try again ***\n";
            condition = true;
        }
        
        else {
            condition = false;
        }
        
    } while (condition);

    cout << "\nEnter the power level of the reactor (in GWth): ";
    cin >> power;
    
    cout << "\nEnter the energy released per fission in MeV (typically we use 200 MeV): ";
    cin >> energyperfission;
    
    cout << "\nEnter the reactor-detector distance (in km): ";
    cin >> standoff;
    
    cout << "\nEnter the acquisition time (in years): ";
    cin >> time;
    
    cout << "\nEnter water volume (in kilotons): ";
    cin >> watervolume;
    
    
    TF1 * U235foldedspectrum  = new TF1("U235spectrum", "(exp(0.870+(-0.160*x)+(-0.0910*x*x)))*(7.8*pow(10,-45)*0.511*x)",0,8);
    TF1 * U238foldedspectrum  = new TF1("U238spectrum", "(exp(0.976+(-0.162*x)+(-0.0790*x*x)))*(7.8*pow(10,-45)*0.511*x)",0,8);
    TF1 * Pu239foldedspectrum = new TF1("Pu239spectrum","(exp(0.896+(-0.239*x)+(-0.0981*x*x)))*(7.8*pow(10,-45)*0.511*x)",0,8);
    TF1 * Pu241foldedspectrum = new TF1("Pu241spectrum","(exp(0.793+(-0.080*x)+(-0.1085*x*x)))*(7.8*pow(10,-45)*0.511*x)",0,8);
    
    U235foldedspectrum->SetNpx(10000);
    U238foldedspectrum->SetNpx(10000);
    Pu239foldedspectrum->SetNpx(10000);
    Pu241foldedspectrum->SetNpx(10000);

    double fissionrate = (power*pow(10,9)) * 6.241509*pow(10,12) * (1./energyperfission);

    double U235fissionrate  = fissionrate * U235concentration;
    double U238fissionrate  = fissionrate * U238concentration;
    double Pu239fissionrate = fissionrate * Pu239concentration;
    double Pu241fissionrate = fissionrate * Pu241concentration;
    
    double num_electrons = (watervolume*pow(10,9)) * (1./18.01528) * (6.022*pow(10,23)) * 10.;
    
    double U235_integral  = U235foldedspectrum ->Integral(0,8);
    double U238_integral  = U238foldedspectrum ->Integral(0,8);
    double Pu239_integral = Pu239foldedspectrum->Integral(0,8);
    double Pu241_integral = Pu241foldedspectrum->Integral(0,8);
    
    double prefactor = num_electrons/(4. * 3.14159 * (standoff*pow(10,5)) * (standoff*pow(10,5)));
    
    double result = prefactor * ((U235fissionrate*U235_integral) + (U238fissionrate*U238_integral) + (Pu239fissionrate*Pu239_integral) + (Pu241fissionrate*Pu241_integral)) * (time*3600.*24.*365.);
    
    //cout << "\n==> Number of expected interactions = " << result  << "\n\n";
    
    
    cout << "\n\nI will now write the ES macro file for WATCHMAN...\n\n";
    cout << "Would you like visualization included? (y/n): ";
    
    cin >> choice;
    if (choice == "y" || choice == "Y" || choice == "Yes" || choice == "yes"){visualization = true;}
    else if (choice == "n" || choice == "N" || choice == "No" || choice == "no"){visualization = false;}
    else {
        cout << "I do not recognize that command, so you don't get visualization...\n";
        visualization = false;
    }
    
    cout << "\nEnter the incident direction direction vector (x,y,z) (dont worry, I'll normalize it)\n";
    cout << "x: "; cin >> x;
    cout << "y: "; cin >> y;
    cout << "z: "; cin >> z;
    
    fstream myfile;
    myfile.open("watchman_reactor_es.mac",ios::in | ios::out | ios::trunc);
    myfile << fixed << setprecision(4);
    
    myfile << "#Set the detector parameters\n";
    myfile << "/rat/db/set DETECTOR experiment \"Watchman\"\n";
    myfile << "/rat/db/set DETECTOR geo_file \"Watchman/Watchman.geo\"\n\n";
    
    myfile << "/run/initialize\n";
    myfile << "/process/activate Cerenkov\n\n";
    
    if (visualization){
        myfile << "/vis/open OGLSX 1000x100\n";
        myfile << "#/vis/open VRML2FILE\n";
        myfile << "/vis/scene/create\n";
        myfile << "/vis/scene/add/volume\n";
        myfile << "/vis/sceneHandler/attach\n";
        myfile << "/vis/scene/add/trajectories smooth\n";
        myfile << "/vis/modeling/trajectories/create/drawByCharge\n";
        myfile << "/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true\n";
        myfile << "/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 2\n";
        myfile << "/vis/scene/add/trajectories\n";
        myfile << "/vis/scene/add/hits\n";
        myfile << "/vis/viewer/set/viewpointVector 1 0.5 0.5\n";
        myfile << "/vis/modeling/trajectories/create/drawByParticleID\n";
        myfile << "/vis/modeling/trajectories/drawByParticleID-0/set e- blue\n";
        myfile << "/vis/modeling/trajectories/drawByParticleID-0/set geantino green\n";
        myfile << "/vis/modeling/trajectories/drawByParticleID-0/set opticalphoton yellow\n";
        myfile << "/vis/viewer/set/autoRefresh true\n";
        myfile << "#/vis/scene/endOfEventAction accumulate\n";
        myfile << "/tracking/FillPointCont true\n\n";
    
        myfile << "/tracking/storeTrajectory 1\n";
        myfile << "/vis/viewer/refresh\n";
        myfile << "/vis/viewer/flush\n\n";
    }
    
    myfile << "# BEGIN EVENT LOOP\n";
    myfile << "/rat/proc simpledaq\n";
    myfile << "#/rat/proc fitbonsai\n";
    myfile << "/rat/proc count\n";
    myfile << "/rat/procset update 10\n\n";
    
    myfile << "/rat/proclast outroot\n";
    myfile << "/rat/procset file \"watchman.root\"\n";
    myfile << "#END EVENT LOOP\n\n";
    
    myfile << "/generator/add combo reactor_es:point\n";
    myfile << "/generator/vtx/set " << x/(fabs(x+y+z)) << " " << y/(fabs(x+y+z)) << " " << z/(fabs(x+y+z)) << "\n";
    myfile << "/generator/pos/set 0 0 0\n";
    myfile << "/generator/reactor_es/U235 "  << U235concentration  <<"\n";
    myfile << "/generator/reactor_es/U238 "  << U238concentration  << "\n";
    myfile << "/generator/reactor_es/Pu239 " << Pu239concentration << "\n";
    myfile << "/generator/reactor_es/Pu241 " << Pu241concentration << "\n\n";
    
    myfile << "/run/beamOn " << int(result) << "\n";

    myfile.close();

    
    
    gROOT->ProcessLine(".q");
}


