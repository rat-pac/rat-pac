#ifndef FIT_PARAM
#define FIT_PARAM

#define FIT_PARAM_NONE -32767

#define MAX_NUMBER 10
class fit_param
{
  static char      *numbers[MAX_NUMBER];
  static float     tres;
  static float     tcoin;
  static float     dlim;
  static float     tlim;
  static short int nselall;
  static float     gcon0;
  static float     dw4hit;
  static float     dwfit;
  static float     clusgtheta0;
  static float     clusgthetaplus;
  static float     clusgthetaminus;
  static float     clusgdirweight;
  static short int np;
  static float     *cdwall;
  static float     *gdiff;
  static float     lastdiff;
  static float     *gfrac;
  static float     lastfrac;
  static float     tim0;
  static float     *time;
  static float     *clustheta0;
  static float     *clusthetaplus;
  static float     *clusthetaminus;
  static float     *clusdirweight;
  static float     *rmin;
  static float     *rstop;
  static float     bgrid;
  static float     oc0;
  static float     opangle;
  static float     omangle;
  static float     obgdiff;
  static float     blastdiff;
  static float     obgfrac;
  static float     cbgfrac;
  static float     blastfrac;
  static float     obrmin;
  static float     obrstop;
  static float     ocf;
  static float     bdwall;
  static float     bdwfit;
  static float     cgrid;
  static float     bpangle;
  static float     bmangle;
  static float     cc0;
  static float     bc0;
  static float     cpangle;
  static float     cmangle;
  static float     cbgdiff;
  static float     cbrmin;
  static float     cbrstop;

  char parse(char *line,unsigned char length,
	     char *key1,char *key2,char *key3,
	     char type,void *var);
  inline void parse(char *line);
  inline void ang_constr(float &theta_0,
			 float &theta_plus,float &theta_minus);
  inline void printspace(int n);

 public:
  fit_param(void);
  void print(void);
  inline float     tresolution(void)
    {
      return(tres);
    }
  inline float     tcoincidence(void)
    {
      return(tcoin);
    }
  inline float     pair_distance_limit_fraction(void)
    {
      return(dlim);
    }
  inline float     pair_time_limit_fraction(void)
    {
      return(tlim);
    }
  inline short int nsel_allcombo(void)
    {
      return(nselall);
    }
  inline float     init_grid_constant(void)
    {
      return(gcon0);
    }
  inline float     dwall4hit(void)
    {
      return(dw4hit);
    }
  inline float     dwallfit(void)
    {
      return(dwfit);
    }
  inline float     clusfit_grid_cos_theta(void)
    {
      return(clusgtheta0);
    }
  inline float     clusfit_grid_plus_deviation(void)
    {
      return(clusgthetaplus);
    }
  inline float     clusfit_grid_minus_deviation(void)
    {
      return(clusgthetaminus);
    }
  inline float     clusfit_grid_direction_weight(void)
    {
      return(clusgdirweight);
    }
  inline float     clusfit_cos_theta(int i)
    {
      return(clustheta0[i]);
    }
  inline float     clusfit_plus_deviation(int i)
    {
      return(clusthetaplus[i]);
    }
  inline float     clusfit_minus_deviation(int i)
    {
      return(clusthetaminus[i]);
    }
  inline float     clusfit_direction_weight(int i)
    {
      return(clusdirweight[i]);
    }
  inline short int npass(void)
    {
      return(np);
    }
  inline float     clus_dwall(int i)
    {
      return(cdwall[i]);
    }
  inline float     min_gdn_difference(int i)
    {
      return(gdiff[i]);
    }
  inline float     last_min_gdn_difference(void)
    {
      return(lastdiff);
    }
  inline float     gdn_fraction(int i)
    {
      return(gfrac[i]);
    }
  inline float     last_gdn_fraction(void)
    {
      return(lastfrac);
    }
  inline float     grid_time_window(void)
    {
      return(tim0);
    }
  inline float     time_window(int i)
    {
      return(time[i]);
    }
  inline float     minimum_radius(int i)
    {
      return(rmin[i]);
    }
  inline float     stop_radius(int i)
    {
      return(rstop[i]);
    }
  inline float     clusgrid(void)
    {
      return(cgrid);
    }
  inline float     bongrid(void)
    {
      return(bgrid);
    }
  inline float     oangle(void)
    {
      return(oc0);
    }
  inline float     oplusdevangle(void)
    {
      return(opangle);
    }
  inline float     ominusdevangle(void)
    {
      return(omangle);
    }
  inline float     bminusdevangle(void)
    {
      return(bmangle);
    }
  inline float     obon_min_gdn_difference(void)
    {
      return(obgdiff);
    }
  inline float     bon_last_min_gdn_difference(void)
    {
      return(blastdiff);
    }
  inline float    obon_gdn_fraction(void)
    {
      return(obgfrac);
    }
  inline float    cbon_gdn_fraction(void)
    {
      return(cbgfrac);
    }
  inline float     bon_last_gdn_fraction(void)
    {
      return(blastfrac);
    }
  inline float     obon_minimum_radius(void)
    {
      return(obrmin);
    }
  inline float     obon_stop_radius(void)
    {
      return(obrstop);
    }
  inline float     bon_dwallfit(void)
    {
      return(bdwfit);
    }
  inline float     bon_dwall(void)
    {
      return(bdwall);
    }
  inline float     cangle(void)
    {
      return(cc0);
    }
  inline float     bangle(void)
    {
      return(bc0);
    }
  inline float     bplusdevangle(void)
    {
      return(bpangle);
    }
  inline float     cplusdevangle(void)
    {
      return(cpangle);
    }
  inline float     cminusdevangle(void)
    {
      return(cmangle);
    }
  inline float     cbon_min_gdn_difference(void)
    {
      return(cbgdiff);
    }
  inline float     cbon_minimum_radius(void)
    {
      return(cbrmin);
    }
  inline float     cbon_stop_radius(void)
    {
      return(cbrstop);
    }
};
#endif
