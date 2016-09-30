#include <cutils/log.h>
#include "vsync.h"

using namespace android;


VSyncThread::VSyncThread(int display, const nsecs_t refreshPeriod)
	: mEnabled(false),
	  mDisplay(display),
      mNextFakeVSync(0),
      mRefreshPeriod(refreshPeriod) {
	
}

void VSyncThread::setEnabled(bool enabled) {
    Mutex::Autolock _l(mLock);
    if (mEnabled != enabled) {
        mEnabled = enabled;
        mCondition.signal();
    }
}

void VSyncThread::registerHwcProcs(const hwc_procs_t* procs) {
	// Mutex::Autolock _l(mLock);
    if (mHwcProcs != procs) {
        mHwcProcs = procs;
    }
}

void VSyncThread::onFirstRef() {
	run("HwcVSync", PRIORITY_URGENT_DISPLAY + PRIORITY_MORE_FAVORABLE);
}
bool VSyncThread::threadLoop() {
    { // scope for lock
        Mutex::Autolock _l(mLock);
        while (!mEnabled) {
            mCondition.wait(mLock);
        }
    }

    const nsecs_t period = mRefreshPeriod;
    const nsecs_t now = systemTime(CLOCK_MONOTONIC);
    nsecs_t next_vsync = mNextFakeVSync;
    nsecs_t sleep = next_vsync - now;
    if (sleep < 0) {
        // we missed, find where the next vsync should be
        sleep = (period - ((now - next_vsync) % period));
        next_vsync = now + sleep;
    }
    mNextFakeVSync = next_vsync + period;

    struct timespec spec;
    spec.tv_sec  = next_vsync / 1000000000;
    spec.tv_nsec = next_vsync % 1000000000;

    int err;
    do {
        err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
    } while (err<0 && errno == EINTR);

    if (err == 0) {
		// Note: maybe we should use a seperated lock for mHwcProcs
		// Mutex::Autolock _l(mLock);
		if (mHwcProcs && mHwcProcs->vsync) {
			mHwcProcs->vsync(mHwcProcs, mDisplay, next_vsync);
		}
    }
	
    return true;
}

