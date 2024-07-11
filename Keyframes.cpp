/*Created at [7/10/2024 8:55:39 PM] by [KDot].*/
#include "Keyframes.h"

#include <malloc.h>
#include <memory.h>

#include <stdio.h>

namespace an_v1_impl
{
	namespace vars
	{
		const unsigned long MAX_ANIMATORS = 8;
	}
}

namespace an_v1_impl
{
	struct KEY_FRAME_STATE
	{
		float Timer;

		Andromeda::Animation::KEY_FRAME Frame;

		KEY_FRAME_STATE *Next;
	};

	void init_KEY_FRAME_STATE(KEY_FRAME_STATE *const pcState, const unsigned long frameID, const unsigned long dataSize)
	{
		(*pcState).Frame.Data = malloc(dataSize);
		(*pcState).Frame.FrameID = frameID;

		(*pcState).Timer = 0;
		(*pcState).Next = nullptr;
	}

	void reset_KEY_FRAME_STATE(KEY_FRAME_STATE *const pcState)
	{
		(*pcState).Timer = 0;
	}

	class animator :public Andromeda::Animation::IKeyframes::IAnimator
	{
	private:

		unsigned long DataSize_;

		long Step_;
		unsigned long StepCount_;

		Andromeda::Animation::ANIMATOR_STATE State_;

		KEY_FRAME_STATE *Frames_;
		KEY_FRAME_STATE *Current_;

		Andromeda::Animation::IAnimatorStep LerpCall_;

		void countSteps();
		void advanceFrame();

	public:

		animator();
		~animator();

		void init();
		void setup(const unsigned long dataSize, Andromeda::Animation::IAnimatorStep lerpFunc);

	public:

		virtual void CreateFrame(Andromeda::Animation::KEY_FRAME **const ppcOut);
		virtual void GetFrame(const unsigned long frameID, Andromeda::Animation::KEY_FRAME *const pcOut);
		virtual void RemoveFrame(const unsigned long frameID);

		virtual void Start();

		virtual void Pause();
		virtual void Stop();

		virtual void Step(const double deltaTime, void *const pcDest);

		virtual Andromeda::Animation::ANIMATOR_STATE GetState();
	};

	void animator::countSteps()
	{
		StepCount_ = 0;
		KEY_FRAME_STATE *temp = Frames_;
		while (temp != nullptr)
		{
			++StepCount_;
			temp = temp->Next;
		}
	}

	void animator::advanceFrame()
	{
		++Step_;
		Current_ = Current_->Next;

		if (Current_ != nullptr)
			reset_KEY_FRAME_STATE(Current_);
	}

	animator::animator()
	{
		Step_ = -1;
		StepCount_ = 0;
		DataSize_ = 0;
		State_ = Andromeda::Animation::ANIMATOR_STATE::NONE;
	}

	animator::~animator()
	{

	}
	
	void animator::init()
	{
		Step_ = 0;
		StepCount_ = 0;
		DataSize_ = 0;
		State_ = Andromeda::Animation::ANIMATOR_STATE::INIT;

		Frames_ = nullptr;
	}

	void animator::setup(const unsigned long dataSize, Andromeda::Animation::IAnimatorStep lerpFunc)
	{
		DataSize_ = dataSize;
		LerpCall_ = lerpFunc;
	}

	void animator::CreateFrame(Andromeda::Animation::KEY_FRAME **const ppcOut)
	{
		if (!ppcOut)
			return;

		if (Frames_ == nullptr)
		{
			StepCount_ = 0;

			Frames_ = new KEY_FRAME_STATE;
			init_KEY_FRAME_STATE(Frames_, StepCount_++, DataSize_);

			(*ppcOut) = &Frames_->Frame;

			return;
		}

		KEY_FRAME_STATE *temp = Frames_;
		while (temp->Next != nullptr)
			temp = temp->Next;

		temp->Next = new KEY_FRAME_STATE;
		init_KEY_FRAME_STATE(temp->Next, StepCount_, DataSize_);

		(*ppcOut) = &temp->Next->Frame;
	}

	void animator::GetFrame(const unsigned long frameID, Andromeda::Animation::KEY_FRAME *const pcOut)
	{
		if (!pcOut)
			return;

		unsigned long counter = 0;
		KEY_FRAME_STATE *temp = Frames_;

		while (temp != nullptr)
		{
			if (counter == frameID)
			{
				(*pcOut).Data = temp->Frame.Data;
				(*pcOut).FrameID = temp->Frame.FrameID;

				return;
			}

			++counter;
			temp = temp->Next;
		}
	}

	void animator::RemoveFrame(const unsigned long frameID)
	{
	}

	void animator::Start()
	{
		if (State_ == Andromeda::Animation::ANIMATOR_STATE::INIT)
		{
			countSteps();
			State_ = Andromeda::Animation::ANIMATOR_STATE::PLAY;

			Current_ = Frames_;
			reset_KEY_FRAME_STATE(Current_);

			return;
		}

		if (State_ == Andromeda::Animation::ANIMATOR_STATE::STOP || State_ == Andromeda::Animation::ANIMATOR_STATE::DONE)
		{
			countSteps();
			State_ = Andromeda::Animation::ANIMATOR_STATE::PLAY;

			Current_ = Frames_;
			reset_KEY_FRAME_STATE(Current_);

			return;
		}

		State_ = Andromeda::Animation::ANIMATOR_STATE::PLAY;
	}

	void animator::Pause()
	{
		State_ = Andromeda::Animation::ANIMATOR_STATE::PAUSE;
	}

	void animator::Stop()
	{
		State_ = Andromeda::Animation::ANIMATOR_STATE::STOP;
	}
	
	void animator::Step(const double deltaTime, void *const pcDest)
	{
		if (Current_ == nullptr)
		{
			State_ = Andromeda::Animation::ANIMATOR_STATE::DONE;
			return;
		}

		if (Current_->Next == nullptr)
		{
			State_ = Andromeda::Animation::ANIMATOR_STATE::DONE;
			return;
		}

		if (State_ != Andromeda::Animation::ANIMATOR_STATE::PLAY)
			return;

		Current_->Timer += float(deltaTime);
		
		if (Current_->Frame.FrameTime < 0.01f)
		{
			advanceFrame();
			return;
		}

		bool flag = false;
		if (Current_->Timer >= Current_->Frame.FrameTime)
		{
			Current_->Timer = Current_->Frame.FrameTime;
			flag = true;
		}

		if (pcDest != nullptr)
		{
			const double dt = (Current_->Timer / Current_->Frame.FrameTime);
			LerpCall_(&Current_->Frame, &Current_->Next->Frame, pcDest, dt);
		}

		if (flag)
			advanceFrame();
	}
	
	Andromeda::Animation::ANIMATOR_STATE animator::GetState()
	{
		return Andromeda::Animation::ANIMATOR_STATE(State_);
	}
}

namespace an_v1_impl
{
	struct ANIMATORS
	{
		char InUse[vars::MAX_ANIMATORS];
		animator Array[vars::MAX_ANIMATORS];

		ANIMATORS *Next;
	};

	void setup_ANIMATORS(ANIMATORS *const pcA)
	{
		for (unsigned long i = 0; i < vars::MAX_ANIMATORS; ++i)
		{
			(*pcA).InUse[i] = 0;
			(*pcA).Array[i].init();
		}

		(*pcA).Next = nullptr;
	}

	class keyFrames :public Andromeda::Animation::IKeyframes
	{
	private:

		ANIMATORS Head_;

	public:

		keyFrames();
		~keyFrames();

		void init();

	public:

		virtual long GetAnimator(const unsigned long dataSize, IAnimator **const ppcOut, Andromeda::Animation::IAnimatorStep lerpFunc);
	};

	keyFrames::keyFrames()
	{

	}

	keyFrames::~keyFrames()
	{

	}
	
	void keyFrames::init()
	{
		setup_ANIMATORS(&Head_);
	}
	
	long keyFrames::GetAnimator(const unsigned long dataSize, IAnimator **const ppcOut, Andromeda::Animation::IAnimatorStep lerpFunc)
	{
		if (!ppcOut)
			return -1;

		if (dataSize == 0)
			return -2;

		if (!lerpFunc)
			return -3;

		ANIMATORS *temp = &Head_;
		bool flag = false;

		while (temp != nullptr)
		{
			for (unsigned long i = 0; i < vars::MAX_ANIMATORS; ++i)
			{
				if (temp->InUse[i] == 0)
				{
					temp->InUse[i] = 1;

					temp->Array[i].setup(dataSize, lerpFunc);
					(*ppcOut) = &temp->Array[i];
					return (flag == true ? 1 : 0);
				}
			}

			if (temp->Next == nullptr)
			{
				temp->Next = new ANIMATORS;
				setup_ANIMATORS(temp->Next);
				flag = true;
			}
		}

		return 0;
	}
}

namespace Andromeda
{
	namespace Animation
	{
		IKeyframes *g_KeyFrames_Ptr = nullptr;

		long CreateKeyframes(IKeyframes **const ppcOut)
		{
			if (g_KeyFrames_Ptr == nullptr)
			{
				g_KeyFrames_Ptr = new an_v1_impl::keyFrames;
				((an_v1_impl::keyFrames*)g_KeyFrames_Ptr)->init();
			}

			if (ppcOut) (*ppcOut) = g_KeyFrames_Ptr;

			return 0;
		}
	}
}