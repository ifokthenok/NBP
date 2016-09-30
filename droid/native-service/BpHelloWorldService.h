#ifndef BPHELLOWORLDSERVICE_H
#define BPHELLOWORLDSERVICE_H

#include <binder/Parcel.h>
#include "IHelloWorldService.h"

namespace android {

class BpHelloWorldService : public BpInterface<IHelloWorldService> {
public:
    BpHelloWorldService(const sp<IBinder>& impl);
    virtual status_t helloWorld(const char* msg);
};


}; // namespace androidtuple


#endif // BPHELLOWORLDSERVICE_H
