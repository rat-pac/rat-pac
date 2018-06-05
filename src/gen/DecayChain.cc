#include <RAT/DecayChain.hh>
#include <RAT/FermiFunction.hh>

#include <Randomize.hh>

#include <CLHEP/Vector/LorentzVector.h>
#include <cstring>
#include <cmath>
#include <iostream>

#undef DEBUG

namespace RAT {

  DecayChain::DecayChain()
  {
    Reset();
  }

  DecayChain::DecayChain(const std::string Name)
  {
#ifdef DEBUG
    std::cout << "In RAT:: DecayChain::DecayChain(), with Name="
	      << Name << std::endl;
#endif

    Reset();
    SetChainName(Name);
  }

  DecayChain::~DecayChain()
  {
    Reset();
  }
 
  void DecayChain::Reset()
  {
#ifdef DEBUG
    std::cout << "In RAT:: DecayChain::Reset()" << std::endl;
#endif

    SetVerbose();
    SetEquilibrium();
    SelectParent();
    IncludeDaughter(true);

    isChainElement = false;

    SetChainName("");

    NumberOfElements = 0;

    tstart = 0.;
    SetNGenerated(0);

    // See if the $RATDecayDataDir environment variable exists.
    char* path = getenv("RATDecayDataDir");
    std::string dirName;
    if ( path == 0 )
      {
	// RATDecayDataDir enviornment variable not found.  
	// Check to see if RATROOT is set
	path = getenv("RATROOT");
	if( path == 0 )
	  {
	    // Environment variable not found... so just use a name
	    // relative to the current directory.
	    dirName = "./data";
	  }
	else
	  {
	    // RATROOT enviornment variable found.
	    // Add data subdir to get the correct directory. 
	    dirName = std::string(path) + "/data";
	  }
      }
    else
      {
	dirName = std::string(path);
      }
    std::string fileName = dirName + "/beta_decays.dat";
    SetFileName(fileName);
  }

  void DecayChain::Show()
  {
    printf("Decay Chain Name: %s .\n", GetChainName().c_str());
    int nE = GetNElements();
    if (nE > 0) {
      printf("Elements : %d \n", nE);
      printf
	("Name \t Branch \t Decay Type \t Half-Life \t Weight \t Prob \t Level \n");
      for (int i = 0; i < nE; i++) {
	printf("%s \t %d \t %d \t %g \t %g \t %g \t %d \n", GetElementName(i).c_str(),
	       GetElementBranch(i), GetElementDecay(i), GetLifetime(i),
	       GetElementWt(i), GetElementProb(i), CheckChainLevel(i));
      }
    } else {
      printf("No elements defined. \n");
    }
    printf(" \n");
  }

  void DecayChain::AddElement(const std::string Name, int iLocation, int iDecay,
			      double tau, double wt)
  {
#ifdef DEBUG
    std::cout << "RAT:: DecayChain::AddElement()" << std::endl;
#endif

    int iChain = GetNElements();
  
    Element[iChain] = new BetaFunction(Name);
    Element[iChain] -> SetFileName(GetFileName());
#ifdef DEBUG
    Element[iChain] -> SetVerbose(true);
#endif

    bool iFound = Element[iChain]->ReadInputFile(Name, iDecay);
  
    SetElementName(iChain, Name);
    SetElementBranch(iChain, iLocation);
    SetElementDecay(iChain, iDecay);
    SetLifetime(iChain, tau);
    SetElementWt(iChain, wt);
  
    NumberOfElements++;
  
    SetDecaySequence();
  
    if (isVerbose) {
      if (!iFound)
	printf("No such element found in decay scheme. \n");
      Element[iChain]->Show();
    }
  }

  void DecayChain::SetElementName(int iBranch, const std::string Name)
  {
    if (iBranch <= NumberOfElements) {
      ElementName[iBranch] = Name;
    }
  }

  void DecayChain::SetElementNumber(int iBranch)
  {
    if (iBranch <= NumberOfElements)
      ElementNumber = iBranch;
  }

  void DecayChain::SetElementBranch(int iBranch, int iLocation)
  {
    if (iBranch <= NumberOfElements)
      ElementBranch[iBranch] = iLocation;
  }

  void DecayChain::SetElementWt(int iBranch, double wt)
  {
    if (iBranch <= NumberOfElements)
      ElementWeight[iBranch] = wt;
  }

  void DecayChain::SetElementDecay(int iBranch, int iDecay)
  {
    if (iBranch <= NumberOfElements)
      ElementDecay[iBranch] = iDecay;
  }

  void DecayChain::SetLifetime(int iBranch, double tau)
  {
    if (iBranch <= NumberOfElements)
      ElementLife[iBranch] = tau;
  }

  bool DecayChain::ReadInputFile(const std::string dName)
  {
  
    std::string dProbe = "CHAIN:";
    std::string eProbe = "END";
    char dummy[80];
    char tName[80];
    char sName[80];
  
    bool iFound = false;
  
    int eP = 0;
    int iChain, iDecay;
    float weight, tau;
  
    FILE *inputFile;
  
    inputFile = fopen(GetFileName().c_str(), "r");
  
    if (!inputFile) {
      printf("Error opening file : %s \n", GetFileName().c_str());
      return iFound;
    }
  
    bool iRead = true;
  
    while ((iRead) && (!iFound)) {
      iRead = (fscanf(inputFile, "%s", dummy) > 0);
      std::string iString(dummy);

      if (iString == dProbe) 
	{
	  fscanf(inputFile, "%s", tName);
	  std::string iString2(tName);
	  if (dName == iString2) 
	    {
	      iFound = true;
	      fscanf(inputFile, "%d", &eP);
	      if (isVerbose)
		printf("Reading %s \n \n", tName);
	      for (int j = 0; j < eP; j++) 
		{
		  fscanf(inputFile, "%s %d %f %d %f", sName, &iChain,
			 &weight, &iDecay, &tau);
		  std::string iString3(sName);
		  if(iDecay != NullParticle) {
		    AddElement(iString3, iChain, iDecay, (double) tau,
			       (double) weight);
		  }
		}
	    }
	  else if (iString == eProbe) 
	    {
	      iRead = false;
	    }
	}
    } // end while
  
    fclose(inputFile);
    if (iFound) {
      if (isVerbose)
	Show();
    } else {
      printf("No such chain found: %s .\n  Looking for element only...\n", dName.c_str());
      AddElement(dName);
      iFound = true;
    }
    return iFound;
  }

  const std::string DecayChain::GetElementName(int iBranch)
  {
    if (iBranch > GetNElements())
      return "";
    return ElementName[iBranch];
  }

  int DecayChain::GetElementBranch(int iBranch)
  {

    if (iBranch > GetNElements())
      return 0;

    return ElementBranch[iBranch];

  }

  int DecayChain::GetElementDecay(int iBranch)
  {

    if (iBranch > GetNElements())
      return 0;

    return ElementDecay[iBranch];

  }

  double DecayChain::GetElementWt(int iBranch)
  {

    if (iBranch > GetNElements())
      return 0.;

    return ElementWeight[iBranch];

  }

  double DecayChain::GetLifetime(int iBranch)
  {

    if (iBranch > GetNElements())
      return 0.;

    return ElementLife[iBranch];

  }

  void DecayChain::GenerateDecayElement(int i)
  {
  
    int nE = GetNElements();
  
    if (nE <= 0) return;
  
    Element[i]->GenerateEvent();
  
    int nP = Element[i]->GetNGenerated();
    tstart += Element[i]->GetEventTime();

    if (nP > 0) {

      int iStart = GetNGenerated();

      for (int j = 0; j < nP; j++) {
	int k = iStart + j;
	SetParentName(k, Element[i]->GetName());
	int iNext = i;
	if (i > GetNElements()-1) {
	  for(int q = GetNElements()-1; q > i; q--){
	    if (CheckInChain(i,q)) iNext = q;
	  }
	}
	SetDaughterName(k, Element[iNext]->GetName());
	ParticleID[k]     = Element[i]->GetEventID(j);
	ParticleEnergy[k] = Element[i]->GetEventEnergy(j);
	SetEventTime(k,tstart);
	SetParticleInfo(i, j, k);
      }

      bool iDaughter = ((CreateDaughter) && (isEquilibrium));
      if (iDaughter){
	int k = iStart + nP;
	int isoID = 100000000;
	isoID += (int) (Element[i]->GetTargetMass()) * 1000;
	isoID += (int) (Element[i]->GetCharge());
	ParticleInfo_t theDaughter = AddDaughterInfo(i);
	SetParentName(k, Element[i]->GetName());
	int iNext = i;
	if (i > GetNElements()-1) {
	  for(int q = GetNElements()-1; q > i; q--){
	    if (CheckInChain(i,q)) iNext = q;
	  }
	}
	SetDaughterName(k, Element[iNext]->GetName());
	ParticleID[k]     = isoID;
	ParticleEnergy[k] = theDaughter.vector.e();
	SetEventTime(k,tstart);
	theParticleInfo[k] = theDaughter;
	nP++;
      }
    }
  
    SetNGenerated(GetNGenerated()+nP);
    SetElementNumber(i);  
  }

  void DecayChain::GenerateDecayElement(const std::string iElement)
  {
    for(int i=0; i<NumberOfElements; i++) {
      const std::string tName = GetElementName(i);
      if(tName == iElement) {
	GenerateDecayElement(i);
	return;
      }
    }
  }

  void DecayChain::GenerateDecayChain(const std::string iNameStart)
  {

    SetEquilibrium(true);
  
    int nE = GetNElements();
    int iStart = 0;
    int iEnd = nE;
  
    if (nE <= 0)
      return;

    for (int i = iStart; i < iEnd; i++) {
      const std::string tName = GetElementName(i);
      if (tName == iNameStart)
	iStart = i;
    }

    double rMax = 0.;
    for (int i = iStart; i < iEnd; i++)
      rMax = rMax + GetElementProb(i);

    SetNGenerated(0);
    isChainElement = false;
    double r = rMax * GetRandomNumber();
    double Prob = 0.;
    int iTag = -1;
    for (int i = iStart; i < iEnd; i++) {
      Prob = Prob + GetElementProb(i);
      if ((iTag < 0) && (r < Prob))
	iTag = i;
    }

    tstart = 0.;
    GenerateDecayElement(iTag);
  }

  void DecayChain::GenerateDecayChain(int iBranch)
  {
    int nE = GetNElements();
    if (iBranch > nE)
      return;
    const std::string tName = GetElementName(iBranch);
    GenerateDecayChain(tName);
  }

  const std::string DecayChain::GetParentName(int iEvent)
  {
    if (iEvent > GetNGenerated()) return "";
    return ParentName[iEvent];
  }

  const std::string DecayChain::GetDaughterName(int iEvent)
  {
    if (iEvent > GetNGenerated()) return "";
    return DaughterName[iEvent];
  }

  int DecayChain::GetEventID(int iEvent)
  {
    if (iEvent > GetNGenerated()) return 0;
    if (!CheckParentMatch(iEvent)) return 0;

    return ParticleID[iEvent];
  }

  double DecayChain::GetEventEnergy(int iEvent)
  {
    if (iEvent > GetNGenerated()) return 0.;
    if (!CheckParentMatch(iEvent)) return 0;

    return ParticleEnergy[iEvent];
  }

  double DecayChain::GetEventTime(int iEvent)
  {
    if (iEvent > GetNGenerated()) return 0.;
    if (!CheckParentMatch(iEvent)) return 0;

    return ParticleTime[iEvent];
  }

  void DecayChain::SetEventTime(int iEvent, double time)
  {
    ParticleTime[iEvent] = time;
  }

  void DecayChain::GenerateFullChain(const std::string iElement)
  {

    SetEquilibrium(false);
    int nE = GetNElements();
    if (nE <= 0) return;
  
    int iStart = 0;
    for (int i = 0; i < nE; i++) {
      const std::string tName = GetElementName(i);
    
      if (tName == iElement)
	iStart = i;
    
    }
  
    int iBefore = iStart;
    int iReject = -1;
    double Br = 1.;
    double RelWt = 1.;
    tstart = 0.;

    SetNGenerated(0);
    isChainElement = false;
  
    for (int j = iStart; j < nE; j++) {
      bool iCheck = (CheckInChain(j, iStart) && CheckInChain(j, iBefore));

      if (iReject > 0)
	iCheck = ((iCheck) && (!CheckInChain(j, iReject)));

      if (iCheck) {
	Br = GetElementWt(j);
	double Prob = RelWt * GetRandomNumber();
	if (Prob <= Br) {
	  GenerateDecayElement(j);
	  iBefore = j;
	  iReject = -1;
	  RelWt = 1.;
	} else {
	  RelWt = RelWt - Br;
	  iReject = j;
	}
      }
      isChainElement = true;
    }
    isChainElement = false;

    double tMax = 0.;
    for(int i=0;i<GetNGenerated();i++){
      if (GetEventTime(i)>tMax) tMax = GetEventTime(i);
    }
    for(int i=0;i<GetNGenerated();i++) SetEventTime(i,GetEventTime(i)-tMax);
  }

  void DecayChain::GenerateFullChain(int iBranch)
  {
    int nE = GetNElements();
    if (iBranch > nE)
      return;
    const std::string tName = GetElementName(iBranch);
    GenerateFullChain(tName);
  }

  bool DecayChain::CheckParentMatch(int iEvent)
  {
    bool iPass = false;
  
    if (oneParent == "all") iPass = true;
    if (oneParent == "ALL") iPass = true;
    if (oneParent == "0/")  iPass = true;
  
    if (oneParent == GetParentName(iEvent)) iPass = true;
  
    return iPass;
  }

  void DecayChain::SetDecaySequence()
  {
    int iStart = 0;
    int iEnd = GetNElements();
    double BrL[5];
    for (int j = iStart; j < iEnd; j++) {
      int jLevel = CheckChainLevel(j);
      BrL[jLevel] = GetElementWt(j);
      for (int k = iStart; k < j; k++) {
	int kLevel = CheckChainLevel(k);
	if (CheckInChain(k, j)){
	  BrL[kLevel] = BrL[kLevel] * GetElementWt(k);
	}
      }
    
      double Br = 1;
      for(int l=0; l <= jLevel; l++) Br *= BrL[l];

      if (CheckInChain(iStart, j)) {
      
	double weight = 1.;
	double A_Source = Element[j]->GetTargetMass();
      
	if (A_Source > 0) {
	
	  double Lambda = log(2.) / GetLifetime(iStart);
	  double Lambda_d = log(2.) / GetLifetime(j);
	  double LTau = 1.;
	
	  if (Lambda_d > 1.e-12)
	    LTau = (1. - exp(-1. * Lambda_d)) / Lambda_d;
	
	  weight = Br * (N_A / A_Source) * Lambda * LTau;
	
	}

	if (GetEquilibrium()) weight = Br;
      
	SetElementProb(j, weight);
      
      }
    }
  }

  void DecayChain::SetElementProb(int iBranch, double prob)
  {
    if (iBranch <= GetNElements())
      ElementProb[iBranch] = prob;
  }

  double DecayChain::GetElementProb(int iBranch)
  {
    if (iBranch > GetNElements())
      return 0;
    return ElementProb[iBranch];
  }

  bool DecayChain::CheckInChain(int iElement, int jElement)
  {
    bool inChain = true;
    int index0 = 0;
    int index1 = 0;
    for (int l = 0; l < 3; l++) {
      int divisor = (int) pow(10., 1. * l);
      index0 = (GetElementBranch(iElement) / divisor) % 10;
      index1 = (GetElementBranch(jElement) / divisor) % 10;
      if (((index0 * index1) != 0) && (index1 != index0))
	inChain = false;
    }
    return inChain;
  }

  int DecayChain::CheckChainLevel(int iElement)
  {
    int level = 0;
    for (int l = 0; l < 3; l++) {
      int divisor = (int) pow(10., 1. * l);
      int index = (GetElementBranch(iElement) / divisor) % 10;
      if (index != 0) level = l;
    }
    return level;
  }

  void DecayChain::SetParticleInfo(int iTag, int jTag, int kTag){

    int pid = Element[iTag] -> GetEventID(jTag);

    double mass = 0.;
    if ((pid == DecayBeta) || (pid ==DecayEC)) mass = ElectronMass;
    if (pid == DecayAlpha) mass = AlphaMass;

    double energy = Element[iTag] -> GetEventEnergy(jTag);
    energy += mass;
    double pp  = sqrt(pow(energy,2) - pow(mass,2));
    double cz  = GetRandomNumber(-1.,1.);
    double phi = GetRandomNumber(0.,2.*M_PI);

    double px = pp * sqrt(1. - pow(cz,2)) * cos(phi);
    double py = pp * sqrt(1. - pow(cz,2)) * sin(phi);
    double pz = pp * cz;

    theParticleInfo[kTag].vector.setE(energy);
    theParticleInfo[kTag].vector.setPx(px);
    theParticleInfo[kTag].vector.setPy(py);
    theParticleInfo[kTag].vector.setPz(pz);
    theParticleInfo[kTag].ID = pid;

  }

  DecayChain::ParticleInfo_t DecayChain::AddDaughterInfo(int iTag){

    ParticleInfo_t theDaughterInfo;

    int iType = Element[iTag] -> GetDecayType();
    double A  = Element[iTag] -> GetTargetMass();
    double Z  = Element[iTag] -> GetCharge();

    /*double deltaA = 0.;
      double deltaZ = 0.;*/
    double massI  = 0.;
    if (iType == DecayBeta) {/*deltaZ = +1.;*/ massI = ElectronMass;}
    if (iType == DecayEC)   {/*deltaZ = -1.;*/ massI = ElectronMass;}
    if (iType == DecayAlpha){/*deltaA = -4.;deltaZ = -2.;*/ massI = AlphaMass;}
  
    double MassD = Nucl_Mass(A,Z);
    double Q  = 0.;
    for (int i = 0; i<Element[iTag]->GetNParticles(0); i++){ 
      Q += Element[iTag] -> GetEnergy(0,i);
    }

    double TRecoil = Q * (massI / MassD) + pow(Q,2.)/(2.*MassD);
    double ERecoil = TRecoil + MassD;  
    double PRecoil = sqrt(pow(ERecoil,2.) - pow(MassD,2.)); 
    double cz  = GetRandomNumber(-1.,1.);
    double phi = GetRandomNumber(0.,2.*M_PI);
    
    double px = PRecoil * sqrt(1. - pow(cz,2)) * cos(phi);
    double py = PRecoil * sqrt(1. - pow(cz,2)) * sin(phi);
    double pz = PRecoil * cz;

    int isoID = 100000000;
    isoID += (int) A * 1000;
    isoID += (int) Z;

    theDaughterInfo.vector.setE(ERecoil);
    theDaughterInfo.vector.setPx(px);
    theDaughterInfo.vector.setPy(py);
    theDaughterInfo.vector.setPz(pz);
    theDaughterInfo.ID = isoID;

    return theDaughterInfo;
  }

  DecayChain::ParticleInfo_t DecayChain::GetParticleInfo(int iEvent)
  {
    ParticleInfo_t blankInfo = {0, CLHEP::HepLorentzVector()};

    if (iEvent > GetNGenerated()) return blankInfo;
    if (!CheckParentMatch(iEvent)) return blankInfo;

    return theParticleInfo[iEvent];
  }

  double DecayChain::GetRandomNumber(double rmin, double rmax)
  {
    double rnd = G4UniformRand(); // random number from 0 to 1.
    double value = rmin + (rmax - rmin) * rnd;
    return value;
  }

} // namespace RAT
