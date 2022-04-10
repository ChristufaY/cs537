#include "types.h"
#include "stat.h"
#include "user.h"
#include "iostat.h"
int
main(int argc, char *argv[])
{

  if(argc < 3 || argc > 3){
    exit();
  }

   
   struct iostat *i;
   i = (struct iostat*)malloc(sizeof(struct iostat));
   for(int j = 0; j < atoi(argv[1]); j++)
      read(-1, (void*)0, -1);
   for(int k = 0; k < atoi(argv[2]); k++)
      write(-1, (void*)0, -1);
   int result = getiocounts(i);

   printf(1, "%d %d %d\n", result, i->readcount, i->writecount);
   free(i);
   exit();

}
