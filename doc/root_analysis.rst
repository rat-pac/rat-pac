Off-line Analysis in ROOT
-------------------------
Introduction
````````````

`ROOT <http://root.cern.ch/>`_ is currently the tool of choice for analyzing RAT output for a variety of reasons which include that ROOT commands look like C++.  For a high-level look on how to use ROOT, you should consult the [http://www.slac.stanford.edu/BFROOT/www/doc/workbook/root1/root1.html BARBAR collaboration's ROOT page], because it is well done and there is no point to repeat that here.  The purpose of this article, as it stands, is to familiarize the user with ROOT analysis pertaining to RAT in particular.

Macros versus command-line
''''''''''''''''''''''''''

Like in RAT, it is better to use macros than command-line commands to perform certain tasks.  In both cases, this stems from the complexity of the commands and also because there happen to be a lot of commands needed to do just about anything.  It will be assumed here that you are using macros.

Starting your macro
```````````````````

Function definition and asthetics
'''''''''''''''''''''''''''''''''

Macros can be used to generate plots to disk in some easily accessible format like jpeg.  To start your macro you need to define a function and also you will need to change some of the visual information in ROOT.  For example, here is the top of $RATROOT/absorbtion.c, whose code is either commented or clearly purposed::


    void absorbtion()
    {
      //
      // First, let's re-set some graphical options, to overcome root's
      // disasterous defaults.
      //
      gROOT->SetStyle("Plain");
    
      gStyle->SetOptStat(0); // This determines if you want a stats box
      gStyle->SetOptFit(0); // This determines if you want a fit info box
      gStyle->GetAttDate()->SetTextColor(1);
      gStyle->SetOptTitle(0); // no title; comment out if you want a title
      gStyle->SetLabelFont(132,"XYZ");
      gStyle->SetTextFont(132);
      gStyle->SetTitleFont(132,"XYZ");
    
      gROOT->ForceStyle();

File access
'''''''''''

Now that you have your plots looking pretty, it is time to get information into ROOT so that way you actually have something to display.  To do this, you declare a a TFile object and a TTree object.  After declaring the TFile object, as shown below, to link to your ROOT output, then the TTree object links to the tree in the ROOT file.  The [http://en.wikipedia.org/wiki/Tree_data_structure tree] is just a way to store information, so it means the same thing here as it does in regular programming.  Here is an example::

    TFile absfile("../test_absorbtion.root");
    TTree *T1=(TTree*)absfile.Get("T");

Creating a histogram
''''''''''''''''''''

Now we have access to our data, but we need to create something like a histogram or n-dimensional plot to put the data in.  Earlier we had only defined how things should look, we have not defined a histogram or something of the like.  Histograms, in root are objects called TH1F (and if you figure out what it stands for, add it to this page).  The constructor to TH1F takes a few arguments, as seen below: the name, 
options, the number of bins, the minimal value and the maximum value.  The functions important functions that act on TH1F are well named, so you can figure out what they do just by looking at the example below::


    TH1F *noabs = new TH1F("noabs"," ", 200, 1500, 3000);
    noabs->SetLineColor(kBlue);
    noabs->SetLineWidth(3);
    noabs->SetLineStyle(1);
    noabs->SetXTitle("Number of photons hits per event (hits)");
    noabs->SetYTitle("Number of events (events)");
    noabs->SetTitle("Photons to hit PMTs with and without attenuation");

Filling the histogram
'''''''''''''''''''''

We now have a histogram, but it isn't being displayed anywhere, so it is time to fix that.  There is a function which acts upon the TTree object we defined earlier called "Draw" which takes data from the tree and puts it in some plot like a histogram.  So, for example::

    T1->Draw("numPE>>noabs");  // Puts "numPE" from file into "noabs" histogram

Rendering the histogram
'''''''''''''''''''''''

TH1F, like all plot objects like histograms, also has a function called Draw, which doesn't have to take arguements, that is used for writing to a canvas, where a canvis is the window you see on your screen.  If no canvas has been created, it creates one called "c1".  So after running the command::

    noabs->Draw();

You should see your plot.

Writing the image to disk
''''''''''''''''''''''''''

Here is an example of writing the image to disk in many formats::

    c1->Print("absorbtion.gif");
    c1->Print("absorbtion.eps");
    c1->Print("absorbtion.jpg");

And now you are done.

Further examples
````````````````

There are further examples in CVS in $RATROOT/root which cover a wide range of functions, such as drawing lengends, drawing multiple sets of data on the same canvas and so on and so forth.  These are worth taking a look at since most of what will ever need to be done in RAT has been done, and is available in the root directory.

