/*Created at [7/11/2024 11:57:52 AM] by [KDot].*/

#ifndef _FlowControl_H_
#define _FlowControl_H_

namespace Andromeda
{
	namespace Flow
	{
		typedef void(*IFlowCall)(const void *const cpcInput, void *const pcOutput);

		class IFlowControl
		{
		public:

			virtual void AddControl(IFlowCall callback) = 0;
			virtual void Execute(void *const pcOutput) = 0;
		};

		__declspec(dllexport) long CreateFlowControl(const unsigned long dataSize, IFlowControl **const ppcOut);
	}
}

#endif