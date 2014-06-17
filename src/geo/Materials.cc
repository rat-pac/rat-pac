#include <algorithm>
#include <functional>
#include <numeric>
#include <RAT/Materials.hh>
#include <G4NistManager.hh>

using namespace::std;

namespace RAT {

std::map<std::string, G4OpticalSurface*> Materials::optical_surface;

void Materials::LoadOpticalSurfaces() {
  // Duplicate of work in GLG4DetectorConstruction, but there is no
  // global store for optical surfaces, so we have to make one ourselves

  // Materials used below (assume LoadMaterials() already called)
  G4Material* _stainless = G4Material::GetMaterial("stainless_steel");
  G4Material* _stainless_316L = G4Material::GetMaterial("stainless_steel_316L");
  G4Material* _aluminum = G4Material::GetMaterial("aluminum");
  G4Material* _polyethylene = G4Material::GetMaterial("polyethylene");
  G4Material* _tyvek = G4Material::GetMaterial("tyvek");
  G4Material* _tank_liner = G4Material::GetMaterial("tank_liner");
  G4Material* _blackAcryl = G4Material::GetMaterial("acrylic_black");
  G4Material* _whiteAcryl = G4Material::GetMaterial("acrylic_white");
  G4Material* _polycast = G4Material::GetMaterial("acrylic_polycast");
  G4Material* _ptfe = G4Material::GetMaterial("ptfe");
  G4Material* _ptfe_fabric = G4Material::GetMaterial("ptfe_fabric");
  G4Material* _zno2 = G4Material::GetMaterial("ZnO2");
  G4Material* _quartz = G4Material::GetMaterial("quartz");
  G4Material* _glass = G4Material::GetMaterial("glass");
  G4Material* _sapphire = G4Material::GetMaterial("sapphire");
  
  // load ALL the photocathodes here
  // note that *each* photocathode will need to be loaded here
  G4OpticalSurface *Photocathode =  new G4OpticalSurface("photocathode");
  Photocathode->SetType(dielectric_metal); // ignored if RINDEX defined
  Photocathode->SetMaterialPropertiesTable(
           G4Material::GetMaterial("photocathode")
                ->GetMaterialPropertiesTable() );
  optical_surface[Photocathode->GetName()] = Photocathode;
  
  G4OpticalSurface *Photocathode_R5912HQE =  new G4OpticalSurface("photocathode_R5912_HQE");
  Photocathode_R5912HQE->SetType(dielectric_metal); // ignored if RINDEX defined
  Photocathode_R5912HQE->SetMaterialPropertiesTable(
           G4Material::GetMaterial("photocathode_R5912_HQE")
                ->GetMaterialPropertiesTable() );
  optical_surface[Photocathode_R5912HQE->GetName()] = Photocathode_R5912HQE;
  
  G4OpticalSurface *Photocathode_R11065 =  new G4OpticalSurface("photocathode_R11065");
  Photocathode_R11065->SetType(dielectric_metal); // ignored if RINDEX defined
  Photocathode_R11065->SetMaterialPropertiesTable(
           G4Material::GetMaterial("photocathode_R11065")
                ->GetMaterialPropertiesTable() );
  optical_surface[Photocathode_R11065->GetName()] = Photocathode_R11065;
  
  
  G4OpticalSurface *Photocathode_R1408 =  new G4OpticalSurface("photocathode_R1408");
  Photocathode_R1408->SetType(dielectric_metal); // ignored if RINDEX defined
  Photocathode_R1408->SetMaterialPropertiesTable(
           G4Material::GetMaterial("photocathode_R1408")
                ->GetMaterialPropertiesTable() );
  optical_surface[Photocathode_R1408->GetName()] = Photocathode_R1408;
  
  G4OpticalSurface *Photocathode_et9390b =  new G4OpticalSurface("photocathode_et9390b");
  Photocathode_et9390b->SetType(dielectric_metal); // ignored if RINDEX defined
  Photocathode_et9390b->SetMaterialPropertiesTable(
           G4Material::GetMaterial("photocathode_et9390b")
                ->GetMaterialPropertiesTable() );
  optical_surface[Photocathode_et9390b->GetName()] = Photocathode_et9390b;
  
  G4OpticalSurface *Stainless =  new G4OpticalSurface("stainless_steel");
  Stainless->SetFinish(ground); 
  Stainless->SetModel(glisur); 
  Stainless->SetType(dielectric_metal); 
  Stainless->SetPolish(0.1);              // a guess -- FIXME?
  Stainless->SetMaterialPropertiesTable(
           _stainless->GetMaterialPropertiesTable() );
  optical_surface[Stainless->GetName()] = Stainless;
  
  G4OpticalSurface *Stainless_316L =  new G4OpticalSurface("stainless_steel_316L");
  Stainless_316L->SetFinish(ground); 
  Stainless_316L->SetModel(glisur); 
  Stainless_316L->SetType(dielectric_metal); 
  Stainless_316L->SetPolish(0.1);              // a guess -- FIXME?
  Stainless_316L->SetMaterialPropertiesTable(
           _stainless_316L->GetMaterialPropertiesTable() );
  optical_surface[Stainless_316L->GetName()] = Stainless_316L;
  
  G4OpticalSurface *Aluminum =  new G4OpticalSurface("aluminum");
  Aluminum->SetFinish(ground); 
  Aluminum->SetModel(glisur); 
  Aluminum->SetType(dielectric_metal); 
  Aluminum->SetPolish(0.9);              // a guess -- FIXME?
  Aluminum->SetMaterialPropertiesTable(
           _aluminum->GetMaterialPropertiesTable() );
  optical_surface[Aluminum->GetName()] = Aluminum;

  G4OpticalSurface *Polyethylene =  new G4OpticalSurface("polyethylene");
  Polyethylene->SetFinish(ground); // a guess -- FIXME?
  Polyethylene->SetModel(glisur); // a guess -- FIXME?
  Polyethylene->SetType(dielectric_dielectric); // a guess -- FIXME?
  Polyethylene->SetPolish(0.7);              // a guess -- FIXME?
  Polyethylene->SetMaterialPropertiesTable(
     _polyethylene->GetMaterialPropertiesTable() );
  optical_surface[Polyethylene->GetName()] = Polyethylene;

  G4OpticalSurface *Tyvek =  new G4OpticalSurface("tyvek");
  Tyvek->SetFinish(ground); 
  Tyvek->SetModel(glisur); 
  Tyvek->SetType(dielectric_metal); 
  Tyvek->SetPolish(0.01);              // a guess -- FIXME
  Tyvek->SetMaterialPropertiesTable(
           _tyvek->GetMaterialPropertiesTable() );
  optical_surface[Tyvek->GetName()] = Tyvek;

  G4OpticalSurface *Tank_liner =  new G4OpticalSurface("tank_liner");
  Tank_liner->SetFinish(ground); 
  Tank_liner->SetModel(glisur); 
  Tank_liner->SetType(dielectric_metal); 
  Tank_liner->SetPolish(0.01);              // a guess -- FIXME
  Tank_liner->SetMaterialPropertiesTable(
           _tank_liner->GetMaterialPropertiesTable() );
  optical_surface[Tank_liner->GetName()] = Tank_liner;

  G4OpticalSurface *BlackSheet =  new G4OpticalSurface("black_sheet");
  BlackSheet->SetFinish(ground); 
  BlackSheet->SetModel(glisur); 
  BlackSheet->SetType(dielectric_metal); 
  BlackSheet->SetPolish(0.1);              // a guess -- FIXME
  BlackSheet->SetMaterialPropertiesTable(
           _blackAcryl->GetMaterialPropertiesTable() );
  optical_surface[BlackSheet->GetName()] = BlackSheet;

  G4OpticalSurface *WhiteAcrylic =  new G4OpticalSurface("acrylic_white");
  WhiteAcrylic->SetFinish(ground); 
  WhiteAcrylic->SetModel(glisur); 
  WhiteAcrylic->SetType(dielectric_metal); 
  WhiteAcrylic->SetPolish(0.01);              // a guess -- FIXME?
  WhiteAcrylic->SetMaterialPropertiesTable(
           _whiteAcryl->GetMaterialPropertiesTable() );
  optical_surface[WhiteAcrylic->GetName()] = WhiteAcrylic;
  
  G4OpticalSurface *PolishedPolycast =  new G4OpticalSurface("polished_polycast");
  PolishedPolycast->SetFinish(ground); // a guess -- FIXME?
  PolishedPolycast->SetModel(glisur); // a guess -- FIXME?
  PolishedPolycast->SetType(dielectric_dielectric); // a guess -- FIXME?
  PolishedPolycast->SetPolish(0.9);              // a guess -- FIXME?
  PolishedPolycast->SetMaterialPropertiesTable(
     _polycast->GetMaterialPropertiesTable() );
  optical_surface[PolishedPolycast->GetName()] = PolishedPolycast;
  
  G4OpticalSurface *ptfe =  new G4OpticalSurface("ptfe");
  ptfe->SetFinish(ground); 
  ptfe->SetModel(glisur); 
  ptfe->SetType(dielectric_metal); 
  ptfe->SetPolish(0.01);              // a guess -- FIXME?
  ptfe->SetMaterialPropertiesTable(
           _ptfe->GetMaterialPropertiesTable() );
  optical_surface[ptfe->GetName()] = ptfe;
  
  G4OpticalSurface *ptfe_fabric =  new G4OpticalSurface("ptfe_fabric");
  ptfe_fabric->SetFinish(ground); 
  ptfe_fabric->SetModel(glisur); 
  ptfe_fabric->SetType(dielectric_metal); 
  ptfe_fabric->SetPolish(0.01);              // a guess -- FIXME?
  ptfe_fabric->SetMaterialPropertiesTable(
           _ptfe_fabric->GetMaterialPropertiesTable() );
  optical_surface[ptfe_fabric->GetName()] = ptfe_fabric;
  
  G4OpticalSurface *zno2 =  new G4OpticalSurface("ZnO2");
  zno2->SetFinish(ground); 
  zno2->SetModel(glisur); 
  zno2->SetType(dielectric_metal); 
  zno2->SetPolish(0.01);              // a guess -- FIXME?
  zno2->SetMaterialPropertiesTable(
           _zno2->GetMaterialPropertiesTable() );
  optical_surface[zno2->GetName()] = zno2;
  
  G4OpticalSurface *quartz =  new G4OpticalSurface("quartz");
  quartz->SetFinish(polished); 
  quartz->SetModel(glisur); 
  quartz->SetType(dielectric_dielectric); 
  quartz->SetPolish(0.9);              // Using SetFinish(polished) is the same as SetPolish(1.0) for a dielectric_dielectric
  quartz->SetMaterialPropertiesTable(
           _quartz->GetMaterialPropertiesTable() );
  optical_surface[quartz->GetName()] = quartz;
  
  G4OpticalSurface *glass =  new G4OpticalSurface("glass");
  glass->SetFinish(ground); 
  glass->SetModel(glisur); 
  glass->SetType(dielectric_dielectric); 
  glass->SetPolish(0.9);              // a guess -- FIXME?
  glass->SetMaterialPropertiesTable(
           _glass->GetMaterialPropertiesTable() );
  optical_surface[glass->GetName()] = glass;
  
  G4OpticalSurface *sapphire =  new G4OpticalSurface("sapphire");
  sapphire->SetFinish(ground);
  sapphire->SetModel(glisur);
  sapphire->SetType(dielectric_dielectric);
  sapphire->SetPolish(0.9);              // a guess -- FIXME?
  sapphire->SetMaterialPropertiesTable(
                                       _sapphire->GetMaterialPropertiesTable() );
  optical_surface[sapphire->GetName()] = sapphire;

  G4OpticalSurface *mirror = new G4OpticalSurface("mirror");
  mirror->SetFinish(polishedfrontpainted); // needed for mirror
  mirror->SetModel(glisur); 
  mirror->SetType(dielectric_metal); 
  mirror->SetPolish(0.999);              // a guess -- FIXME
  G4MaterialPropertiesTable* propMirror=
    new G4MaterialPropertiesTable();
  propMirror->AddProperty("REFLECTIVITY", new G4MaterialPropertyVector());
  propMirror->AddEntry("REFLECTIVITY", twopi*hbarc/(800.0e-9*m), 0.9999);
  propMirror->AddEntry("REFLECTIVITY", twopi*hbarc/(60.0e-9*m), 0.9999);
  mirror->SetMaterialPropertiesTable( propMirror ); 
  optical_surface[mirror->GetName()] = mirror;

  G4OpticalSurface* styrofoam_surface=new G4OpticalSurface("styrofoam");
  styrofoam_surface->SetFinish(ground);
  // left default Model (glisur)
  styrofoam_surface->SetType(dielectric_metal);
  // styrofoam_surface->SetPolish();//rather than guessing, left default (=1.0)
  styrofoam_surface->SetMaterialPropertiesTable(
    G4Material::GetMaterial("styrofoam")->GetMaterialPropertiesTable());
  optical_surface[styrofoam_surface->GetName()]=styrofoam_surface;
}

// ----------------------------------------------------------------
void Materials::ConstructMaterials() {
  // = Elements =====================

  G4String name;
  G4String symbol;

  DB* db = DB::Get();

  DBLinkGroup elem = db->GetLinkGroup("ELEMENT");
  DBLinkGroup::iterator i_table;
  for (i_table=elem.begin(); i_table!=elem.end(); i_table++) {
    std::string namedb = i_table->first;
    DBLinkPtr table = i_table->second;
    double adb;
    int zdb = 0;
    string symbol;

    // Get common fields
    try {
      symbol= table->GetS("SYMBOL");
      zdb = table->GetI("z");
    }
    catch (DBNotFoundError &e) {
      G4cout << "Materials error: Could not construct elements" << G4endl;
    }

    // Check if this element has special isotope abundances
    try {
      const vector<int>& isotopes = table->GetIArray("isotopes");
      const vector<double>& isotopes_frac = table->GetDArray("isotopes_frac");

      G4Element* elem = new G4Element(namedb, symbol, isotopes.size());
      for (unsigned i_isotope=0; i_isotope<isotopes.size(); i_isotope++) {
        // Leave out last field of constructor so mass/mole of isotope
        // is pulled from NIST database.
        G4Isotope* iso = new G4Isotope(namedb, zdb, isotopes[i_isotope]);
        elem->AddIsotope(iso, isotopes_frac[i_isotope]);
      }
      // All done, skip next try block
      continue;
    }
    catch (DBNotFoundError &e) {
      // No isotope list, assume it is a normal element, next try
      // block will handle it
    }

    try {
      adb = table->GetD("a");
      new G4Element(namedb, symbol, zdb, adb*g/mole);
    }
    catch (DBNotFoundError &e) {
      G4cout << "Materials error: Could not construct elements" << G4endl;
    }
  }
  
  // === Material ===============================================

  vector<string> queue;

  DBLinkGroup mats = db->GetLinkGroup("MATERIAL");
  for (DBLinkGroup::iterator iv=mats.begin(); iv!=mats.end(); iv++) {
    std::string namedb = iv->first;
    G4cout << "Loaded material: " << namedb << G4endl;

    DBLinkPtr table = iv->second;
    if(!BuildMaterial(namedb, table)) {
      queue.push_back(namedb);
      G4cout << namedb << " thrown on construction queue" << G4endl;
    }
  }

  for (vector<string>::iterator i=queue.begin(); i!=queue.end(); i++) {
    std::string namedb = *i;
    DBLinkPtr table = DB::Get()->GetLink("MATERIAL", namedb);
    G4cout << "queue: " << namedb << G4endl;
    BuildMaterial(namedb,table);
  }

  G4Material::GetMaterial("stainless_steel");  
  G4Material::GetMaterial("aluminum");  
  G4Material::GetMaterial("acrylic_black");
  G4Material::GetMaterial("acrylic_white");
  G4Material::GetMaterial("acrylic_polycast");
  G4Material::GetMaterial("polyethylene");
  G4Material::GetMaterial("tyvek");
  G4Material::GetMaterial("ptfe");
  G4Material::GetMaterial("ptfe_fabric");
  G4Material::GetMaterial("ZnO2");
  G4Material::GetMaterial("quartz");
  G4Material::GetMaterial("glass");
  G4Material::GetMaterial("sapphire");

  // == Optics ==================================================

  LoadOptics();
}


bool Materials::BuildMaterial(string namedb, DBLinkPtr table) {
  G4MaterialPropertiesTable* MPT = NULL;

  double densitydb;
  int nelementsdb;
  int nmaterialsdb;
  G4State state = kStateUndefined;
  double temperature;
  double pressure;

  try {
    densitydb = table->GetD("density") * g / cm3;
    nelementsdb = table->GetI("nelements");
    nmaterialsdb = table->GetI("nmaterials");
  }
  catch (DBNotFoundError &e) {
    G4cout << "Materials: Material read error" << G4endl;
    return false;
  }
    
  try {
    string stringstate = table->GetS("state");
    if (stringstate == "gas")
      state = kStateGas;
    else if (stringstate == "solid")
      state = kStateSolid;
    else if (stringstate == "liquid")
      state = kStateLiquid;
  }
  catch (DBNotFoundError &e) {
    state = kStateUndefined;
  }
      
  try {
    temperature = table->GetD("temperature");
  }
  catch (DBNotFoundError &e) {
    temperature = STP_Temperature;
  }

  try {
    pressure = table->GetD("pressure") * STP_Pressure;
  }
  catch (DBNotFoundError &e) {
    pressure = STP_Pressure;
  }

  G4Material* tempptr = 
    new G4Material(namedb, densitydb, nelementsdb + nmaterialsdb,
                   state, temperature, pressure);

  string formula = "GOOD";
  try {
    formula = table->GetS("formula");
  }
  catch (DBNotFoundError &e) {
    formula = "BAD";
  }

  if (formula != "BAD"){
    tempptr->SetChemicalFormula(formula);
  }

  double mol = 0;
  formula = "GOOD";
  try {
    mol = table->GetD("mol");
  }
  catch (DBNotFoundError &e) {
    formula = "BAD";
  }

  if (formula != "BAD") {
    MPT = new G4MaterialPropertiesTable();
    MPT->AddConstProperty("MOL", mol/g);
    tempptr->SetMaterialPropertiesTable(MPT);
  }

  if (nelementsdb > 0) {
    vector<string> elemname = table->GetSArray("elements");
    vector<double> elemprop = table->GetDArray("elemprop");

    if (elemname.size() != elemprop.size()) {
      G4cout << "Death...oh Materials material reader, "
             << " how you have forsaken me" << G4endl; //fixme tk
      exit(-1);
    }

    for (vector<string>::size_type i=0; i<elemname.size(); i++) {
      tempptr->AddElement(G4Element::GetElement(elemname[i]), elemprop[i]);
    }
  }

  if (nmaterialsdb > 0) {
    vector<string> elemname = table->GetSArray("materials");
    vector<double> elemprop = table->GetDArray("matprop");

    if (elemname.size() != elemprop.size()) {
      G4cout << "Death...oh Materials material reader, "
             << "how you have forsaken me"<<G4endl; //fixme tk
      exit(-1);
    }

    for (vector<string>::size_type i=0; i<elemname.size(); i++) {
      std::string addmatname = elemname[i];
      G4Material* addmatptr = NULL;
      if (addmatname == "G4_Gd") {
        G4NistManager* man = G4NistManager::Instance();
        addmatptr = man->FindOrBuildMaterial("G4_Gd");
      }
      else{
        addmatptr = G4Material::GetMaterial(elemname[i]);
      }

      // If we encounter a material that hasn't been build,
      // add the material that contains it to the queue
      if (addmatptr != NULL) {
        tempptr->AddMaterial(addmatptr, elemprop[i]);
      }
      else {
        G4MaterialTable* tmp_table =
          (G4MaterialTable*) tempptr->GetMaterialTable();
        std::vector<G4Material*>::iterator it = tmp_table->begin() + tempptr->GetIndex(); 
        delete tempptr;
        tmp_table->erase(it); 
        return false; 
      }
    }
  }

  return true;
}


G4MaterialPropertyVector*
Materials::LoadProperty(DBLinkPtr table, std::string name) {
  int wavelength_opt = 0;
  try {
    std::string optstring = table->GetS(name + "_option");
    if (optstring == "wavelength")
      wavelength_opt = 1;
    else if (optstring == "dy_dwavelength")
      wavelength_opt = 2;
    else if(optstring == "energy")
      wavelength_opt = 0;
    else
      wavelength_opt = 0;
  }
  catch (DBNotFoundError &e) {
    wavelength_opt = 0;
  }

  vector<double> val1, val2;
  try {
    val1 = table->GetDArray(name + "_value1");
    val2 = table->GetDArray(name + "_value2");
  }
  catch (DBNotFoundError &e) {
    G4cout << "Could not read property " << name
           << " of " << table->GetIndex() << G4endl;
    return NULL;
  }

  if (val1.size() != val2.size()) {
    G4cout << "Array size error in Materials: "
           << "bad property value sizes" << G4endl;
    return NULL;
  }

  // Material properties have to be inserted in energy order, so
  // if this is a wavelength, we have to go backwards
  int direction;
  int start;
  if (wavelength_opt) {
    start = val1.size() - 1;
    direction = -1;
  }
  else {
    start = 0;
    direction = 1;
  }

  G4MaterialPropertyVector* pv = new G4MaterialPropertyVector();
  for (int i=start; i>=0 && i<(int)val1.size(); i+=direction) {
    double E_value= val1[i];
    double p_value= val2[i];
    if (wavelength_opt) {
      if (E_value != 0.0) {
        double lam = E_value;
        E_value = twopi * hbarc / (lam * nanometer);
        if (wavelength_opt == 2)
          p_value *= lam / E_value;
      }
      else {
        G4cerr << "Materials property vector zero wavelength!\n";
      }
    }
    pv->InsertValues(E_value, p_value);
  }

  return pv;
}


void
Materials::BuildMaterialPropertiesTable(G4Material* material, DBLinkPtr table) {
  // Bail if the MPT already exists
  if (material->GetMaterialPropertiesTable() != NULL) {
    return;
  }

  std::string name = std::string(material->GetName());
  // Determine which fields to load
  vector<std::string> props;
  try {
    props = table->GetSArray("PROPERTY_LIST");
  }
  catch (DBNotFoundError &e) {
    G4cout << "Materials failed to get property list on: " << name << G4endl;
    return;
  }

  G4MaterialPropertiesTable* mpt = new G4MaterialPropertiesTable();
  material->SetMaterialPropertiesTable(mpt);

  // Loop over DB fields containing material (optical) properties
  for (vector<std::string>::iterator i=props.begin(); i!=props.end(); i++) {
    // Handle const properties
    if (*i == "LIGHT_YIELD"           || *i == "dEdxCOEFF"            ||
        *i == "WLSTIMECONSTANT"       || *i == "WLSMEANNUMBERPHOTONS" ||
        *i == "FASTTIMECONSTANT"      || *i == "SLOWTIMECONSTANT"     ||
        *i == "YIELDRATIO"            || *i == "RESOLUTIONSCALE"      ||
        *i == "WLSFASTTIMECONSTANT"   || *i == "WLSSLOWTIMECONSTANT"  ||
        *i == "WLSYIELDRATIO"         || *i == "WLSRESOLUTIONSCALE"   ||
        *i == "WLSSCINTILLATIONYIELD" || *i == "SCINTILLATIONYIELD"   ||
        *i == "LAMBERTIAN_REFLECTION" || *i == "LAMBERTIAN_FORWARD"   ||
        *i == "LAMBERTIAN_BACKWARD"   || *i == "ELECTRICFIELD"        ||
        *i == "TOTALNUM_INT_SITES") {
      mpt->AddConstProperty(i->c_str(), table->GetD(*i));
      continue;
    }

    // Skip COMPONENTS specification
    if (*i == "COMPONENTS" || *i == "COMPONENT_FRACTIONS") {
      continue;
    }

    // Otherwise, this is a property vector
    G4MaterialPropertyVector* pv = mpt->GetProperty((*i).c_str());
    if (!pv) {
      pv = LoadProperty(table, *i);
    }

    if (pv) {
      mpt->AddProperty((*i).c_str(), pv);
    }
    else {
      continue;
    }
  }
}


void Materials::LoadOptics() {
  DBLinkGroup mats = DB::Get()->GetLinkGroup("OPTICS");

  // Load everything in OPTICS
  for (DBLinkGroup::iterator iv=mats.begin(); iv!=mats.end(); iv++) {
    std::string name = iv->first;
    G4cout << "Loading optics: " << name << G4endl;
    
    G4Material* material = G4Material::GetMaterial(name);
    if (material == NULL) {
      G4cout << "While loading optics in Materials, "
             << "there was a bad material name: " << name << G4endl;
      continue;
    }

    // Build MPT if necessary
    BuildMaterialPropertiesTable(material, iv->second);
  }

  // Handle optics for composite materials
  for (DBLinkGroup::iterator iv=mats.begin(); iv!=mats.end(); iv++) {
    std::string name = iv->first;
    DBLinkPtr table = iv->second;

    vector<std::string> props;
    try {
      props = table->GetSArray("PROPERTY_LIST");
    }
    catch (DBNotFoundError &e) {
      G4cout << "Materials failed to get property list on: " << name << G4endl;
      Log::Die("Materials unable to load property list");
      return;
    }

    if (std::find(props.begin(), props.end(), "COMPONENTS") == props.end()) {
      continue;
    }

    std::vector<std::string> components;
    std::vector<double> fractions;

    // Load and validate the table entries
    try {
      components = table->GetSArray("COMPONENTS");
      fractions = table->GetDArray("COMPONENT_FRACTIONS");
    }
    catch (DBNotFoundError &e) {
      Log::Die("Materials: Error loading COMPONENTS property.");
    }

    Log::Assert(components.size() == fractions.size(),
                "Materials: COMPONENTS and COMPONENT_FRACTIONS length mismatch");
    double total_fractions =
      std::accumulate(fractions.begin(), fractions.end(), 0.0);
    Log::Assert(total_fractions < 1.01 && total_fractions > 0.99,
                "Materials: Component fractions must sum to 1");

    G4Material* material = G4Material::GetMaterial(name);
    Log::Assert(material, "Materials: Unable to locate primary material");
    G4MaterialPropertiesTable* mpt = material->GetMaterialPropertiesTable();

    // Loop over components, adding their properties to the current material's
    // properties table. They are numbered by their index so that we can
    // e.g. absorb on X and reemit with the right spectrum, except for
    // scattering, which is combined into a single scattering length
    mpt->AddConstProperty("NCOMPONENTS", components.size());
    for (size_t i=0; i<components.size(); i++) {
      std::string compname = components[i];
      double fraction = fractions[i];
      std::stringstream ss;
      ss << "FRACTION" << i + 1;
      mpt->AddConstProperty(ss.str().c_str(), fraction);

      G4cout << " - Component " << i << ": " << compname
             << " (" << 100.0 * fraction << "%)" << G4endl;

      G4Material* component = G4Material::GetMaterial(compname);
      Log::Assert(material, "Materials: Unable to locate component material");
      const std::map<G4String,
                     G4MaterialPropertyVector*>* mpm = mpt->GetPropertiesMap();
      const std::map<G4String,
                     G4double>* mpmc = mpt->GetPropertiesCMap();

      G4MaterialPropertiesTable* cpt = component->GetMaterialPropertiesTable();

      // Copy component vector properties to material
      G4double* absorption_coeff_x = NULL;
      G4double* absorption_coeff_y = NULL;
      G4double* rayleigh_coeff_x = NULL;
      G4double* rayleigh_coeff_y = NULL;

      const std::map<G4String,
                     G4MaterialPropertyVector*>* cpm = cpt->GetPropertiesMap();
      std::map<G4String,
               G4MaterialPropertyVector*>::const_iterator it;
      for (it=cpm->begin(); it!=cpm->end(); it++) {
        std::string name = it->first;
        if (name.find("REEMITWAVEFORM") != std::string::npos    ||
            name.find("SCINTILLATION_WLS") != std::string::npos ||
            name.find("ABSLENGTH") != std::string::npos) {
          Log::Assert(mpm->find(name) == mpm->end(),
                      "Materials: Composite material cannot contain the same properties as components");
          G4cout << compname << " has " << name << "!" << G4endl;
          std::stringstream ss;
          ss << name << i + 1;
          mpt->AddProperty(ss.str().c_str(), it->second);

          // Also compute total absorption length
          if (name.find("ABSLENGTH") != std::string::npos) {
            if (!absorption_coeff_x) {
              G4MaterialPropertyVector* tempv = it->second;
              size_t entries = tempv->GetVectorLength();
              absorption_coeff_x = new G4double[entries];
              absorption_coeff_y = new G4double[entries];
              for (size_t ientry=0; ientry<entries; ientry++) {
                absorption_coeff_x[ientry] = tempv->Energy(ientry);
                absorption_coeff_y[ientry] = fraction / tempv->Value(ientry);
              }
            }
          }

          size_t entries = it->second->GetVectorLength();
          for (size_t ientry=0; ientry<entries; ientry++) {
            G4double ahere = it->second->Value(absorption_coeff_x[ientry]);
            absorption_coeff_y[ientry] += fraction / ahere;
          }
        }

        // The scattering lengths are combined
        else if (name.find("RSLENGTH") != std::string::npos) {
          Log::Assert(mpm->find(name) == mpm->end(),
                      "Materials: Composite material cannot contain the same properties as components");
          if (!rayleigh_coeff_x) {
            G4MaterialPropertyVector* tempv = it->second;
            G4int entries = tempv->GetVectorLength();
            rayleigh_coeff_x = new G4double[entries];
            rayleigh_coeff_y = new G4double[entries];
            for (G4int ientry=0; ientry<entries; ientry++) {
              rayleigh_coeff_x[ientry] = tempv->Energy(ientry);
              rayleigh_coeff_y[ientry] = fraction / tempv->Value(ientry);
            }
          }

          size_t entries = it->second->GetVectorLength();
          for (size_t ientry=0; ientry<entries; ientry++) {
            G4double ahere = it->second->Value(rayleigh_coeff_x[ientry]);
            rayleigh_coeff_y[ientry] += fraction / ahere;
          }
        }
      }

      // Copy component constant properties to material
      const std::map<G4String,
                     G4double>* cpmc = cpt->GetPropertiesCMap();
      std::map<G4String,
               G4double>::const_iterator itc;
      for (itc=cpmc->begin(); itc!=cpmc->end(); itc++) {
        std::string name = itc->first;
        if (name.find("REEMISSION_PROB") != std::string::npos) {
          Log::Assert(mpmc->find(name) == mpmc->end(),
                      "Materials: Composite material cannot contain the same properties as components");
          G4cout << compname << " has " << name << G4endl;
          std::stringstream ss;
          ss << name << i + 1;
          mpt->AddConstProperty(ss.str().c_str(), itc->second);
        }
      }
    }

    G4cout << "----" << G4endl;
    const std::map<G4String,
                   G4MaterialPropertyVector*>* mpm = mpt->GetPropertiesMap();
    std::map<G4String,
             G4MaterialPropertyVector*>::const_iterator it;
    for (it=mpm->begin(); it!=mpm->end(); it++) {
      G4cout << it->first << G4endl;
    }
    const std::map<G4String,
                   G4double>* mcpm = mpt->GetPropertiesCMap();
    std::map<G4String,
             G4double>::const_iterator cit;
    for (cit=mcpm->begin(); cit!=mcpm->end(); cit++) {
      G4cout << cit->first << G4endl;
    }
    G4cout << "----" << G4endl;
  }
}

} // namespace RAT

