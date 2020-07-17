#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "hashpipe.h"
#include "hashpipe_databuf.h"


#define CACHE_ALIGNMENT         64
#define N_INPUT_BLOCKS          3 
#define N_OUTPUT_BLOCKS         3
#define T_RSL			150			//us
#define TEST			0

#define N_BEAM			16

#define N_BEAM_PER_PACK		4			//Number of Beams for one packet
#define N_CHAN_PER_PACK		1024			//Number of channels for 1 packet and 1 pole
#define N_PACKETS_PER_SPEC	1			//Number of packets for one spectrum
							//Number of channels for one spectrum
#define N_BYTES_DATA_POINT	1			//Number of bytes per datapoint
#define N_POLS_PKT		1			//Number of polarizations per packet
#define N_BYTES_HEADER		8			//Number of Bytes of header
#define N_SPEC_BUFF             256*4			//Number of spectrums per buffer
#define N_BITS_DATA_POINT       N_BYTES_DATA_POINT*8 	//Number of bits per datapoint in packet
#define N_CHANS_SPEC		(N_CHAN_PER_PACK * N_PACKETS_PER_SPEC) 					//Channels in spectrum for 1 beam.
#define DATA_SIZE_PACK		(unsigned long)(N_CHAN_PER_PACK * N_POLS_PKT * N_BEAM_PER_PACK *  N_BYTES_DATA_POINT) 	//Packet size without Header 
#define PKTSIZE			(DATA_SIZE_PACK + N_BYTES_HEADER)					//Total Packet size 
#define PKTSIZELG		(DATA_SIZE_PACK + N_BYTES_HEADER+42)					//Total Packet size with MAC level

#define N_BYTES_PER_SPEC	(DATA_SIZE_PACK/N_BEAM_PER_PACK*N_PACKETS_PER_SPEC)			//Spectrum siz with 1 beam
#define BUFF_SIZE		(unsigned long)(N_SPEC_BUFF*N_BYTES_PER_SPEC*N_BEAM_PER_PACK) 		//Buffer size with 4 beams
#define N_CHANS_BUFF		(N_SPEC_BUFF*N_CHANS_SPEC)     						//Channels in one buffer with 1 beam
#define N_SPEC_PER_FILE	 	10240*4*2*10									// Number of spectrums per file \
				int{time(s)/T_samp(s)/N_SPEC_BUFF}*N_SPEC_BUFF  e.g. 20s data: int(20/0.001/128)*128
#define N_BYTES_PER_FILE	(N_SPEC_PER_FILE * N_BYTES_PER_SPEC / N_POLS_PKT)	// we can save (I,Q,U,V) polaration into disk. 


// Used to pad after hashpipe_databuf_t to maintain cache alignment
typedef uint8_t hashpipe_databuf_cache_alignment[
  CACHE_ALIGNMENT - (sizeof(hashpipe_databuf_t)%CACHE_ALIGNMENT)
];

//Define BeamGroup Data structure
typedef struct BeamGroup_data {

   uint8_t Beam_0[N_CHANS_BUFF];
   uint8_t Beam_1[N_CHANS_BUFF];
   uint8_t Beam_2[N_CHANS_BUFF];
   uint8_t Beam_3[N_CHANS_BUFF];

}BeamGroup_data_t;

typedef struct {
        int Year;
        int Month;
        int Day;
        int Hour;
        int Min;
        int Sec;
        int Msec;
        int Usec;
} Mcntime_t;

//static Mcntime_t File_time;

/* INPUT BUFFER STRUCTURES*/
typedef struct TL_input_block_header {
   uint64_t	netmcnt;        // Counter for ring buffer
   		
} TL_input_block_header_t;

typedef uint8_t TL_input_header_cache_alignment[
   CACHE_ALIGNMENT - (sizeof(TL_input_block_header_t)%CACHE_ALIGNMENT)
];

typedef struct TL_input_block {

   TL_input_block_header_t header;
   TL_input_header_cache_alignment padding; // Maintain cache alignment
//   uint8_t  data[N_CHANS_BUFF * N_POLS_PKT]; //Input buffer for all channels
   uint8_t  data[N_CHANS_BUFF * 4]; //Input buffer for all channels

} TL_input_block_t;

typedef struct TL_input_databuf {
   hashpipe_databuf_t header;
   hashpipe_databuf_cache_alignment padding; // Maintain cache alignment
   TL_input_block_t block[N_INPUT_BLOCKS];
} TL_input_databuf_t;


/*
  * OUTPUT BUFFER STRUCTURES
  */
typedef struct TL_output_block_header {
	 uint64_t     netmcnt;        // Counter for ring buffer

} TL_output_block_header_t;

typedef uint8_t TL_output_header_cache_alignment[
   CACHE_ALIGNMENT - (sizeof(TL_output_block_header_t)%CACHE_ALIGNMENT)
];

typedef struct TL_output_block {

   TL_output_block_header_t header;
   TL_output_header_cache_alignment padding; // Maintain cache alignment
   BeamGroup_data_t data;

} TL_output_block_t;

typedef struct TL_output_databuf {
   hashpipe_databuf_t header;
   hashpipe_databuf_cache_alignment padding; // Maintain cache alignment
   TL_output_block_t block[N_OUTPUT_BLOCKS];
} TL_output_databuf_t;

/*
 * INPUT BUFFER FUNCTIONS
 */
hashpipe_databuf_t *TL_input_databuf_create(int instance_id, int databuf_id);

static inline TL_input_databuf_t *TL_input_databuf_attach(int instance_id, int databuf_id)
{
    return (TL_input_databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int TL_input_databuf_detach(TL_input_databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline void TL_input_databuf_clear(TL_input_databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline int TL_input_databuf_block_status(TL_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_input_databuf_total_status(TL_input_databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

static inline int TL_input_databuf_wait_free(TL_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_input_databuf_busywait_free(TL_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_input_databuf_wait_filled(TL_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_input_databuf_busywait_filled(TL_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_input_databuf_set_free(TL_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_input_databuf_set_filled(TL_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}

/*
 * OUTPUT BUFFER FUNCTIONS
 */

hashpipe_databuf_t *TL_output_databuf_create(int instance_id, int databuf_id);

static inline void TL_output_databuf_clear(TL_output_databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline TL_output_databuf_t *TL_output_databuf_attach(int instance_id, int databuf_id)
{
    return (TL_output_databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int TL_output_databuf_detach(TL_output_databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline int TL_output_databuf_block_status(TL_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_output_databuf_total_status(TL_output_databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

static inline int TL_output_databuf_wait_free(TL_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_output_databuf_busywait_free(TL_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_free((hashpipe_databuf_t *)d, block_id);
}
static inline int TL_output_databuf_wait_filled(TL_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_output_databuf_busywait_filled(TL_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_output_databuf_set_free(TL_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

static inline int TL_output_databuf_set_filled(TL_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}


