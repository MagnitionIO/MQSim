#include "NVM_PHY_ONFI.h"

namespace SSD_Components {
	void NVM_PHY_ONFI::ConnectToTransactionServicedSignal(Sim_Object *instance, TransactionServicedHandlerType function)
	{
		connectedTransactionServicedHandlers.emplace_back(instance, function);
	}

	/*
	* Different FTL components maybe waiting for a transaction to be finished:
	* HostInterface: For user reads and writes
	* Address_Mapping_Unit: For mapping reads and writes
	* TSU: For the reads that must be finished for partial writes (first read non updated parts of page data and then merge and write them into the new page)
	* GarbageCollector: For gc reads, writes, and erases
	*/
	void NVM_PHY_ONFI::broadcastTransactionServicedSignal(NVM_Transaction_Flash* transaction)
	{
		for (const auto &it : connectedTransactionServicedHandlers) {
			it.second(it.first, transaction);
		}
		delete transaction;//This transaction has been consumed and no more needed
	}

	void NVM_PHY_ONFI::ConnectToChannelIdleSignal(MQSimEngine::Sim_Object *instance, ChannelIdleHandlerType function)
	{
		connectedChannelIdleHandlers.emplace_back(instance, function);
	}

	void NVM_PHY_ONFI::broadcastChannelIdleSignal(flash_channel_ID_type channelID)
	{
		for (const auto &it : connectedChannelIdleHandlers) {
			it.second(it.first, channelID);
		}
	}

	void NVM_PHY_ONFI::ConnectToChipIdleSignal(MQSimEngine::Sim_Object *instance, ChipIdleHandlerType function)
	{
		connectedChipIdleHandlers.emplace_back(instance, function);
	}

	void NVM_PHY_ONFI::broadcastChipIdleSignal(NVM::FlashMemory::Flash_Chip* chip)
	{
		for (const auto & it : connectedChipIdleHandlers) {
			it.second(it.first, chip);
		}
	}
}