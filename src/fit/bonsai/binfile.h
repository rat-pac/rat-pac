#include <stdio.h>

class binfile
{
  FILE *fp;
  char offset[14];
  inline void order2(char *point);
  inline void order4(char *point);
  inline void order8(char *point);

 public:
  binfile(char *name,char mode);
  binfile(FILE *f);
  ~binfile(void);
  int read(int *&sizes,int *&numbers,void **&starts);
  void write(int *sizes,int *numbers,void **starts);
};
