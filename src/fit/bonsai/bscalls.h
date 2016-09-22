
void itfitter_init(comtype *itgeom);
void it_sort(comtype2 *itevent);
int it_clusfit(float *vert,float *result,float *maxgood,int *nsel,
	       comtype2 *itevent,void *gridbuffer,short int maxsize);
int it_bsfit(float *vertex,float *result,float *maxlike,int *nsel,
	     comtype2 *itevent,void *gridbuffer);
void it_clus_bonsai_fit(float *clusvert,float *bonsaivert,
			float *clusresult,float *bonsairesult,
			float *maxgood,float *maxlike,
			int *nsel,int *nclusfit,int *nbonsaifit,
			comtype2 *itevent);
void it_clus_and_bonsai_fit(float *clusvert,float *bonsaivert,
			    float *clusresult,float *bonsairesult,
			    float *maxgood,float *maxlike,
			    int *nsel,int *nclusfit,int *nbonsaifit,
			    float cluswallcut,comtype2 *itevent);
void itfitter_exit(void);
void it_vfwrite(FILE *itvw,unsigned int event_number,int trigger_type,
		int keep,int flag,unsigned short clock48_high,
		unsigned short clock48_middle,unsigned short clock48_low,
		unsigned int fit_tubes,
		unsigned int cfit,unsigned int cvertex_tubes,
		float *cvtx,float *cdir,float cgof,
		unsigned int bfit,unsigned int bvertex_tubes,
		float *bvtx,float *bdir,float bgof,float bll,float bll0);
short int it_vfread(FILE *itvw,unsigned int *event_number,int *trigger_type,
	       int *keep,int *flag,unsigned short *clock48_high,
	       unsigned short *clock48_middle,unsigned short *clock48_low,
	       unsigned int *fit_tubes,
	       unsigned int *cfit,unsigned int *cvertex_tubes,
	       float *cvtx,float *cdir,float *cgof,
	       unsigned int *bfit,unsigned int *bvertex_tubes,
	       float *bvtx,float *bdir,float *bgof,float *bll,float *bll0);
void cfbsloadinit_(void);
void cfbsinit_(int *np,float *ps);
void cfbsexit_(void);
void storetest_(int *nhit,int *cab,float *t,float *q,float *tsig,float *dwallmin,
		int *nsel,float *goodn);
int clusfit_(float *vert,float *result,float *maxgood,int *nsel,
	     int *nhit,int *cab,float *t,float *q);
int bonsaifit_(float *vert,float *result,float *maxlike,int *nsel,
	       int *nhit,int *cab,float *t,float *q);
int startbonsaifit_(float *vert,float *result,float *maxlike,int *nsel,
		    int *nhit,int *cab,float *t,float *q);
void clusbonsaifit_(float *clusvert,float *bonsaivert,
		    float *clusresult,float *bonsairesult,
		    float *maxgood,float *maxlike,
		    int *nsel,int *nclusfit,int *nbonsaifit,
		    int *nhit,int *cab,float *t,float *q);
void clusandbonsaifit_(float *clusvert,float *bonsaivert,
		       float *clusresult,float *bonsairesult,
		       float *maxgood,float *maxlike,
		       int *nsel,int *nclusfit,int *nbonsaifit,
		       float *cluswallcut,
		      int *nhit,int *cab,float *t,float *q);
void goodness_(float *guncor,float *vertex,int *nhit,int *cab,float *t,float *q);
int   nwin1_(float *vertex,float *tmin,float *tmax,
	     int *nhit,int *cab,float *t,float *q);
int   nwin2_(float *vertex,float *tmin,float *tmax,float *ttof,int *cab);
int   nwin3_(float *vertex,float *tmin,float *tmax,
	     float *ttof,float *d,int *cab);
void bsfilereadopen_(char *name,int length);
void bsfilewriteopen_(char *name,int length);
void bsfileclose_(void);
char bsfileitread(unsigned int *eventnr,comtype2 *itevent);
void bsfileitwrite(unsigned int eventnr,comtype2 *itevent);
