#ifndef ADDRESS_MAPPING_UNIT_HYBRID_H
#define ADDRESS_MAPPING_UNIT_HYBRID_H

#include "Address_Mapping_Unit_Base.h"

namespace SSD_Components
{
	class Address_Mapping_Unit_Hybrid : public Address_Mapping_Unit_Base
	{
	public:
		Address_Mapping_Unit_Hybrid(sim_object_id_type id, FTL* ftl, NVM_PHY_ONFI* flash_controller, Flash_Block_Manager_Base* block_manager,
			bool ideal_mapping_table, unsigned int ConcurrentStreamNo,
			unsigned int ChannelCount, unsigned int chip_no_per_channel, unsigned int DieNoPerChip, unsigned int PlaneNoPerDie,
			unsigned int Block_no_per_plane, unsigned int Page_no_per_block, unsigned int SectorsPerPage, unsigned int PageSizeInBytes,
			double Overprovisioning_ratio, CMT_Sharing_Mode sharing_mode = CMT_Sharing_Mode::SHARED, bool fold_large_addresses = true);
		void Setup_triggers() override;
		void Start_simulation() override;
		void Validate_simulation_config() override;
		void Execute_simulator_event(MQSimEngine::Sim_Event*) override;

		void Allocate_address_for_preconditioning(const stream_id_type stream_id, std::map<LPA_type, page_status_type>& lpa_list, std::vector<double>& steady_state_distribution) override;
		int Bring_to_CMT_for_preconditioning(stream_id_type stream_id, LPA_type lpa) override;
		unsigned int Get_cmt_capacity() override;
		unsigned int Get_current_cmt_occupancy_for_stream(stream_id_type stream_id) override;
		void Translate_lpa_to_ppa_and_dispatch(const std::list<NVM_Transaction*>& transactionList) override;
		void Get_data_mapping_info_for_gc(const stream_id_type stream_id, const LPA_type lpa, PPA_type& ppa, page_status_type& page_state) override;
		void Get_translation_mapping_info_for_gc(const stream_id_type stream_id, const MVPN_type mvpn, MPPN_type& mppa, sim_time_type& timestamp) override;
		void Allocate_new_page_for_gc(NVM_Transaction_Flash_WR* transaction, bool is_translation_page) override;

		void Store_mapping_table_on_flash_at_start() override;
		LPA_type Get_logical_pages_count(stream_id_type stream_id) override;
		NVM::FlashMemory::Physical_Page_Address Convert_ppa_to_address(const PPA_type ppa) override;
		void Convert_ppa_to_address(const PPA_type ppn, NVM::FlashMemory::Physical_Page_Address& address) override;
		PPA_type Convert_address_to_ppa(const NVM::FlashMemory::Physical_Page_Address& pageAddress) override;

		void Set_barrier_for_accessing_physical_block(const NVM::FlashMemory::Physical_Page_Address& block_address) override;
		void Set_barrier_for_accessing_lpa(stream_id_type stream_id, LPA_type lpa) override;
		void Set_barrier_for_accessing_mvpn(stream_id_type stream_id, MVPN_type mpvn) override;
		void Remove_barrier_for_accessing_lpa(stream_id_type stream_id, LPA_type lpa) override;
		void Remove_barrier_for_accessing_mvpn(stream_id_type stream_id, MVPN_type mpvn) override;
		void Start_servicing_writes_for_overfull_plane(const NVM::FlashMemory::Physical_Page_Address plane_address) override;
	private:
		bool query_cmt(NVM_Transaction_Flash* transaction) override;
		PPA_type online_create_entry_for_reads(LPA_type lpa, const stream_id_type stream_id, NVM::FlashMemory::Physical_Page_Address& read_address, uint64_t read_sectors_bitmap) override;
		void manage_user_transaction_facing_barrier(NVM_Transaction_Flash* transaction) override;
		void manage_mapping_transaction_facing_barrier(stream_id_type stream_id, MVPN_type mvpn, bool read) override;
		bool is_lpa_locked_for_gc(stream_id_type stream_id, LPA_type lpa) override;
		bool is_mvpn_locked_for_gc(stream_id_type stream_id, MVPN_type mvpn) override;
	};
}

#endif // !ADDRESS_MAPPING_UNIT_HYBRID_H
