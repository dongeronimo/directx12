#pragma once
namespace common
{
	class GameTimer
	{
	public:
		GameTimer();
		//In seconds
		float TotalTime()const;
		//In seconds
		float DeltaTime()const;
		//Call before msg loop
		void Reset();
		//call when unpaused;
		void Start();
		//call when paused
		void Stop();
		//call every frame
		void Tick();
	private:
		double mSecondsPerCount;
		double mDeltaTime;
		__int64 mBaseTime;
		__int64 mPausedTime;
		__int64 mStopTime;
		__int64 mPrevTime;
		__int64 mCurrTime;
		bool mStopped;
	};
}