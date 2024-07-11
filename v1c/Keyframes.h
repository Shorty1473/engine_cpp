/*Created at [7/10/2024 8:55:39 PM] by [KDot].*/

#ifndef _Keyframes_H_
#define _Keyframes_H_

namespace Andromeda
{
	namespace Animation
	{
		enum class ANIMATOR_STATE
		{
			NONE,
			INIT,
			PLAY,
			PAUSE,
			STOP,
			DONE
		};

		struct KEY_FRAME
		{
			float FrameTime;
			long FrameID;
			void *Data;
		};

		typedef void(*IAnimatorStep)(const KEY_FRAME *const pcA, const KEY_FRAME *const pcB, void *const pcOut, const double step);

		class IAnimator
		{
		public:

			virtual void CreateFrame(KEY_FRAME **const ppcOut) = 0;
			virtual void GetFrame(const unsigned long frameID, KEY_FRAME *const pcOut) = 0;
			virtual void RemoveFrame(const unsigned long frameID) = 0;

			/*
			If called after Stop() -> Starts at beginning
			If called after Pause() -> Starts at pause point
			If called while (state == DONE), starts at beginning
			*/
			virtual void Start() = 0;

			virtual void Pause() = 0;
			virtual void Stop() = 0;

			virtual void Step(const double deltaTime, void *const pcDest) = 0;

			virtual ANIMATOR_STATE GetState() = 0;
		};

		class IKeyframes
		{
		public:

			virtual long GetAnimator(const unsigned long dataSize, IAnimator **const ppcOut, IAnimatorStep lerpFunc) = 0;
		};

		__declspec(dllexport) long CreateKeyframes(IKeyframes **const ppcOut);
	}
}

#endif