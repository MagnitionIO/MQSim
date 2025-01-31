#ifndef STATS_H
#define STATS_H

#include "SSD_Defs.h"

namespace SSD_Components
{
	class Stats
	{
	public:
		static Stats* Init_stats(unsigned int channel_no, unsigned int chip_no_per_channel, unsigned int die_no_per_chip, unsigned int plane_no_per_die, unsigned int block_no_per_plane, unsigned int page_no_per_block, unsigned int max_allowed_block_erase_count);
		void Clear_stats(unsigned int channel_no, unsigned int chip_no_per_channel, unsigned int die_no_per_chip, unsigned int plane_no_per_die, unsigned int block_no_per_plane, unsigned int page_no_per_block, unsigned int max_allowed_block_erase_count) const;
		unsigned long IssuedReadCMD, IssuedCopybackReadCMD, IssuedInterleaveReadCMD, IssuedMultiplaneReadCMD, IssuedMultiplaneCopybackReadCMD;
		unsigned long IssuedProgramCMD, IssuedInterleaveProgramCMD, IssuedMultiplaneProgramCMD, IssuedInterleaveMultiplaneProgramCMD, IssuedCopybackProgramCMD, IssuedMultiplaneCopybackProgramCMD;
		unsigned long IssuedEraseCMD, IssuedInterleaveEraseCMD, IssuedMultiplaneEraseCMD, IssuedInterleaveMultiplaneEraseCMD;

		unsigned long IssuedSuspendProgramCMD, IssuedSuspendEraseCMD;

		unsigned long Total_flash_reads_for_mapping, Total_flash_writes_for_mapping;
		unsigned long Total_flash_reads_for_mapping_per_stream[MAX_SUPPORT_STREAMS], Total_flash_writes_for_mapping_per_stream[MAX_SUPPORT_STREAMS];

		unsigned int CMT_hits, readTR_CMT_hits, writeTR_CMT_hits;
		unsigned int CMT_miss, readTR_CMT_miss, writeTR_CMT_miss;
		unsigned int total_CMT_queries, total_readTR_CMT_queries, total_writeTR_CMT_queries;
		
		unsigned int CMT_hits_per_stream[MAX_SUPPORT_STREAMS], readTR_CMT_hits_per_stream[MAX_SUPPORT_STREAMS], writeTR_CMT_hits_per_stream[MAX_SUPPORT_STREAMS];
		unsigned int CMT_miss_per_stream[MAX_SUPPORT_STREAMS], readTR_CMT_miss_per_stream[MAX_SUPPORT_STREAMS], writeTR_CMT_miss_per_stream[MAX_SUPPORT_STREAMS];
		unsigned int total_CMT_queries_per_stream[MAX_SUPPORT_STREAMS], total_readTR_CMT_queries_per_stream[MAX_SUPPORT_STREAMS], total_writeTR_CMT_queries_per_stream[MAX_SUPPORT_STREAMS];
		

		unsigned int Total_gc_executions, Total_gc_executions_per_stream[MAX_SUPPORT_STREAMS];
		unsigned int Total_page_movements_for_gc, Total_gc_page_movements_per_stream[MAX_SUPPORT_STREAMS];

		unsigned int Total_wl_executions, Total_wl_executions_per_stream[MAX_SUPPORT_STREAMS];
		unsigned int Total_page_movements_for_wl, Total_wl_page_movements_per_stream[MAX_SUPPORT_STREAMS];

		unsigned int***** Block_erase_histogram;

    private:
        Stats();
	};
}

#endif // !STATS_H
