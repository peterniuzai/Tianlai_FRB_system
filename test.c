#include <stdio.h>
#include <math.h>
#include <stdint.h>
//#include <io.h>
//#include <process.h>
int main()
{
    uint64_t b=2102;
    int c = 1000;
    int d = b % c;
    uint64_t T_RSL = ((100*1000/4/2048)*2048*4/1000);
    printf("\nTime resolution:%d\n\n",T_RSL);
    printf("d is %04d\n",d);
    char aaa[100]={"1233"};
    char bbb[100]={"1235"};
    c = strcmp(aaa,bbb);
    printf("result=%d\n",c);
    char * p={"123What is this?"};

//    exit(0);
    //printf("\n??? %c\n",*(p+1));
    char  con[50];
    sprintf(con,"%c%c\0",*(p+1),*(p+2));
    int aa=atoi(con);
    printf("\n???%s\n",con);
    printf("****%d\n",aa);

    char a[100];
    sprintf(a,"B%d-%d\n",2,3);
    printf("Beam %s",a); 
    char f_dir[100]={"/FRBTMPFS/test5/"};
    if (access(f_dir,0))
	{
	 printf("Does not Exists!\n");
	 c=mkdir(f_dir);
	 printf("Already create\n");
	}
    else
	{
	 printf("Already Exists!\n");
	}

}
