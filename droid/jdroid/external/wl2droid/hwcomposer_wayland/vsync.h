#ifndef HWCOMPOSER_VSYNC_H_
#define HWCOMPOSER_VSYNC_H_

#include <stdint.h>
#include <sys/types.h>

#include <hardware/hwcomposer_defs.h>
#include <hardware/hwcomposer.h>


//#include <ui/Fence.h>

//#include <utils/BitSet.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
//#include <utils/StrongPointer.h>
#include <utils/Thread.h>
//#include <utils/Timers.h>
//#include <utils/Vector.h>

using android::Mutex;
using android::Condition;
using android::Thread;


extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
            		const struct timespec *request, 
            		struct timespec *remain);


class VSyncThread : public Thread {

public:
	VSyncThread(int display=0, const nsecs_t refreshPeriod=40*1000*1000);
	void setEnabled(bool enabled);

	void registerHwcProcs(const hwc_procs_t* procs);

private:
	mutable Mutex mLock;
	Condition mCondition;
	bool mEnabled;
	const int mDisplay;
	mutable nsecs_t mNextFakeVSync;
	nsecs_t mRefreshPeriod;
	const hwc_procs_t* mHwcProcs;
	virtual void onFirstRef();
	virtual bool threadLoop();
};




#endif // HWCOMPOSER_VSYNC_H_

