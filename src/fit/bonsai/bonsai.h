#ifndef BONSAI
#define BONSAI
class vertex;

class bonsai
{
  static bonsai  *maxbranch;
  static vertex *fit;

  int    point_index,nbranch;
  bonsai **branches;

  inline void makebranch(int index);
  inline void killbranch(int i)
    { if (branches[i]==NULL) delete branches[i];
      branches[i]=NULL; }
  inline bonsai(int index);

 public:
  bonsai(int index,vertex *vfit);
  bonsai(int n,int *index,vertex *vfit);
  inline ~bonsai(void)
    {
      int i;
      
      if (branches==NULL) return;
      for(i=0; i<nbranch; i++)
	killbranch(i);
      delete branches;
      branches=NULL;
      nbranch=0;
    }

  void sprout(void);
  int  prune(void);
  void print(int it);
  inline int index(void)
    { return(point_index); }
  inline int bestindex(void)
    { return(maxbranch->point_index); }
  /*inline float like(void)
    { return(likelihood); }
  inline void pos(float *vert)
    {
      *vert=*vertex;
      vert[1]=vertex[1];
      vert[2]=vertex[2];
      vert[3]=vertex[3];
    };
  inline float bestlike(void)
    { return(maxbranch->like()); }
  inline void bestpos(float *vert)
  { maxbranch->pos(vert); }*/
};
#endif
