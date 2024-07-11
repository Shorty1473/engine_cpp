/*Created at [7/11/2024 11:57:52 AM] by [KDot].*/
#include "FlowControl.h"

#include <malloc.h>
#include <memory.h>

namespace flow_vars
{
	const unsigned long MAX_ARRAY_SIZE = 8;
}

namespace an_v1_impl
{
	struct FLOW_DATA
	{
		Andromeda::Flow::IFlowCall FuncArray[flow_vars::MAX_ARRAY_SIZE];

		FLOW_DATA *Next;
	};

	void init_FLOW_DATA(FLOW_DATA *const pc)
	{
		for (unsigned long i = 0; i < flow_vars::MAX_ARRAY_SIZE; ++i)
			pc->FuncArray[i] = nullptr;

		pc->Next = nullptr;
	}

	class flowControl :public Andromeda::Flow::IFlowControl
	{
	private:

		unsigned long DataSize_;
		void *Data_;

		FLOW_DATA *Functions_ = nullptr;

	public:

		flowControl();
		~flowControl();

		void init(const unsigned long dataSize);

	public:

		virtual void AddControl(Andromeda::Flow::IFlowCall callback);
		virtual void Execute(void *const pcOutput);
	};

	flowControl::flowControl()
	{

	}

	flowControl::~flowControl()
	{
	}

	void flowControl::init(const unsigned long dataSize)
	{
		if (Functions_ == nullptr)
		{
			Functions_ = new FLOW_DATA;
			init_FLOW_DATA(Functions_);
		}

		DataSize_ = dataSize;
		Data_ = malloc(DataSize_);
	}

	void flowControl::AddControl(Andromeda::Flow::IFlowCall callback)
	{
		FLOW_DATA *temp = Functions_;

		while (temp != nullptr)
		{
			for (unsigned long i = 0; i < flow_vars::MAX_ARRAY_SIZE; ++i)
			{
				if (temp->FuncArray[i] == nullptr)
				{
					temp->FuncArray[i] = callback;
					return;
				}
			}

			if (temp->Next == nullptr)
			{
				temp->Next = new FLOW_DATA;
				init_FLOW_DATA(temp->Next);
			}

			temp = temp->Next;
		}
	}
	
	void flowControl::Execute(void *const pcOutput)
	{
		FLOW_DATA *temp = Functions_;

		while (temp != nullptr)
		{
			for (unsigned long i = 0; i < flow_vars::MAX_ARRAY_SIZE; ++i)
			{
				if (temp->FuncArray[i] == nullptr)
					continue;

				temp->FuncArray[i](Data_, Data_);
			}

			temp = temp->Next;
		}

		if (pcOutput) memcpy(pcOutput, Data_, DataSize_);
	}
}

namespace flow_vars
{
	struct FlowDataArray
	{
		char InUse[MAX_ARRAY_SIZE];
		an_v1_impl::flowControl Array[MAX_ARRAY_SIZE];

		FlowDataArray *Next;

	};

	void init_DataArray(FlowDataArray *const pcIn)
	{
		for (unsigned long i = 0; i < MAX_ARRAY_SIZE; ++i)
			(*pcIn).InUse[i] = 0;

		(*pcIn).Next = nullptr;
	}

	FlowDataArray *FlowList = nullptr;
}

namespace Andromeda
{
	namespace Flow
	{
		long CreateFlowControl(const unsigned long dataSize, IFlowControl **const ppcOut)
		{
			if (flow_vars::FlowList == nullptr)
			{
				flow_vars::FlowList = new flow_vars::FlowDataArray;
				flow_vars::init_DataArray(flow_vars::FlowList);
			}

			flow_vars::FlowDataArray *temp = flow_vars::FlowList;
			while (temp != nullptr)
			{
				for (unsigned long i = 0; i < flow_vars::MAX_ARRAY_SIZE; ++i)
				{
					if (temp->InUse[i] == 0)
					{
						temp->InUse[i] = 1;
						temp->Array[i].init(dataSize);

						(*ppcOut) = &temp->Array[i];
						return 0;
					}
				}

				if (temp->Next == nullptr)
				{
					temp->Next = new flow_vars::FlowDataArray;
					flow_vars::init_DataArray(temp->Next);
				}

				temp = temp->Next;
			}

			return 0;
		}
	}
}