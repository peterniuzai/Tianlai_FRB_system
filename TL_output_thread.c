/*
 * TL_output_thread.c
 * Chenhui Niu, 
 * NAOC
 * peterniu@nao.cas.cn
 */
//#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "hashpipe.h"
#include "TL_databuf.h"
#include "filterbank.h"
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include "TL_net_thread.h"
//#include "TL_net_thread.c"
//static int block_idx=0;
extern int beam_ID[4];
extern bool data_type;
extern bool start_file;
extern uint64_t init_mcnt;
static int GPS_time[8];
Mcntime_t File_time;
void ReadGPSTime()
{
        FILE *fp;
        fp = fopen("/FRBTMPFS/GPS_File.txt","r");

        char * ch;
        char tar[100];
//        int time[8];
	int cur_count = 0;
        while (fgets(tar, 1000,fp) != NULL)
        {
                ch = strtok(tar,"\n");
                while(ch!=NULL)
                {

                        GPS_time[cur_count]=atoi(ch);
                        ch = strtok(NULL,"\n");
                        cur_count++;
                }

        }

        fclose(fp);
}

Mcntime_t time_calculate(int gps_time[8], uint64_t netmcnt)
{
	int yy = gps_time[0];//->Year;
	int mm = gps_time[1];//->Month;
        int dd = gps_time[2];//->Day;
        int HH = gps_time[3];//->Hour;
        int MM = gps_time[4];//->Min;
        int SS = gps_time[5];//->Sec;
        int ms = gps_time[6];//->Msec;
        int us = gps_time[7];//->Usec;
        struct tm tm_str;
        tm_str.tm_year = yy;
        tm_str.tm_mon  = mm;
        tm_str.tm_mday = dd;
        tm_str.tm_hour = HH;
        tm_str.tm_min  = MM;
        tm_str.tm_sec  = SS;

	//uint64_t mcnt = netmcnt;
	uint64_t time_interval = (T_RSL*1000/4/2048)*2048*4/1000;
//	printf("Time interval%d\n", time_interval);
	uint64_t us_increase = netmcnt*time_interval;
	uint64_t us_Total = us + us_increase;
//	printf("Us:%d,us_increase:%ld,us_total:%ld\n", us,us_increase,us_Total);
//	printf("Netmcnt:%ld,init_mcnt:%ld\n",netmcnt,init_mcnt);
	
	File_time.Usec = us_Total%1000;
	File_time.Msec = (ms + us_Total/1000)%1000;
	tm_str.tm_sec  = SS + (ms + us_Total/1000)/1000;
        mktime(&tm_str);

//	Mcntime_t File_time;
	File_time.Year = tm_str.tm_year;
	File_time.Month= tm_str.tm_mon;
	File_time.Day  = tm_str.tm_mday;
	File_time.Hour = tm_str.tm_hour;
	File_time.Min  = tm_str.tm_min;
	File_time.Sec  = tm_str.tm_sec;

	return File_time;
}



typedef struct {
    uint64_t    mcnt;           // counter for packet
    bool        source_from;    // 0 - Integrated, 1 - Without Integrated
    int         BeamGroup_ID;   // beam ID
} packet_header_t;



static void *run(hashpipe_thread_args_t * args)
{
	printf("\n%.2f Mbytes for each Filterbank file.\n ",float(N_BYTES_PER_FILE)/1024/1024);
//	printf("\n%d Channels per Buff.\n ",N_CHANS_BUFF);
	ReadGPSTime();
	// Local aliases to shorten access to args fields
	// Our input buffer happens to be a TL_ouput_databuf
	TL_output_databuf_t *db = (TL_output_databuf_t *)args->ibuf;
	hashpipe_status_t st = args->st;
	const char * status_key = args->thread_desc->skey;
	int rv, N_files,dir_status;
	int block_idx = 0;
	uint64_t N_Bytes_save = 0;
	uint64_t N_Bytes_file = N_BYTES_PER_FILE;
	uint64_t netmcnt = 0;
	int filb_flag = 1;


	FILE * TL_file_Beam_0;
	FILE * TL_file_Beam_1;
	FILE * TL_file_Beam_2;
	FILE * TL_file_Beam_3;
	char f_fil_B0[250];
	char f_fil_B1[250];
	char f_fil_B2[250];
        char f_fil_B3[250];
	char Filname_B0[250]={""};
        char Filname_B1[250]={""};
	char Filname_B2[250]={""};
        char Filname_B3[250]={""};
		

	sleep(1);
	
        if (access("/FRBTMPFS/",0))
        {
//               printf("Saving directory does not Exists!\n");
               dir_status = mkdir("/FRBTMPFS/",0);
//               printf("dir created!\n");
        }
//        else
//        {
//               printf("Saving Directory Already Exists!\n");
//        }

        char File_dir[50];
        char t_stamp[50];
        Mcntime_t f_t;
	printf("\nOutput thread initial Done!\n");
	

	/* Main loop */
	while (run_threads()) {
		hashpipe_status_lock_safe(&st);
		hputi4(st.buf, "OUTBLKIN", block_idx);
		hputi8(st.buf, "DATSAVMB",(N_Bytes_save/1024/1024));
		hputi4(st.buf, "NFILESAV",N_files);
		hputs(st.buf, status_key, "waiting");
		hashpipe_status_unlock_safe(&st);
		// Wait for data to storage
		while ((rv=TL_output_databuf_wait_filled(db, block_idx))
		!= HASHPIPE_OK) {
		if (rv==HASHPIPE_TIMEOUT) {
			hashpipe_status_lock_safe(&st);
			hputs(st.buf, status_key, "blocked");
			hputi4(st.buf, "OUTBLKIN", block_idx);
			hashpipe_status_unlock_safe(&st);
			continue;
			} else {
				hashpipe_error(__FUNCTION__, "error waiting for filled databuf");
				pthread_exit(NULL);
				break;
			}
		}
		
		hashpipe_status_lock_safe(&st);
		hputs(st.buf, status_key, "processing");
		hputi4(st.buf, "OUTBLKIN", block_idx);
		hashpipe_status_unlock_safe(&st);
		if (filb_flag ==1 && start_file ==1 ){
		        sprintf(File_dir,"/FRBTMPFS/Beam%02d-%02d/",beam_ID[0],beam_ID[3]);
//		        sprintf(File_dir,"/data0/obs/Beam%02d-%02d/",beam_ID[0],beam_ID[3]);
		        if (access(File_dir,0))
		        {
		               dir_status = mkdir(File_dir,0755);
		        }
	//		
	//		char t_stamp[50];
	//		Mcntime_t f_t;
			if (N_files ==0)
                        	netmcnt = init_mcnt;
			f_t   = time_calculate(GPS_time,netmcnt);

//			printf("\nNetmcnt:%d\n",netmcnt);

			sprintf(t_stamp,"_%04d-%02d-%02d_%02d_%02d_%02d.fil.working",
					f_t.Year,f_t.Month,f_t.Day,f_t.Hour,f_t.Min,f_t.Sec);
                        if (data_type ==0 ){
	                        sprintf(f_fil_B0,"%sB_%02d%s" ,File_dir,beam_ID[0],t_stamp);
	                        sprintf(f_fil_B1,"%sB_%02d%s" ,File_dir,beam_ID[1],t_stamp);
				sprintf(f_fil_B2,"%sB_%02d%s" ,File_dir,beam_ID[2],t_stamp);
				sprintf(f_fil_B3,"%sB_%02d%s" ,File_dir,beam_ID[3],t_stamp);
			}
			
			WriteHeader(f_fil_B0);
			WriteHeader(f_fil_B1);
			WriteHeader(f_fil_B2);
			WriteHeader(f_fil_B3);
//			printf("write header done!\n");
	
			N_files += 1;
			TL_file_Beam_0 = fopen(f_fil_B0,"a+");
			TL_file_Beam_1 = fopen(f_fil_B1,"a+");
			TL_file_Beam_2 = fopen(f_fil_B2,"a+");
			TL_file_Beam_3 = fopen(f_fil_B3,"a+");
	
//		        printf("starting write data to %s ...\n",File_dir);
		}

                fwrite(db->block[block_idx].data.Beam_0,sizeof(db->block[block_idx].data.Beam_0),1,TL_file_Beam_0);
                fwrite(db->block[block_idx].data.Beam_1,sizeof(db->block[block_idx].data.Beam_1),1,TL_file_Beam_1);
                fwrite(db->block[block_idx].data.Beam_2,sizeof(db->block[block_idx].data.Beam_2),1,TL_file_Beam_2);
                fwrite(db->block[block_idx].data.Beam_3,sizeof(db->block[block_idx].data.Beam_3),1,TL_file_Beam_3);



		N_Bytes_save += BUFF_SIZE/N_BEAM_PER_PACK;//N_POLS_PKT;		
	
		if (TEST){

			printf("**Save Information**\n");
			printf("beam_ID:%d,%d,%d,%d \n",beam_ID[0],beam_ID[1],beam_ID[2],beam_ID[3]);
			printf("Buffsize: %lu",BUFF_SIZE);
			printf("flib_flag:%d\n",filb_flag);
			printf("Data save:%f\n",float(N_Bytes_save)/1024/1024);
			printf("Total file size:%f\n",float(N_Bytes_file)/1024/1024);
			printf("Devide:%lu\n\n",N_Bytes_save % N_Bytes_file);

			}

		if (N_Bytes_save % N_Bytes_file ==0){

			filb_flag = 1;
			
			netmcnt = db->block[block_idx].header.netmcnt - N_SPEC_PER_FILE ;	

			fclose(TL_file_Beam_0);
		        fclose(TL_file_Beam_1);
		        fclose(TL_file_Beam_2);
		        fclose(TL_file_Beam_3);

                        strncpy(Filname_B0, f_fil_B0, strlen(f_fil_B0)-8);
                        strncpy(Filname_B1, f_fil_B1, strlen(f_fil_B1)-8);
                        strncpy(Filname_B2, f_fil_B2, strlen(f_fil_B2)-8);
                        strncpy(Filname_B3, f_fil_B3, strlen(f_fil_B3)-8);
                        rename(f_fil_B0,Filname_B0);
                        rename(f_fil_B1,Filname_B1);
                        rename(f_fil_B2,Filname_B2);
                        rename(f_fil_B3,Filname_B3);

			}

		else{
			filb_flag = 0;

			}		

		TL_output_databuf_set_free(db,block_idx);
		block_idx = (block_idx + 1) % db->header.n_block;
		

		//Will exit if thread has been cancelled
		pthread_testcancel();

	}
//	fclose(TL_file_Beam_0);
//	fclose(TL_file_Beam_1);
//        fclose(TL_file_Beam_2);
//        fclose(TL_file_Beam_3);
	return THREAD_OK;
}

static hashpipe_thread_desc_t TL_output_thread = {
	name: "TL_output_thread",
	skey: "OUTSTAT",
	init: NULL, 
	run:  run,
	ibuf_desc: {TL_output_databuf_create},
	obuf_desc: {NULL}
};

static __attribute__((constructor)) void ctor()
{
	register_hashpipe_thread(&TL_output_thread);
}

