//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Jan 24 11:04:23 2014 by ROOT version 5.34/10
// from TTree treeout/Tree of events
// found on file: eventsout_nuwro_1_24_2014.root
//////////////////////////////////////////////////////////

#ifndef tree_h
#define tree_h

#include <string>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include <TObject.h>

// Fixed size dimensions of array or collections stored in the TTree if any.
const Int_t kMaxin = 2;
const Int_t kMaxtemp = 2;
const Int_t kMaxout = 3;
const Int_t kMaxpost = 8;
const Int_t kMaxall = 6;

using namespace std;

class tree {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
 //event           *e;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Bool_t          flag_qel;
   Bool_t          flag_res;
   Bool_t          flag_dis;
   Bool_t          flag_coh;
   Bool_t          flag_mec;
   Bool_t          flag_nc;
   Bool_t          flag_cc;
   Bool_t          flag_anty;
   Int_t           par_random_seed;
   Int_t           par_number_of_events;
   Int_t           par_number_of_test_events;
   Int_t           par_save_test_events;
   Int_t           par_user_events;
   string          par_user_params_string;
   Int_t           par_beam_type;
   string          par_beam_energy_string;
   Int_t           par_beam_particle;
   Double_t        par_beam_direction_x;
   Double_t        par_beam_direction_y;
   Double_t        par_beam_direction_z;
   string          par_beam_content_string;
   string          par_beam_folder;
   Int_t           par_beam_file_first;
   Int_t           par_beam_file_limit;
   Bool_t          par_beam_weighted;
   Double_t        par_beam_offset_x;
   Double_t        par_beam_offset_y;
   Double_t        par_beam_offset_z;
   Int_t           par_beam_placement;
   Int_t           par_beam_test_only;
   Int_t           par_target_type;
   Int_t           par_nucleus_p;
   Int_t           par_nucleus_n;
   Double_t        par_nucleus_E_b;
   Double_t        par_nucleus_kf;
   string          par_target_content_string;
   string          par_geo_file;
   string          par_geo_name;
   string          par_geo_volume;
   Double_t        par_geo_o_x;
   Double_t        par_geo_o_y;
   Double_t        par_geo_o_z;
   Double_t        par_geo_d_x;
   Double_t        par_geo_d_y;
   Double_t        par_geo_d_z;
   Int_t           par_nucleus_target;
   Int_t           par_nucleus_model;
   Bool_t          par_dyn_qel_cc;
   Bool_t          par_dyn_qel_nc;
   Bool_t          par_dyn_res_cc;
   Bool_t          par_dyn_res_nc;
   Bool_t          par_dyn_dis_cc;
   Bool_t          par_dyn_dis_nc;
   Bool_t          par_dyn_coh_cc;
   Bool_t          par_dyn_coh_nc;
   Bool_t          par_dyn_mec_cc;
   Bool_t          par_dyn_mec_nc;
   Int_t           par_qel_vector_ff_set;
   Int_t           par_qel_axial_ff_set;
   Int_t           par_qel_rpa;
   Int_t           par_qel_strange;
   Int_t           par_qel_strangeEM;
   Double_t        par_delta_s;
   Double_t        par_qel_cc_axial_mass;
   Double_t        par_qel_nc_axial_mass;
   Double_t        par_qel_s_axial_mass;
   Bool_t          par_flux_correction;
   Int_t           par_sf_method;
   Int_t           par_sf_pb;
   Bool_t          par_cc_smoothing;
   Int_t           par_delta_FF_set;
   Double_t        par_pion_axial_mass;
   Double_t        par_pion_C5A;
   Int_t           par_spp_precision;
   Double_t        par_res_dis_cut;
   Bool_t          par_coh_mass_correction;
   Bool_t          par_coh_new;
   Int_t           par_mec_kind;
   Double_t        par_mec_ratio_pp;
   Double_t        par_mec_ratio_ppp;
   Bool_t          par_kaskada_on;
   Double_t        par_kaskada_w;
   Bool_t          par_kaskada_redo;
   Bool_t          par_kaskada_writeall;
   string          par_formation_zone;
   Double_t        par_tau;
   Double_t        par_formation_length;
   Bool_t          par_first_step;
   Double_t        par_step;
   Int_t           par_xsec;
   Bool_t          par_pauli_blocking;
   Bool_t          par_mixed_order;
   string          par_path_to_data;
   Int_t           in_;
   Double_t        in_t[kMaxin];   //[in_]
   Double_t        in_x[kMaxin];   //[in_]
   Double_t        in_y[kMaxin];   //[in_]
   Double_t        in_z[kMaxin];   //[in_]
   Double_t        in__mass[kMaxin];   //[in_]
   Double_t        in_r_t[kMaxin];   //[in_]
   Double_t        in_r_x[kMaxin];   //[in_]
   Double_t        in_r_y[kMaxin];   //[in_]
   Double_t        in_r_z[kMaxin];   //[in_]
   Int_t           in_pdg[kMaxin];   //[in_]
   Char_t          in_ks[kMaxin];   //[in_]
   Char_t          in_orgin[kMaxin];   //[in_]
   Double_t        in_travelled[kMaxin];   //[in_]
   Int_t           in_id[kMaxin];   //[in_]
   Int_t           in_mother[kMaxin];   //[in_]
   Int_t           in_endproc[kMaxin];   //[in_]
   Double_t        in_his_fermi[kMaxin];   //[in_]
   Bool_t          in_primary[kMaxin];   //[in_]
   Int_t           temp_;
   Double_t        temp_t[kMaxtemp];   //[temp_]
   Double_t        temp_x[kMaxtemp];   //[temp_]
   Double_t        temp_y[kMaxtemp];   //[temp_]
   Double_t        temp_z[kMaxtemp];   //[temp_]
   Double_t        temp__mass[kMaxtemp];   //[temp_]
   Double_t        temp_r_t[kMaxtemp];   //[temp_]
   Double_t        temp_r_x[kMaxtemp];   //[temp_]
   Double_t        temp_r_y[kMaxtemp];   //[temp_]
   Double_t        temp_r_z[kMaxtemp];   //[temp_]
   Int_t           temp_pdg[kMaxtemp];   //[temp_]
   Char_t          temp_ks[kMaxtemp];   //[temp_]
   Char_t          temp_orgin[kMaxtemp];   //[temp_]
   Double_t        temp_travelled[kMaxtemp];   //[temp_]
   Int_t           temp_id[kMaxtemp];   //[temp_]
   Int_t           temp_mother[kMaxtemp];   //[temp_]
   Int_t           temp_endproc[kMaxtemp];   //[temp_]
   Double_t        temp_his_fermi[kMaxtemp];   //[temp_]
   Bool_t          temp_primary[kMaxtemp];   //[temp_]
   Int_t           out_;
   Double_t        out_t[kMaxout];   //[out_]
   Double_t        out_x[kMaxout];   //[out_]
   Double_t        out_y[kMaxout];   //[out_]
   Double_t        out_z[kMaxout];   //[out_]
   Double_t        out__mass[kMaxout];   //[out_]
   Double_t        out_r_t[kMaxout];   //[out_]
   Double_t        out_r_x[kMaxout];   //[out_]
   Double_t        out_r_y[kMaxout];   //[out_]
   Double_t        out_r_z[kMaxout];   //[out_]
   Int_t           out_pdg[kMaxout];   //[out_]
   Char_t          out_ks[kMaxout];   //[out_]
   Char_t          out_orgin[kMaxout];   //[out_]
   Double_t        out_travelled[kMaxout];   //[out_]
   Int_t           out_id[kMaxout];   //[out_]
   Int_t           out_mother[kMaxout];   //[out_]
   Int_t           out_endproc[kMaxout];   //[out_]
   Double_t        out_his_fermi[kMaxout];   //[out_]
   Bool_t          out_primary[kMaxout];   //[out_]
   Int_t           post_;
   Double_t        post_t[kMaxpost];   //[post_]
   Double_t        post_x[kMaxpost];   //[post_]
   Double_t        post_y[kMaxpost];   //[post_]
   Double_t        post_z[kMaxpost];   //[post_]
   Double_t        post__mass[kMaxpost];   //[post_]
   Double_t        post_r_t[kMaxpost];   //[post_]
   Double_t        post_r_x[kMaxpost];   //[post_]
   Double_t        post_r_y[kMaxpost];   //[post_]
   Double_t        post_r_z[kMaxpost];   //[post_]
   Int_t           post_pdg[kMaxpost];   //[post_]
   Char_t          post_ks[kMaxpost];   //[post_]
   Char_t          post_orgin[kMaxpost];   //[post_]
   Double_t        post_travelled[kMaxpost];   //[post_]
   Int_t           post_id[kMaxpost];   //[post_]
   Int_t           post_mother[kMaxpost];   //[post_]
   Int_t           post_endproc[kMaxpost];   //[post_]
   Double_t        post_his_fermi[kMaxpost];   //[post_]
   Bool_t          post_primary[kMaxpost];   //[post_]
   Int_t           all_;
   Double_t        all_t[kMaxall];   //[all_]
   Double_t        all_x[kMaxall];   //[all_]
   Double_t        all_y[kMaxall];   //[all_]
   Double_t        all_z[kMaxall];   //[all_]
   Double_t        all__mass[kMaxall];   //[all_]
   Double_t        all_r_t[kMaxall];   //[all_]
   Double_t        all_r_x[kMaxall];   //[all_]
   Double_t        all_r_y[kMaxall];   //[all_]
   Double_t        all_r_z[kMaxall];   //[all_]
   Int_t           all_pdg[kMaxall];   //[all_]
   Char_t          all_ks[kMaxall];   //[all_]
   Char_t          all_orgin[kMaxall];   //[all_]
   Double_t        all_travelled[kMaxall];   //[all_]
   Int_t           all_id[kMaxall];   //[all_]
   Int_t           all_mother[kMaxall];   //[all_]
   Int_t           all_endproc[kMaxall];   //[all_]
   Double_t        all_his_fermi[kMaxall];   //[all_]
   Bool_t          all_primary[kMaxall];   //[all_]
   Double_t        weight;
   Double_t        norm;
   Double_t        r_x;
   Double_t        r_y;
   Double_t        r_z;
   Double_t        density;
   Int_t           dyn;
   Int_t           nod[12];
   Int_t           pr;
   Int_t           nr;

   // List of branches
   TBranch        *b_e_fUniqueID;   //!
   TBranch        *b_e_fBits;   //!
   TBranch        *b_e_flag_qel;   //!
   TBranch        *b_e_flag_res;   //!
   TBranch        *b_e_flag_dis;   //!
   TBranch        *b_e_flag_coh;   //!
   TBranch        *b_e_flag_mec;   //!
   TBranch        *b_e_flag_nc;   //!
   TBranch        *b_e_flag_cc;   //!
   TBranch        *b_e_flag_anty;   //!
   TBranch        *b_e_par_random_seed;   //!
   TBranch        *b_e_par_number_of_events;   //!
   TBranch        *b_e_par_number_of_test_events;   //!
   TBranch        *b_e_par_save_test_events;   //!
   TBranch        *b_e_par_user_events;   //!
   TBranch        *b_e_par_user_params_string;   //!
   TBranch        *b_e_par_beam_type;   //!
   TBranch        *b_e_par_beam_energy_string;   //!
   TBranch        *b_e_par_beam_particle;   //!
   TBranch        *b_e_par_beam_direction_x;   //!
   TBranch        *b_e_par_beam_direction_y;   //!
   TBranch        *b_e_par_beam_direction_z;   //!
   TBranch        *b_e_par_beam_content_string;   //!
   TBranch        *b_e_par_beam_folder;   //!
   TBranch        *b_e_par_beam_file_first;   //!
   TBranch        *b_e_par_beam_file_limit;   //!
   TBranch        *b_e_par_beam_weighted;   //!
   TBranch        *b_e_par_beam_offset_x;   //!
   TBranch        *b_e_par_beam_offset_y;   //!
   TBranch        *b_e_par_beam_offset_z;   //!
   TBranch        *b_e_par_beam_placement;   //!
   TBranch        *b_e_par_beam_test_only;   //!
   TBranch        *b_e_par_target_type;   //!
   TBranch        *b_e_par_nucleus_p;   //!
   TBranch        *b_e_par_nucleus_n;   //!
   TBranch        *b_e_par_nucleus_E_b;   //!
   TBranch        *b_e_par_nucleus_kf;   //!
   TBranch        *b_e_par_target_content_string;   //!
   TBranch        *b_e_par_geo_file;   //!
   TBranch        *b_e_par_geo_name;   //!
   TBranch        *b_e_par_geo_volume;   //!
   TBranch        *b_e_par_geo_o_x;   //!
   TBranch        *b_e_par_geo_o_y;   //!
   TBranch        *b_e_par_geo_o_z;   //!
   TBranch        *b_e_par_geo_d_x;   //!
   TBranch        *b_e_par_geo_d_y;   //!
   TBranch        *b_e_par_geo_d_z;   //!
   TBranch        *b_e_par_nucleus_target;   //!
   TBranch        *b_e_par_nucleus_model;   //!
   TBranch        *b_e_par_dyn_qel_cc;   //!
   TBranch        *b_e_par_dyn_qel_nc;   //!
   TBranch        *b_e_par_dyn_res_cc;   //!
   TBranch        *b_e_par_dyn_res_nc;   //!
   TBranch        *b_e_par_dyn_dis_cc;   //!
   TBranch        *b_e_par_dyn_dis_nc;   //!
   TBranch        *b_e_par_dyn_coh_cc;   //!
   TBranch        *b_e_par_dyn_coh_nc;   //!
   TBranch        *b_e_par_dyn_mec_cc;   //!
   TBranch        *b_e_par_dyn_mec_nc;   //!
   TBranch        *b_e_par_qel_vector_ff_set;   //!
   TBranch        *b_e_par_qel_axial_ff_set;   //!
   TBranch        *b_e_par_qel_rpa;   //!
   TBranch        *b_e_par_qel_strange;   //!
   TBranch        *b_e_par_qel_strangeEM;   //!
   TBranch        *b_e_par_delta_s;   //!
   TBranch        *b_e_par_qel_cc_axial_mass;   //!
   TBranch        *b_e_par_qel_nc_axial_mass;   //!
   TBranch        *b_e_par_qel_s_axial_mass;   //!
   TBranch        *b_e_par_flux_correction;   //!
   TBranch        *b_e_par_sf_method;   //!
   TBranch        *b_e_par_sf_pb;   //!
   TBranch        *b_e_par_cc_smoothing;   //!
   TBranch        *b_e_par_delta_FF_set;   //!
   TBranch        *b_e_par_pion_axial_mass;   //!
   TBranch        *b_e_par_pion_C5A;   //!
   TBranch        *b_e_par_spp_precision;   //!
   TBranch        *b_e_par_res_dis_cut;   //!
   TBranch        *b_e_par_coh_mass_correction;   //!
   TBranch        *b_e_par_coh_new;   //!
   TBranch        *b_e_par_mec_kind;   //!
   TBranch        *b_e_par_mec_ratio_pp;   //!
   TBranch        *b_e_par_mec_ratio_ppp;   //!
   TBranch        *b_e_par_kaskada_on;   //!
   TBranch        *b_e_par_kaskada_w;   //!
   TBranch        *b_e_par_kaskada_redo;   //!
   TBranch        *b_e_par_kaskada_writeall;   //!
   TBranch        *b_e_par_formation_zone;   //!
   TBranch        *b_e_par_tau;   //!
   TBranch        *b_e_par_formation_length;   //!
   TBranch        *b_e_par_first_step;   //!
   TBranch        *b_e_par_step;   //!
   TBranch        *b_e_par_xsec;   //!
   TBranch        *b_e_par_pauli_blocking;   //!
   TBranch        *b_e_par_mixed_order;   //!
   TBranch        *b_e_par_path_to_data;   //!
   TBranch        *b_e_in_;   //!
   TBranch        *b_in_t;   //!
   TBranch        *b_in_x;   //!
   TBranch        *b_in_y;   //!
   TBranch        *b_in_z;   //!
   TBranch        *b_in__mass;   //!
   TBranch        *b_in_r_t;   //!
   TBranch        *b_in_r_x;   //!
   TBranch        *b_in_r_y;   //!
   TBranch        *b_in_r_z;   //!
   TBranch        *b_in_pdg;   //!
   TBranch        *b_in_ks;   //!
   TBranch        *b_in_orgin;   //!
   TBranch        *b_in_travelled;   //!
   TBranch        *b_in_id;   //!
   TBranch        *b_in_mother;   //!
   TBranch        *b_in_endproc;   //!
   TBranch        *b_in_his_fermi;   //!
   TBranch        *b_in_primary;   //!
   TBranch        *b_e_temp_;   //!
   TBranch        *b_temp_t;   //!
   TBranch        *b_temp_x;   //!
   TBranch        *b_temp_y;   //!
   TBranch        *b_temp_z;   //!
   TBranch        *b_temp__mass;   //!
   TBranch        *b_temp_r_t;   //!
   TBranch        *b_temp_r_x;   //!
   TBranch        *b_temp_r_y;   //!
   TBranch        *b_temp_r_z;   //!
   TBranch        *b_temp_pdg;   //!
   TBranch        *b_temp_ks;   //!
   TBranch        *b_temp_orgin;   //!
   TBranch        *b_temp_travelled;   //!
   TBranch        *b_temp_id;   //!
   TBranch        *b_temp_mother;   //!
   TBranch        *b_temp_endproc;   //!
   TBranch        *b_temp_his_fermi;   //!
   TBranch        *b_temp_primary;   //!
   TBranch        *b_e_out_;   //!
   TBranch        *b_out_t;   //!
   TBranch        *b_out_x;   //!
   TBranch        *b_out_y;   //!
   TBranch        *b_out_z;   //!
   TBranch        *b_out__mass;   //!
   TBranch        *b_out_r_t;   //!
   TBranch        *b_out_r_x;   //!
   TBranch        *b_out_r_y;   //!
   TBranch        *b_out_r_z;   //!
   TBranch        *b_out_pdg;   //!
   TBranch        *b_out_ks;   //!
   TBranch        *b_out_orgin;   //!
   TBranch        *b_out_travelled;   //!
   TBranch        *b_out_id;   //!
   TBranch        *b_out_mother;   //!
   TBranch        *b_out_endproc;   //!
   TBranch        *b_out_his_fermi;   //!
   TBranch        *b_out_primary;   //!
   TBranch        *b_e_post_;   //!
   TBranch        *b_post_t;   //!
   TBranch        *b_post_x;   //!
   TBranch        *b_post_y;   //!
   TBranch        *b_post_z;   //!
   TBranch        *b_post__mass;   //!
   TBranch        *b_post_r_t;   //!
   TBranch        *b_post_r_x;   //!
   TBranch        *b_post_r_y;   //!
   TBranch        *b_post_r_z;   //!
   TBranch        *b_post_pdg;   //!
   TBranch        *b_post_ks;   //!
   TBranch        *b_post_orgin;   //!
   TBranch        *b_post_travelled;   //!
   TBranch        *b_post_id;   //!
   TBranch        *b_post_mother;   //!
   TBranch        *b_post_endproc;   //!
   TBranch        *b_post_his_fermi;   //!
   TBranch        *b_post_primary;   //!
   TBranch        *b_e_all_;   //!
   TBranch        *b_all_t;   //!
   TBranch        *b_all_x;   //!
   TBranch        *b_all_y;   //!
   TBranch        *b_all_z;   //!
   TBranch        *b_all__mass;   //!
   TBranch        *b_all_r_t;   //!
   TBranch        *b_all_r_x;   //!
   TBranch        *b_all_r_y;   //!
   TBranch        *b_all_r_z;   //!
   TBranch        *b_all_pdg;   //!
   TBranch        *b_all_ks;   //!
   TBranch        *b_all_orgin;   //!
   TBranch        *b_all_travelled;   //!
   TBranch        *b_all_id;   //!
   TBranch        *b_all_mother;   //!
   TBranch        *b_all_endproc;   //!
   TBranch        *b_all_his_fermi;   //!
   TBranch        *b_all_primary;   //!
   TBranch        *b_e_weight;   //!
   TBranch        *b_e_norm;   //!
   TBranch        *b_e_r_x;   //!
   TBranch        *b_e_r_y;   //!
   TBranch        *b_e_r_z;   //!
   TBranch        *b_e_density;   //!
   TBranch        *b_e_dyn;   //!
   TBranch        *b_e_nod;   //!
   TBranch        *b_e_pr;   //!
   TBranch        *b_e_nr;   //!

   tree(TTree *tree=0);
   virtual ~tree();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef tree_cxx
tree::tree(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("/net/nudsk0001/d00/scratch/spitzj/eventsout_nuwroxsec_numu_kpipe_3_25_2015.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("/net/nudsk0001/d00/scratch/spitzj/eventsout_nuwroxsec_numu_kpipe_3_25_2015.root");
      }
      f->GetObject("treeout",tree);

   }
   Init(tree);
}

tree::~tree()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t tree::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t tree::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void tree::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_e_fUniqueID);
   fChain->SetBranchAddress("fBits", &fBits, &b_e_fBits);
   fChain->SetBranchAddress("flag.qel", &flag_qel, &b_e_flag_qel);
   fChain->SetBranchAddress("flag.res", &flag_res, &b_e_flag_res);
   fChain->SetBranchAddress("flag.dis", &flag_dis, &b_e_flag_dis);
   fChain->SetBranchAddress("flag.coh", &flag_coh, &b_e_flag_coh);
   fChain->SetBranchAddress("flag.mec", &flag_mec, &b_e_flag_mec);
   fChain->SetBranchAddress("flag.nc", &flag_nc, &b_e_flag_nc);
   fChain->SetBranchAddress("flag.cc", &flag_cc, &b_e_flag_cc);
   fChain->SetBranchAddress("flag.anty", &flag_anty, &b_e_flag_anty);
   fChain->SetBranchAddress("par.random_seed", &par_random_seed, &b_e_par_random_seed);
   fChain->SetBranchAddress("par.number_of_events", &par_number_of_events, &b_e_par_number_of_events);
   fChain->SetBranchAddress("par.number_of_test_events", &par_number_of_test_events, &b_e_par_number_of_test_events);
   fChain->SetBranchAddress("par.save_test_events", &par_save_test_events, &b_e_par_save_test_events);
   fChain->SetBranchAddress("par.user_events", &par_user_events, &b_e_par_user_events);
   fChain->SetBranchAddress("par.user_params.string", &par_user_params_string, &b_e_par_user_params_string);
   fChain->SetBranchAddress("par.beam_type", &par_beam_type, &b_e_par_beam_type);
   fChain->SetBranchAddress("par.beam_energy.string", &par_beam_energy_string, &b_e_par_beam_energy_string);
   fChain->SetBranchAddress("par.beam_particle", &par_beam_particle, &b_e_par_beam_particle);
   fChain->SetBranchAddress("par.beam_direction.x", &par_beam_direction_x, &b_e_par_beam_direction_x);
   fChain->SetBranchAddress("par.beam_direction.y", &par_beam_direction_y, &b_e_par_beam_direction_y);
   fChain->SetBranchAddress("par.beam_direction.z", &par_beam_direction_z, &b_e_par_beam_direction_z);
   fChain->SetBranchAddress("par.beam_content.string", &par_beam_content_string, &b_e_par_beam_content_string);
   fChain->SetBranchAddress("par.beam_folder", &par_beam_folder, &b_e_par_beam_folder);
   fChain->SetBranchAddress("par.beam_file_first", &par_beam_file_first, &b_e_par_beam_file_first);
   fChain->SetBranchAddress("par.beam_file_limit", &par_beam_file_limit, &b_e_par_beam_file_limit);
   fChain->SetBranchAddress("par.beam_weighted", &par_beam_weighted, &b_e_par_beam_weighted);
   fChain->SetBranchAddress("par.beam_offset.x", &par_beam_offset_x, &b_e_par_beam_offset_x);
   fChain->SetBranchAddress("par.beam_offset.y", &par_beam_offset_y, &b_e_par_beam_offset_y);
   fChain->SetBranchAddress("par.beam_offset.z", &par_beam_offset_z, &b_e_par_beam_offset_z);
   fChain->SetBranchAddress("par.beam_placement", &par_beam_placement, &b_e_par_beam_placement);
   fChain->SetBranchAddress("par.beam_test_only", &par_beam_test_only, &b_e_par_beam_test_only);
   fChain->SetBranchAddress("par.target_type", &par_target_type, &b_e_par_target_type);
   fChain->SetBranchAddress("par.nucleus_p", &par_nucleus_p, &b_e_par_nucleus_p);
   fChain->SetBranchAddress("par.nucleus_n", &par_nucleus_n, &b_e_par_nucleus_n);
   fChain->SetBranchAddress("par.nucleus_E_b", &par_nucleus_E_b, &b_e_par_nucleus_E_b);
   fChain->SetBranchAddress("par.nucleus_kf", &par_nucleus_kf, &b_e_par_nucleus_kf);
   fChain->SetBranchAddress("par.target_content.string", &par_target_content_string, &b_e_par_target_content_string);
   fChain->SetBranchAddress("par.geo_file", &par_geo_file, &b_e_par_geo_file);
   fChain->SetBranchAddress("par.geo_name", &par_geo_name, &b_e_par_geo_name);
   fChain->SetBranchAddress("par.geo_volume", &par_geo_volume, &b_e_par_geo_volume);
   fChain->SetBranchAddress("par.geo_o.x", &par_geo_o_x, &b_e_par_geo_o_x);
   fChain->SetBranchAddress("par.geo_o.y", &par_geo_o_y, &b_e_par_geo_o_y);
   fChain->SetBranchAddress("par.geo_o.z", &par_geo_o_z, &b_e_par_geo_o_z);
   fChain->SetBranchAddress("par.geo_d.x", &par_geo_d_x, &b_e_par_geo_d_x);
   fChain->SetBranchAddress("par.geo_d.y", &par_geo_d_y, &b_e_par_geo_d_y);
   fChain->SetBranchAddress("par.geo_d.z", &par_geo_d_z, &b_e_par_geo_d_z);
   fChain->SetBranchAddress("par.nucleus_target", &par_nucleus_target, &b_e_par_nucleus_target);
   fChain->SetBranchAddress("par.nucleus_model", &par_nucleus_model, &b_e_par_nucleus_model);
   fChain->SetBranchAddress("par.dyn_qel_cc", &par_dyn_qel_cc, &b_e_par_dyn_qel_cc);
   fChain->SetBranchAddress("par.dyn_qel_nc", &par_dyn_qel_nc, &b_e_par_dyn_qel_nc);
   fChain->SetBranchAddress("par.dyn_res_cc", &par_dyn_res_cc, &b_e_par_dyn_res_cc);
   fChain->SetBranchAddress("par.dyn_res_nc", &par_dyn_res_nc, &b_e_par_dyn_res_nc);
   fChain->SetBranchAddress("par.dyn_dis_cc", &par_dyn_dis_cc, &b_e_par_dyn_dis_cc);
   fChain->SetBranchAddress("par.dyn_dis_nc", &par_dyn_dis_nc, &b_e_par_dyn_dis_nc);
   fChain->SetBranchAddress("par.dyn_coh_cc", &par_dyn_coh_cc, &b_e_par_dyn_coh_cc);
   fChain->SetBranchAddress("par.dyn_coh_nc", &par_dyn_coh_nc, &b_e_par_dyn_coh_nc);
   fChain->SetBranchAddress("par.dyn_mec_cc", &par_dyn_mec_cc, &b_e_par_dyn_mec_cc);
   fChain->SetBranchAddress("par.dyn_mec_nc", &par_dyn_mec_nc, &b_e_par_dyn_mec_nc);
   fChain->SetBranchAddress("par.qel_vector_ff_set", &par_qel_vector_ff_set, &b_e_par_qel_vector_ff_set);
   fChain->SetBranchAddress("par.qel_axial_ff_set", &par_qel_axial_ff_set, &b_e_par_qel_axial_ff_set);
   fChain->SetBranchAddress("par.qel_rpa", &par_qel_rpa, &b_e_par_qel_rpa);
   fChain->SetBranchAddress("par.qel_strange", &par_qel_strange, &b_e_par_qel_strange);
   fChain->SetBranchAddress("par.qel_strangeEM", &par_qel_strangeEM, &b_e_par_qel_strangeEM);
   fChain->SetBranchAddress("par.delta_s", &par_delta_s, &b_e_par_delta_s);
   fChain->SetBranchAddress("par.qel_cc_axial_mass", &par_qel_cc_axial_mass, &b_e_par_qel_cc_axial_mass);
   fChain->SetBranchAddress("par.qel_nc_axial_mass", &par_qel_nc_axial_mass, &b_e_par_qel_nc_axial_mass);
   fChain->SetBranchAddress("par.qel_s_axial_mass", &par_qel_s_axial_mass, &b_e_par_qel_s_axial_mass);
   fChain->SetBranchAddress("par.flux_correction", &par_flux_correction, &b_e_par_flux_correction);
   fChain->SetBranchAddress("par.sf_method", &par_sf_method, &b_e_par_sf_method);
   fChain->SetBranchAddress("par.sf_pb", &par_sf_pb, &b_e_par_sf_pb);
   fChain->SetBranchAddress("par.cc_smoothing", &par_cc_smoothing, &b_e_par_cc_smoothing);
   fChain->SetBranchAddress("par.delta_FF_set", &par_delta_FF_set, &b_e_par_delta_FF_set);
   fChain->SetBranchAddress("par.pion_axial_mass", &par_pion_axial_mass, &b_e_par_pion_axial_mass);
   fChain->SetBranchAddress("par.pion_C5A", &par_pion_C5A, &b_e_par_pion_C5A);
   fChain->SetBranchAddress("par.spp_precision", &par_spp_precision, &b_e_par_spp_precision);
   fChain->SetBranchAddress("par.res_dis_cut", &par_res_dis_cut, &b_e_par_res_dis_cut);
   fChain->SetBranchAddress("par.coh_mass_correction", &par_coh_mass_correction, &b_e_par_coh_mass_correction);
   fChain->SetBranchAddress("par.coh_new", &par_coh_new, &b_e_par_coh_new);
   fChain->SetBranchAddress("par.mec_kind", &par_mec_kind, &b_e_par_mec_kind);
   fChain->SetBranchAddress("par.mec_ratio_pp", &par_mec_ratio_pp, &b_e_par_mec_ratio_pp);
   fChain->SetBranchAddress("par.mec_ratio_ppp", &par_mec_ratio_ppp, &b_e_par_mec_ratio_ppp);
   fChain->SetBranchAddress("par.kaskada_on", &par_kaskada_on, &b_e_par_kaskada_on);
   fChain->SetBranchAddress("par.kaskada_w", &par_kaskada_w, &b_e_par_kaskada_w);
   fChain->SetBranchAddress("par.kaskada_redo", &par_kaskada_redo, &b_e_par_kaskada_redo);
   fChain->SetBranchAddress("par.kaskada_writeall", &par_kaskada_writeall, &b_e_par_kaskada_writeall);
   fChain->SetBranchAddress("par.formation_zone", &par_formation_zone, &b_e_par_formation_zone);
   fChain->SetBranchAddress("par.tau", &par_tau, &b_e_par_tau);
   fChain->SetBranchAddress("par.formation_length", &par_formation_length, &b_e_par_formation_length);
   fChain->SetBranchAddress("par.first_step", &par_first_step, &b_e_par_first_step);
   fChain->SetBranchAddress("par.step", &par_step, &b_e_par_step);
   fChain->SetBranchAddress("par.xsec", &par_xsec, &b_e_par_xsec);
   fChain->SetBranchAddress("par.pauli_blocking", &par_pauli_blocking, &b_e_par_pauli_blocking);
   fChain->SetBranchAddress("par.mixed_order", &par_mixed_order, &b_e_par_mixed_order);
   fChain->SetBranchAddress("par.path_to_data", &par_path_to_data, &b_e_par_path_to_data);
   fChain->SetBranchAddress("in", &in_, &b_e_in_);
   fChain->SetBranchAddress("in.t", in_t, &b_in_t);
   fChain->SetBranchAddress("in.x", in_x, &b_in_x);
   fChain->SetBranchAddress("in.y", in_y, &b_in_y);
   fChain->SetBranchAddress("in.z", in_z, &b_in_z);
   fChain->SetBranchAddress("in._mass", in__mass, &b_in__mass);
   fChain->SetBranchAddress("in.r.t", in_r_t, &b_in_r_t);
   fChain->SetBranchAddress("in.r.x", in_r_x, &b_in_r_x);
   fChain->SetBranchAddress("in.r.y", in_r_y, &b_in_r_y);
   fChain->SetBranchAddress("in.r.z", in_r_z, &b_in_r_z);
   fChain->SetBranchAddress("in.pdg", in_pdg, &b_in_pdg);
   fChain->SetBranchAddress("in.ks", in_ks, &b_in_ks);
   fChain->SetBranchAddress("in.orgin", in_orgin, &b_in_orgin);
   fChain->SetBranchAddress("in.travelled", in_travelled, &b_in_travelled);
   fChain->SetBranchAddress("in.id", in_id, &b_in_id);
   fChain->SetBranchAddress("in.mother", in_mother, &b_in_mother);
   fChain->SetBranchAddress("in.endproc", in_endproc, &b_in_endproc);
   fChain->SetBranchAddress("in.his_fermi", in_his_fermi, &b_in_his_fermi);
   fChain->SetBranchAddress("in.primary", in_primary, &b_in_primary);
   fChain->SetBranchAddress("temp", &temp_, &b_e_temp_);
   fChain->SetBranchAddress("temp.t", temp_t, &b_temp_t);
   fChain->SetBranchAddress("temp.x", temp_x, &b_temp_x);
   fChain->SetBranchAddress("temp.y", temp_y, &b_temp_y);
   fChain->SetBranchAddress("temp.z", temp_z, &b_temp_z);
   fChain->SetBranchAddress("temp._mass", temp__mass, &b_temp__mass);
   fChain->SetBranchAddress("temp.r.t", temp_r_t, &b_temp_r_t);
   fChain->SetBranchAddress("temp.r.x", temp_r_x, &b_temp_r_x);
   fChain->SetBranchAddress("temp.r.y", temp_r_y, &b_temp_r_y);
   fChain->SetBranchAddress("temp.r.z", temp_r_z, &b_temp_r_z);
   fChain->SetBranchAddress("temp.pdg", temp_pdg, &b_temp_pdg);
   fChain->SetBranchAddress("temp.ks", temp_ks, &b_temp_ks);
   fChain->SetBranchAddress("temp.orgin", temp_orgin, &b_temp_orgin);
   fChain->SetBranchAddress("temp.travelled", temp_travelled, &b_temp_travelled);
   fChain->SetBranchAddress("temp.id", temp_id, &b_temp_id);
   fChain->SetBranchAddress("temp.mother", temp_mother, &b_temp_mother);
   fChain->SetBranchAddress("temp.endproc", temp_endproc, &b_temp_endproc);
   fChain->SetBranchAddress("temp.his_fermi", temp_his_fermi, &b_temp_his_fermi);
   fChain->SetBranchAddress("temp.primary", temp_primary, &b_temp_primary);
   fChain->SetBranchAddress("out", &out_, &b_e_out_);
   fChain->SetBranchAddress("out.t", out_t, &b_out_t);
   fChain->SetBranchAddress("out.x", out_x, &b_out_x);
   fChain->SetBranchAddress("out.y", out_y, &b_out_y);
   fChain->SetBranchAddress("out.z", out_z, &b_out_z);
   fChain->SetBranchAddress("out._mass", out__mass, &b_out__mass);
   fChain->SetBranchAddress("out.r.t", out_r_t, &b_out_r_t);
   fChain->SetBranchAddress("out.r.x", out_r_x, &b_out_r_x);
   fChain->SetBranchAddress("out.r.y", out_r_y, &b_out_r_y);
   fChain->SetBranchAddress("out.r.z", out_r_z, &b_out_r_z);
   fChain->SetBranchAddress("out.pdg", out_pdg, &b_out_pdg);
   fChain->SetBranchAddress("out.ks", out_ks, &b_out_ks);
   fChain->SetBranchAddress("out.orgin", out_orgin, &b_out_orgin);
   fChain->SetBranchAddress("out.travelled", out_travelled, &b_out_travelled);
   fChain->SetBranchAddress("out.id", out_id, &b_out_id);
   fChain->SetBranchAddress("out.mother", out_mother, &b_out_mother);
   fChain->SetBranchAddress("out.endproc", out_endproc, &b_out_endproc);
   fChain->SetBranchAddress("out.his_fermi", out_his_fermi, &b_out_his_fermi);
   fChain->SetBranchAddress("out.primary", out_primary, &b_out_primary);
   fChain->SetBranchAddress("post", &post_, &b_e_post_);
   fChain->SetBranchAddress("post.t", post_t, &b_post_t);
   fChain->SetBranchAddress("post.x", post_x, &b_post_x);
   fChain->SetBranchAddress("post.y", post_y, &b_post_y);
   fChain->SetBranchAddress("post.z", post_z, &b_post_z);
   fChain->SetBranchAddress("post._mass", post__mass, &b_post__mass);
   fChain->SetBranchAddress("post.r.t", post_r_t, &b_post_r_t);
   fChain->SetBranchAddress("post.r.x", post_r_x, &b_post_r_x);
   fChain->SetBranchAddress("post.r.y", post_r_y, &b_post_r_y);
   fChain->SetBranchAddress("post.r.z", post_r_z, &b_post_r_z);
   fChain->SetBranchAddress("post.pdg", post_pdg, &b_post_pdg);
   fChain->SetBranchAddress("post.ks", post_ks, &b_post_ks);
   fChain->SetBranchAddress("post.orgin", post_orgin, &b_post_orgin);
   fChain->SetBranchAddress("post.travelled", post_travelled, &b_post_travelled);
   fChain->SetBranchAddress("post.id", post_id, &b_post_id);
   fChain->SetBranchAddress("post.mother", post_mother, &b_post_mother);
   fChain->SetBranchAddress("post.endproc", post_endproc, &b_post_endproc);
   fChain->SetBranchAddress("post.his_fermi", post_his_fermi, &b_post_his_fermi);
   fChain->SetBranchAddress("post.primary", post_primary, &b_post_primary);
   fChain->SetBranchAddress("all", &all_, &b_e_all_);
   fChain->SetBranchAddress("all.t", all_t, &b_all_t);
   fChain->SetBranchAddress("all.x", all_x, &b_all_x);
   fChain->SetBranchAddress("all.y", all_y, &b_all_y);
   fChain->SetBranchAddress("all.z", all_z, &b_all_z);
   fChain->SetBranchAddress("all._mass", all__mass, &b_all__mass);
   fChain->SetBranchAddress("all.r.t", all_r_t, &b_all_r_t);
   fChain->SetBranchAddress("all.r.x", all_r_x, &b_all_r_x);
   fChain->SetBranchAddress("all.r.y", all_r_y, &b_all_r_y);
   fChain->SetBranchAddress("all.r.z", all_r_z, &b_all_r_z);
   fChain->SetBranchAddress("all.pdg", all_pdg, &b_all_pdg);
   fChain->SetBranchAddress("all.ks", all_ks, &b_all_ks);
   fChain->SetBranchAddress("all.orgin", all_orgin, &b_all_orgin);
   fChain->SetBranchAddress("all.travelled", all_travelled, &b_all_travelled);
   fChain->SetBranchAddress("all.id", all_id, &b_all_id);
   fChain->SetBranchAddress("all.mother", all_mother, &b_all_mother);
   fChain->SetBranchAddress("all.endproc", all_endproc, &b_all_endproc);
   fChain->SetBranchAddress("all.his_fermi", all_his_fermi, &b_all_his_fermi);
   fChain->SetBranchAddress("all.primary", all_primary, &b_all_primary);
   fChain->SetBranchAddress("weight", &weight, &b_e_weight);
   fChain->SetBranchAddress("norm", &norm, &b_e_norm);
   fChain->SetBranchAddress("r.x", &r_x, &b_e_r_x);
   fChain->SetBranchAddress("r.y", &r_y, &b_e_r_y);
   fChain->SetBranchAddress("r.z", &r_z, &b_e_r_z);
   fChain->SetBranchAddress("density", &density, &b_e_density);
   fChain->SetBranchAddress("dyn", &dyn, &b_e_dyn);
   fChain->SetBranchAddress("nod[12]", nod, &b_e_nod);
   fChain->SetBranchAddress("pr", &pr, &b_e_pr);
   fChain->SetBranchAddress("nr", &nr, &b_e_nr);
   Notify();
}

Bool_t tree::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void tree::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t tree::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef tree_cxx
