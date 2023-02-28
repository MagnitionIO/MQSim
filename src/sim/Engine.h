#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <unordered_map>
#include <set>
#include "Sim_Defs.h"
#include "EventTree.h"
#include "Sim_Object.h"
#include "lib/base.h"
#include "Stats.h"
#include "Logical_Address_Partitioning_Unit.h"

namespace MQSimEngine {
	class Engine
	{
		friend class EventTree;
	public:
        Engine()
		{
			this->_EventList = new EventTree;
			started = false;
//            std::freopen("flowTrace.txt", "w", stderr);
		}

		~Engine() {
			delete _EventList;
		}
		
		sim_time_type Time();
		Sim_Event* Register_sim_event(sim_time_type fireTime, Sim_Object* targetObject, void* parameters = NULL, int type = 0);
		void Ignore_sim_event(Sim_Event*);
		void Reset();
		void AddObject(Sim_Object* obj);
		Sim_Object* GetObject(sim_object_id_type object_id);
		void RemoveObject(Sim_Object* obj);
		void Start_simulation();
        void* StartSimulator(void* args){return nullptr;}
        static void* StartServicingThread(void *args){return nullptr;}
        void Stop_simulation();
        bool Has_started();
        bool Is_integrated_execution_mode();
        bool process_request(void *req, uint64_t *required_time, uint64_t curr_time, insert_to_queue_f queue);
        void initialize();
        void report_request_complete(completion_info_t *info);
        void enqueue_completed_request_ids(insert_to_queue_f insert);

        SSD_Components::Stats *stats;
        Utils::Logical_Address_Partitioning_Unit *lapu;

    private:
		sim_time_type _sim_time = 0;
		EventTree* _EventList{};
		std::unordered_map<sim_object_id_type, Sim_Object*> _ObjectList{};
        std::set<completion_info_t*> done_queue{};

        bool stop{};
		bool started;
	};
}

//#define Simulator MQSimEngine::Engine::Instance()

EXTERN_C MQSimEngine::Engine* getEngineInstance();


EXTERN_C MQSimEngine::Sim_Object* getIntegrationFlowInstance();

EXTERN_C void* getDataStoreInstance();

#define Simulator getEngineInstance()

#endif // !ENGINE_H
