Creating and Running `rattest` Tests
------------------------------------

Introduction
````````````

Rattest is a framework for creating unit and functional tests for RAT. These tests should be simple, testing only one aspect of the simulation. For instance, a test of attenuation in acrylic consist of a single light source in a world volume of acrylic -- no PMTs or other geometry. 

At minimum, a test consists of a RAT macro and a ROOT macro -- the Monte Carlo and the analysis. New (simplified) geometries, modified RATDB databases, etc. can also be included. When run, these tests are compared to a standard via a KS test, and a web page is created with histograms (standard and current) and KS test results. The standard RAT logs and output ROOT file is also available for analysis.

The existing rattests are included with the standard RAT distribution, in `$RATROOT/test/`, with the functional tests in `$RATROOT/test/full/<test-name>`. To run a single test, `cd` to the test directory and simply run `rattest <test-name>` where `<test-name>` corresponds to a folder in `$RATROOT/test/full`. Rattest will iterate through the directory structure to find the test, run the RAT macro, run the ROOT macro on the output, and generate a report page.

The `rattest` utility takes the following options::

    Usage: rattest [options]
    
    Options:
      -h, --help         show this help message and exit
      -u, --update       Update "standard" histogram with current results
      -m, --regen-mc     Force Monte Carlo to be regenerated
      -r, --regen-plots  Force histograms to be regenerated
      -t, --text-only    Do not open web pages with plots

Existing RAT Tests
``````````````````

::

    acrylic_attenuation
     Tests the attenuation length of acrylic by generating photons in an acrylic block and checking track lengths

Writing a RAT Test
``````````````````

1. Create a new folder in `$RATROOT/test/full` with useful but short name for your test
2. Create a `rattest.config` file, like this::

    #!python
    # -*- python-*-
    description = '''Tests the attenuation length of acrylic by generating photons in an acrylic block and checking track lengths'''
  
    rat_macro = 'acrylic_attenuation.mac'
    root_macro = 'acrylic_attenuation.C'

The RAT macro and ROOT macro do not need to have the same name as the test, they just have to be consistent with the actual filenames. `rattest` will find your ROOT output file, so you don't have to worry about it.

3. If necessary, create a RAT geometry (.geo) and any modified RATDB (.ratdb). As an example, `acrylic_attenuation` uses the default RATDBs (the default behavior), but the following geometry::

    // -------- GEO[world]
    {
    name: "GEO",
    index: "world",
    valid_begin: [0, 0],
    valid_end: [0, 0],
    mother: "",
    type: "box",
    size: [10000.0, 10000.0, 10000.0],
    material: "acrylic_polycast",
    }

RAT will prefer a geometry or database in your test directory, and default to the ones in `$RATROOT/data`.

4. Create your RAT macro.

Keep things as simple as possible, and turn off as many options as possible. The `acrylic_attenuation` RAT macro::

    /glg4debug/glg4param omit_muon_processes  1.0
    /glg4debug/glg4param omit_hadronic_processes  1.0
    
    /rat/db/set DETECTOR geo_file "acrylic_sphere.geo"
    
    /run/initialize
    
    # BEGIN EVENT LOOP
    /rat/proc count
    /rat/procset update 50
    
    /rat/proc outroot
    /rat/procset file "acrylic_attenuation.root"
    
    # END EVENT LOOP
    /tracking/storeTrajectory 1
    
    /generator/add combo pbomb:point
    /generator/vtx/set 100 100
    /generator/pos/set  0.0 0.0 0.0 
    
    
    /generator/add combo pbomb:point
    /generator/vtx/set 100 200
    /generator/pos/set  0.0 0.0 0.0
    
    ...
    
    /run/beamOn 500

5. Write a ROOT macro

The ROOT macro should create a histogram that captures the benchmark you are looking for. It should consist of a single `void` function `make_plots(TFile *event_file, TTree *T, TFile *out_file)`.

Basically, do your analysis, make a histogram, and output it with `[histogram name]->Write()`. Note that when using `Draw()` to make histograms, you'll probably want the `"goff"` option.

`rattest` will pull histogram names from this macro automatically for creation of the results page.

The ROOT macro from `acrylic_attenuation`::

    void make_plots(TFile *event_file, TTree *T, TFile *out_file)
    {
    
     ...
    
      TH1F *acr_attn_300 = new TH1F("acr_attn_300", "Photon track length (300 nm)", 20, 0, 2500);
      acr_attn_300->SetXTitle("Track length (mm)");
      acr_attn_300->SetYTitle("Count");
      T->Draw("mc.track.GetLastMCTrackStep()->length>>acr_attn_300","TMath::Abs(1.23997279736421566e-03/(mc.track.GetLastMCTrackStep()->ke)-300)<10","goff");
      acr_attn_300->Fit("expo");
      acr_attn_300->Draw("goff");
      acr_attn_300->Write();
    
     ...
    
    }

6. Test it

 Run your RAT macro with the usual `rat [macro name]`, then, in ROOT, run the contents of your analysis macro and ensure that you get what you were looking for.

7. Create a standard

 From the test directory, run `rattest -u [your test name]`. This will create the file `standard.root`, which will be the basis for comparison until the next time you run `rattest` with the `-u` option. Take a look at `results.html` to see how things worked out.

This is pretty much it. If you run `rattest [your test name]` again, you should get a results page (which will open in your default browser unless you specified the `-t` option) with very similar results.

If you think the test is useful to others, commit it to the RAT repository with svn. Be sure to commit only the `rattest.config`, RAT and ROOT macro, any geometry or RATDB files, and `standard.root`.

