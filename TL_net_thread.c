/*
 * TL_net_thread.c
 * Add misspkt correct mechanism.
 *  
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include "hashpipe.h"
#include "TL_databuf.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <netdb.h>
#include <sys/ioctl.h>

static struct hashpipe_udp_params params;
//printf("Host:%s\n",params.bindhost);
//#include "TL_net_thread.h"
//unsigned long long miss_pkt = 0;
//
int  beam_ID[4];
bool data_type=0;
bool start_file=0;
static int total_packets_counted = 0;
uint64_t init_mcnt=0;

static hashpipe_status_t *st_p;
static  const char * status_key;
static int init(hashpipe_thread_args_t * args)
{
        hashpipe_status_t st = args->st;
        hashpipe_status_lock_safe(&st);
	hputs(st.buf, "BACkEND", "TianLai-FRB");
	hputs(st.buf, "Author", "Chenhui Niu");
	hputi8(st.buf,"BUFMCNT",0);
        hputi8(st.buf, "NPACKETS", 0);
        hputi8(st.buf, "DATSAVMB",0);
	hputi8(st.buf,"MiSSPKT",0);
        hashpipe_status_unlock_safe(&st);
        return 0;

}

typedef struct {
    uint64_t    mcnt;           // counter for packet
    bool        source_from;    // 0 - Integrated, 1 - Without Integrated
    int         BeamGroup_ID;   // beam ID
} packet_header_t;


typedef struct {
    uint64_t 	cur_mcnt;
    long 	miss_pkt;
    long   	offset;
    int         initialized;
    int		block_idx;
    bool	start_flag;
} block_info_t;

static block_info_t binfo;
// This function must be called once and only once per block_info structure!
// Subsequent calls are no-ops.
static inline void initialize_block_info(block_info_t * binfo)
{

    // If this block_info structure has already been initialized
    if(binfo->initialized) {
        return;
    }

    binfo->cur_mcnt	= 0;
    binfo->block_idx	= 0;
    binfo->start_flag	= 0;
    binfo->offset	= 0;
    binfo->miss_pkt	= 0;
    binfo->initialized	= 1;
}


static inline void get_header( packet_header_t * pkt_header, char *packet)
{
    uint64_t raw_header;
//    raw_header = le64toh(*(unsigned long long *)p->data);
    memcpy(&raw_header,packet,N_BYTES_HEADER*sizeof(char));
    raw_header = be64toh(raw_header);
    pkt_header->mcnt        	= raw_header  & 0x00ffffffffffffff; 
    pkt_header->source_from	= (raw_header  & 0x8000000000000000)>>63; // 0 - Integrated, 1 - Without Integration
    pkt_header->BeamGroup_ID	= (raw_header  & 0x7f00000000000000)>>56; // 7 bits, only 4 values for beam ID. 
									  // BeamGroup_ID is in [0,1,2,3].
									  // 0:beam[0-3] , 1:beam[4-7] , 2:beam[8-11] , 3:beam[12:15] 
    if (TEST){
	    fprintf(stderr,"**Header**\n ");
	    fprintf(stderr,"Mcnt of Header is :%lu \n ",pkt_header->mcnt);
	    fprintf(stderr,"Source from?(0:Integrated,1:Without Integrated) [%lu] \n ",pkt_header->source_from);
	    fprintf(stderr,"Beam ID :%lu \n ",pkt_header->BeamGroup_ID);
	}
}

static inline void miss_pkt_process( uint64_t pkt_mcnt, TL_input_databuf_t *db) 
{
    binfo.miss_pkt	+= (pkt_mcnt - binfo.cur_mcnt);
    long  miss_pkt       =  pkt_mcnt - binfo.cur_mcnt;
    uint64_t miss_size   =  miss_pkt * DATA_SIZE_PACK;
    int rv;

    if (((binfo.offset + miss_size ) >= BUFF_SIZE) && (miss_size < BUFF_SIZE)){

        while (( rv = TL_input_databuf_wait_free(db, binfo.block_idx))!= HASHPIPE_OK) {
              if (rv==HASHPIPE_TIMEOUT) {
                  hashpipe_status_lock_safe(st_p);
                  hputs(st_p->buf, status_key, "blocked");
                  hashpipe_status_unlock_safe(st_p);
                  continue;
               } else {
                   hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                   pthread_exit(NULL);
                   break;
              }
        }

        memset(db->block[binfo.block_idx].data+binfo.offset,0,(BUFF_SIZE - binfo.offset)*sizeof(char));
        binfo.offset  = binfo.offset + miss_size - BUFF_SIZE;//Give new offset after 1 buffer zero.
	db->block[binfo.block_idx].header.netmcnt = pkt_mcnt;
        // Mark block as full
        if(TL_input_databuf_set_filled(db, binfo.block_idx) != HASHPIPE_OK) {
            hashpipe_error(__FUNCTION__, "error waiting for databuf filled call");
            pthread_exit(NULL);}

        binfo.block_idx = (binfo.block_idx + 1) % db->header.n_block;

        while ((rv = TL_input_databuf_wait_free(db, binfo.block_idx))!= HASHPIPE_OK) {
              if (rv==HASHPIPE_TIMEOUT) {
                  hashpipe_status_lock_safe(st_p);
                  hputs(st_p->buf, status_key, "blocked");
                  hashpipe_status_unlock_safe(st_p);
                  continue;
               } else {
                   hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                   pthread_exit(NULL);
                   break;
              }
          }
        memset(db->block[binfo.block_idx].data,0,(binfo.offset)*sizeof(char));
     }
    
    else if(miss_size > BUFF_SIZE){
		 printf("SYSTEM mcnt:%lu \n ", binfo.cur_mcnt);
                 printf("Packet mcnt:%lu \n ", pkt_mcnt);
		 printf("Miss_size:%lu \n ",miss_size);
		 printf("BUFF_SIZE: %lu \n ",BUFF_SIZE);
		 fprintf(stderr,"Missing Pkt much more than one Buffer...\n");
                 pthread_exit(NULL);
		 exit(1);
		 }
    else{
	   if(TEST){
	         printf("**Miss packet! hooo no!**\n ");
                 printf("binfo mcnt:%lu \n ", binfo.cur_mcnt);
                 printf("Packet mcnt:%lu \n\n", pkt_mcnt);
		   }
           while (( rv = TL_input_databuf_wait_free(db, binfo.block_idx))!= HASHPIPE_OK) {
                 if (rv==HASHPIPE_TIMEOUT) {
                     hashpipe_status_lock_safe(st_p);
                     hputs(st_p->buf, status_key, "blocked");
                     hashpipe_status_unlock_safe(st_p);
                     continue;
                  } else {
                      hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                      pthread_exit(NULL);
                      break;
                 }
             }
           memset(db->block[binfo.block_idx].data,0,(miss_size)*sizeof(char));

	}
	binfo.cur_mcnt = pkt_mcnt + N_PACKETS_PER_SPEC - pkt_mcnt % N_PACKETS_PER_SPEC;
	
}


static inline void process_packet(TL_input_databuf_t *db,char *packet)
{

    packet_header_t pkt_header;	
    uint64_t pkt_mcnt	= 0;
    int seq             = 0;
    int pkt_beamID	= 0;
    int rv		= 0;

    // Parse packet header
    get_header(&pkt_header,packet);
    pkt_mcnt	= pkt_header.mcnt;
    data_type	= pkt_header.source_from;
    pkt_beamID	= pkt_header.BeamGroup_ID;
//    start_file  = 1;
    // Copy Header Information
    int beamgroup_ID = pkt_beamID;
    for(int j=0; j<N_BEAM_PER_PACK; j++)
	{
  	  beam_ID[j] = beamgroup_ID * N_BEAM_PER_PACK + j;
    	}
    if(TEST){
	    fprintf(stderr,"**Before start**\n ");
	    fprintf(stderr,"cur_mcnt: %lu \n ",binfo.cur_mcnt);
	    fprintf(stderr,"pkt_mcnt: %lu \n ",pkt_mcnt);	
	    fprintf(stderr,"start flag :%d \n\n ",binfo.start_flag);
	    }
//    binfo.start_flag=1;
//    if (binfo.start_flag )
//    {
	if(TEST){printf("\n ********start !!!******\n\n");}
        if (total_packets_counted == 0 )
        {
		binfo.cur_mcnt = pkt_mcnt;
		init_mcnt  = pkt_mcnt;
		start_file  = 1;
	}
        total_packets_counted++;


        if(binfo.cur_mcnt == pkt_mcnt){

	    while (( rv = TL_input_databuf_wait_free(db, binfo.block_idx))!= HASHPIPE_OK) 
            {
                   if (rv==HASHPIPE_TIMEOUT)
                   {
                       hashpipe_status_lock_safe(st_p);
                       hputs(st_p->buf, status_key, "blocked");
                       hashpipe_status_unlock_safe(st_p);
//                       continue;
                   } 
                   else
                   {
                        hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                        pthread_exit(NULL);
                        break;
                   }
            }



            // Copy data into buffer
            memcpy((db->block[binfo.block_idx].data)+binfo.offset, packet+8, DATA_SIZE_PACK*sizeof(char));
	    // Show Status of buffer
            hashpipe_status_lock_safe(st_p);
            hputi8(st_p->buf,"MiSSPKT",binfo.miss_pkt);
	    hputi8(st_p->buf,"DTYPE",data_type);
            hashpipe_status_unlock_safe(st_p);

            binfo.offset     += DATA_SIZE_PACK;
            binfo.start_flag  = 1;
            binfo.cur_mcnt   += 1;

            if (binfo.offset == BUFF_SIZE){
	            if(TEST){fprintf(stderr,"\nOffset already buffsize!: %lu \n",binfo.offset);}
		    db->block[binfo.block_idx].header.netmcnt = pkt_mcnt;
	            // Mark block as full
		    if(TL_input_databuf_set_filled(db, binfo.block_idx) != HASHPIPE_OK) {
	        	      hashpipe_error(__FUNCTION__, "error waiting for databuf filled call");
        	    	      pthread_exit(NULL);
              }
		    
	            binfo.block_idx = (binfo.block_idx + 1) % db->header.n_block;
	            binfo.offset = 0;
        	    binfo.start_flag = 0;
	    
            }
        }//if (binfo.cur_mcnt == pkt_mcnt)


	else{
	    
	            miss_pkt_process(pkt_mcnt, db);
        	    binfo.start_flag = 0;
            }

//    }//(seq == 0 || binfo.start_flag )

if(TEST){printf("\n ********End !!!******\n\n");}
}




static void *run(hashpipe_thread_args_t * args)
{
    printf("\nRunning...\n");
    TL_input_databuf_t *db  = (TL_input_databuf_t *)args->obuf;
    if(!binfo.initialized) {
        initialize_block_info(&binfo);
        db->block[binfo.block_idx].header.netmcnt=0;
	printf("\nInitailized!\n");
    }

    hashpipe_status_t st = args->st;
    status_key = args->thread_desc->skey;
    st_p = &st; // allow global (this source file) access to the status buffer
    hgets(st.buf, "BINDHOST", 80, params.bindhost);
    int rv;
    
    sleep(1);   		


    hashpipe_status_lock_safe(&st);
    hputi4(st.buf, "BINDPORT", 10000);
    hputs(st.buf, status_key, "running");
    hashpipe_status_unlock_safe(&st);




    /* Give all the threads a chance to start before opening network socket */
    sleep(3);




	/*Check first two block */
        while ((rv=TL_input_databuf_wait_free(db, 0))
                != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                pthread_exit(NULL);
                break;
            }
        }


        while ((rv=TL_input_databuf_wait_free(db, 1))
                != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                pthread_exit(NULL);
                break;
            }
        }



    /* Main loop */

    /*Create UDP bind from MAC level*/
    //****************************************************************//

    struct sockaddr_ll sll;
    struct ifreq ifr;
    char MAC5;
    int raw_sock = socket(AF_PACKET, SOCK_RAW, htons(0x3));//ETH_P_ALL);
    bzero(&sll, sizeof(sll));
    bzero(&ifr, sizeof(ifr));
    strncpy((char *)ifr.ifr_name, params.bindhost, IFNAMSIZ);
    ioctl(raw_sock, SIOCGIFINDEX, &ifr);
    sll.sll_family   = AF_PACKET;
    sll.sll_protocol = htons(0x3);
    sll.sll_ifindex  = ifr.ifr_ifindex;
    bind(raw_sock, (struct sockaddr *) &sll, sizeof(sll)); 
    if (strcmp(params.bindhost,"p1p1")==0)
	MAC5 = 0x11;
    else if (strcmp(params.bindhost, "p1p2")==0)
	MAC5 = 0x12;
    else if (strcmp(params.bindhost, "p4p1")==0)
	MAC5 = 0x13;
    else if (strcmp(params.bindhost, "p4p2")==0)
	MAC5 = 0x14;
    //****************************************************************//



    int pkt_size;
    char * packet   = (char *)malloc((PKTSIZELG)*sizeof(char));

    while (run_threads()) {
        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "running");
        hputi4(st.buf, "NETBKOUT", binfo.block_idx);
        hputi8(st.buf, "NPACKETS", total_packets_counted);
        hashpipe_status_unlock_safe(&st);

	
	pkt_size = recv(raw_sock, packet, (PKTSIZELG)*sizeof(char), 0);
	
	if (pkt_size == PKTSIZELG && packet[5] == MAC5 ){
		 process_packet((TL_input_databuf_t *)db,packet+42);
	}
	
/*
	else if (pkt_size == -1){
        hashpipe_error("paper_net_thread",
                       "hashpipe_udp_recv returned error");
        perror("hashpipe_udp_recv");
        pthread_exit(NULL);
	}
*/
        pthread_testcancel();

     }// Main loop

     return THREAD_OK;
}

static hashpipe_thread_desc_t TL_net_thread = {
    name: "TL_net_thread",
    skey: "NETSTAT",
    init: init,
    run:  run,
    ibuf_desc: {NULL},
    obuf_desc: {TL_input_databuf_create}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&TL_net_thread);
}
