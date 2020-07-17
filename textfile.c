#include <stdio.h>  
  
int main()  
{  
    int a[10]={0};
    int i;
    FILE *fpRead=fopen("inputfile.txt","r");  //其中"r"是表示 读
    if(fpRead==NULL)  
    {  
        return 0;  
    }  
    for( i=0;i<6;i++)  
    {  
        fscanf(fpRead,"%f ",&a[i]);  
        printf("%f ",a[i]);
        
    }  
    printf("\n");
    return 1;  
} 
