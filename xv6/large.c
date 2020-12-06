#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main()
{
  char buf[512];
  int fd, i, sectors;

  fd = open("large.file", O_CREATE | O_WRONLY);
  if(fd < 0){
    printf(2, "large: cannot open large.file for writing\n");
    exit();
  }

  sectors = 0;
  while(1){
    *(int*)buf = sectors;
    int cc = write(fd, buf, sizeof(buf));
    if(cc <= 0)
      break;
    sectors++;
	if (sectors % 100 == 0)
		printf(2, ".");
  }

  printf(1, "\nwrote %d blocks\n", sectors);

  close(fd);
  fd = open("large.file", O_RDONLY);
  if(fd < 0){
    printf(2, "large: cannot re-open large.file for reading\n");
    exit();
  }
  for(i = 0; i < sectors; i++){
    int cc = read(fd, buf, sizeof(buf));
    if(cc <= 0){
      printf(2, "large: read error at sector %d\n", i);
      exit();
    }
    if(*(int*)buf != i){
      printf(2, "large: read the wrong data (%d) for sector %d\n",
             *(int*)buf, i);
      exit();
    }
  }

  printf(1, "done; ok\n"); 

  exit();
}
