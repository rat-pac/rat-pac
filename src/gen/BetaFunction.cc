#include <RAT/BetaFunction.hh>
#include <RAT/FermiFunction.hh>

#include <Randomize.hh>

#include <cstring>
#include <iostream>

namespace RAT {

  BetaFunction::BetaFunction()
  {
    Reset();
  }

  BetaFunction::BetaFunction(const std::string Name)
  {
    Reset();
    SetName(Name);
  }

  BetaFunction::BetaFunction(const std::string Name, double A, double Z, int reac,
			     double tau)
  {
    Reset();
    SetTarget(Name, A, Z, reac, tau);
  }

  BetaFunction::~BetaFunction()
  {
    Reset();
  }


  void BetaFunction::Reset()
  {
    SetName("\0");
    SetCharge(0.);
    SetTargetMass(0.);
    SetDecayType(DecayBeta);
    SetNeutrinoMass(0.);
    SetLifeTime(0.);
  
    SetVerbose(false);
    SetProbabilityCulmulative(true);
    NumberOfBranches = 0;

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

  void BetaFunction::Show()
  {
    printf("Target Name       : %s \n", GetName().c_str());
    printf("Target Mass       : %f \n", GetTargetMass());
    printf("Target Charge     : %f \n", GetCharge());
    printf("Decay  Type       : %d \n", GetDecayType());
    printf("Target Lifetime   : %f \n", GetLifeTime());
    printf("Neutrino Mass     : %f \n", GetNeutrinoMass());
    printf("Number of Branches: %d \n", GetNBranches());
    printf("Branch Prob. \t Spin \t Endpoint \t NGammas \t Energies \n");
 
    int nBr = GetNBranches();
    if (nBr > 0) {
    
      for (int i = 0; i < nBr; i++) {
	printf("%f \t", GetBranch(i));
	printf("%d \t", GetSpin(i));
	printf("%f \t", GetEndPoint(i));
	int nP = GetNParticles(i) - 1;
	printf("%d \t", nP);
	if (nP > 0) {
	  for (int j = 1; j <= nP; j++) {
	    int id = GetParticleID(i, j);
	    if (id == DecayGamma) {
	      printf("%f \t", GetEnergy(i, j));
	    }
	  }
	  printf(" \n");
	} else {
	  printf(" \n");
	}
      }
      printf(" \n");
    }
  }

  void BetaFunction::SetTarget(const std::string Name, double Z, double A, int reaction,
			       double tau)
  {
    SetName(Name);
    SetTargetMass(A);
    SetCharge(Z);
    SetDecayType(reaction);
    SetNeutrinoMass(0.);
    SetLifeTime(tau);
  }

  void BetaFunction::SetBranches(double Branch, int Spin, double EndPoint)
  {
    int iZ = (int) GetCharge();
    if (iZ == 0) {
      printf("No charge has been set. \n");
      return;
    }
  
    int nBr = GetNBranches();
  
    ParticleBranch[nBr] = Branch;
    ParticleSpin[nBr] = Spin;
    ParticleEndPoint[nBr] = EndPoint;
    ParticleEnergy[nBr][0] = EndPoint;
  
    NumberOfParticles[nBr] = 1;
  
    if (GetDecayType() != DecayAlpha) {
      if (iZ > 0)
	ParticleID[nBr][0] = DecayBeta;
      if (iZ < 0)
	ParticleID[nBr][0] = DecayEC;
    } else {
      ParticleID[nBr][0] = DecayAlpha;
    }
    SetNorm(nBr);
    NumberOfBranches++;
  }

  void BetaFunction::RemoveBranch(int iBranch)
  {
    ParticleBranch.erase(iBranch);
    ParticleSpin.erase(iBranch);
    ParticleEndPoint.erase(iBranch);
    ParticleID.erase(iBranch);
    ParticleEnergy.erase(iBranch);
    NumberOfParticles.erase(iBranch);
    DecayNorm.erase(iBranch);
    NumberOfBranches--;
  }

  void BetaFunction::SetGammas(int iBranch, int pid, double energy)
  {
    int iParticle = GetNParticles(iBranch);
    ParticleID[iBranch][iParticle] = pid;
    ParticleEnergy[iBranch][iParticle] = energy;
    NumberOfParticles[iBranch]++;
  }

  void BetaFunction::SetGammas(double energy)
  {
    int iBranch = GetNBranches() - 1;
    int iParticle = GetNParticles(iBranch);
  
    ParticleID[iBranch][iParticle] = DecayGamma;
    ParticleEnergy[iBranch][iParticle] = energy;
    NumberOfParticles[iBranch]++;
  }

  void BetaFunction::SetParticleID(int iBranch, int n, int id)
  {
    if ((iBranch > NumberOfBranches) || (iBranch < 0)) {
      ErrorLog(2);
      return;
    }
    ParticleID[iBranch][n] = id;
  }

  void BetaFunction::SetTargetMass(double A)
  {
  
    TargetMass = 0.;
    double Z = GetCharge();
    if ((A < 1) && (Z > 0)) {
      TargetMass =
	1.82 + 1.90 * Z + 0.01271 * pow(Z, 2) - 0.00006 * pow(Z, 3);
    } else {
      TargetMass = A;
    }
  }

  void BetaFunction::SetNorm(int iBranch)
  {
  
    double Norm = 0.;
    if (GetDecayType() == DecayAlpha) {
      Norm = 1.;
    } else {
      double eMin = ElectronMass;
      double eMax = eMin + GetEndPoint(iBranch);
      double dE = (eMax - eMin) / 1000.;
      for (double energy = eMin; energy < eMax; energy = energy + dE) {
	double F = GetValue(energy, iBranch);
	if (F > Norm) Norm = F;
      }
    }
    if (Norm <= 0.) Norm = 1.;
    DecayNorm[iBranch] = Norm * 1.11;
  }

  int BetaFunction::GetNParticles(int iBranch)
  {
    if ((iBranch > NumberOfBranches) || (iBranch < 0)) {
      ErrorLog(1);
      return iBAD;
    }
    return NumberOfParticles[iBranch];
  }

  int BetaFunction::GetParticleID(int iBranch, int n)
  {
    int id;
    if ((iBranch > NumberOfBranches) || (iBranch < 0)) {
      ErrorLog(2);
      return iBAD;
    }
    if (n > NumberOfParticles[iBranch]) {
      id = NullParticle;
    } else {
      id = ParticleID[iBranch][n];
    }
    return id;
  }

  int BetaFunction::GetSpin(int iBranch)
  {
    if ((iBranch > NumberOfBranches) || (iBranch < 0)) {
      ErrorLog(0);
      return iBAD;
    }
    return ParticleSpin[iBranch];
  }

  double BetaFunction::GetBranch(int iBranch)
  {
    if ((iBranch > NumberOfBranches) || (iBranch < 0)) {
      ErrorLog(0);
      return iBAD;
    }
    return ParticleBranch[iBranch];
  }

  double BetaFunction::GetEndPoint(int iBranch)
  {
    if ((iBranch > NumberOfBranches) || (iBranch < 0)) {
      ErrorLog(0);
      return iBAD;
    }
    return ParticleEndPoint[iBranch];
  }

  double BetaFunction::GetEnergy(int iBranch, int n)
  {
    double energy = 0.;
    if ((iBranch > NumberOfBranches) || (iBranch < 0)) {
      ErrorLog(2);
      return iBAD;
    }
  
    if (n > NumberOfParticles[iBranch]) {
      return energy;
    } else {
      energy = ParticleEnergy[iBranch][n];
    }
  
    return energy;
  }

  double BetaFunction::GetValue(double energy, int iBranch)
  {
    double Value = 0.;
    int iBeta = GetParticleID(iBranch, 0);
  
    if ((iBeta != DecayEC) && (iBeta != DecayBeta))
      return Value;
  
    int iSpin = GetSpin(iBranch);
    double Z = GetCharge();
    double A = GetTargetMass();
    double nu = GetNeutrinoMass();
    double ePoint = GetEndPoint(iBranch);
    double W0 = 1. + ePoint / ElectronMass;
    double W = energy / ElectronMass;
    Value = Nucl_Beta(iBeta, Z, A, W, W0, iSpin, nu);
    return Value;
  }

  void BetaFunction::GenerateEvent()
  {
    for (int i = 0; i < PartMax; i++) {
      En[i] = 0.;
      IDn[i] = 0;
    }
  
    int iBr = GetNBranches();
    int itag = 0;
    double prob = 0.;
    double r = GetRandomNumber();
  
    for (int i = 0; i < iBr; i++) {
      if (isCulmulative) {
	prob = GetBranch(i);
      } else {
	prob = prob + GetBranch(i);
      }
      if (r < prob) {
	itag = i;
	break;
      }
    }
  
    int nP = GetNParticles(itag);
    double W = 0.;
    for (int n = 0; n <= nP; n++) {
      int id = GetParticleID(itag, n);
      if ((id == DecayBeta) || (id == DecayEC)) {
	bool ithrow = true;
	while (ithrow) {
	  double R = GetNorm(itag) * GetRandomNumber();
	  double energy =
	    ElectronMass + GetEndPoint(itag) * GetRandomNumber();
	  double F = GetValue(energy, itag);
	  W = energy / ElectronMass;
	  if (R < F)
	    ithrow = false;
	}
	IDn[n] = id;
	En[n] = ElectronMass * W - ElectronMass;
      } else {
	IDn[n] = id;
	En[n] = GetEnergy(itag, n);
      }
    }
    SetEventTime();
    nGenerated = nP;
  }

  void BetaFunction::SetEventTime()
  {
    double tau = GetLifeTime();
    double r = GetRandomNumber();
    nTime = -tau * log(r) / log(2.);
  }

  int BetaFunction::GetEventID(int n)
  {
    if (n > nGenerated) {
      ErrorLog(3);
      return iBAD;
    }
    return IDn[n];
  }

  double BetaFunction::GetEventEnergy(int n)
  {
    if (n > nGenerated) {
      ErrorLog(3);
      return iBAD;
    }
    return En[n];
  }

  double BetaFunction::GetEventTotE()
  {
  
    int nG = GetNGenerated();
    double eTot = 0.;
  
    for (int i = 0; i < nG; i++) {
      eTot = eTot + GetEventEnergy(i);
    }
    return eTot;
  }

  double BetaFunction::GetRandomNumber(double rmin, double rmax)
  {
    double rnd = G4UniformRand(); // random number from 0 to 1.
    double value = rmin + (rmax - rmin) * rnd;
    return value;
  }

  bool BetaFunction::ReadInputFile(const std::string dName)
  {
    bool iFound = ReadInputFile(dName, -1, -1);
    return iFound;
  }


  bool BetaFunction::ReadInputFile(const std::string dName, int iType)
  {
    bool iFound = ReadInputFile(dName, -1, -1, iType);
    return iFound;
  }


  bool BetaFunction::ReadInputFile(const std::string dName, int iZ, int iA, int iType)
  {
  
    const int nReadGamma = 6;
  
    std::string eProbe = "END";
    std::string aProbe = "ALPHA:";
    std::string bProbe = "DECAY:";
    char dummy[80];
    std::string dProbe;
    char tName[80];
  
    if (iType != DecayAlpha) {
      dProbe = bProbe;
    } else {
      dProbe = aProbe;
    }
  
    bool iFound = false;
  
    int iSpin, nP;
    int Z, A;
    float tau;
    float iBr, W0, eP[nReadGamma], aC[3];
  
    SetProbabilityCulmulative(true);
  
    FILE* inputFile = fopen(GetFileName().c_str(), "r");
  
    if (!inputFile) {
      printf("Error opening file : %s \n", GetFileName().c_str());
      return iFound;
    }
  
    bool iRead = true;
  
    while ((iRead) && (!iFound)) {
      iRead = (fscanf(inputFile, "%s", dummy) > 0);
      std::string iString(dummy);
      if (iString == dProbe) {
	fscanf(inputFile, "%s", tName);
	fscanf(inputFile, "%d %d %f", &Z, &A, &tau);
	std::string iString2(tName);
	iFound = (dName == iString2);
	iFound = ((iFound) || ((iA == A) && (iZ == Z)));
	if (iFound) {
	  if (isVerbose)
	    printf("Reading %s \n \n", tName);
	  SetTargetMass((double) A);
	  SetCharge((double) Z);
	  SetLifeTime((double) tau);
	  SetDecayType(iType);
	
	  bool iScan = true;
	  while (iScan) {
	    fscanf(inputFile, "%f %d %f %d", &iBr, &iSpin, &W0, &nP);
	    SetBranches((double) iBr, iSpin, (double) W0);
	    for (int j = 0; j < nReadGamma; j++) {
	      fscanf(inputFile, "%f", &eP[j]);
	      if (eP[j] > 0.)
		SetGammas((double) eP[j]);
	    }
	    fscanf(inputFile, "%f %f %f", &aC[0], &aC[1], &aC[2]);
	    if (iBr >= 1.)
	      iScan = false;
	  }
	} else if (iString == eProbe) {
	  iRead = false;
	} else {
	}
      }
    }
  
    fclose(inputFile);
  
    if (iFound) {
      if (isVerbose)
	Show();
    } else {
      printf("No such element found: %s .\n", dName.c_str());
    }
    return iFound;
  }

  void BetaFunction::ErrorLog(int iFlag)
  {
    printf("Error detected: %d \n", iFlag);
  }

} // namespace RAT
