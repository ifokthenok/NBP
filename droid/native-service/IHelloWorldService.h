#ifndef IHELLOWORLDSERVICE_H
#define IHELLOWORLDSERVICE_H

#include <binder/IInterface.h>

namespace android {

enum {
    HW_HELLOWORLD = IBinder::FIRST_CALL_TRANSACTION,
};

class IHelloWorldService : public IInterface {
public:
    virtual status_t helloWorld(const char* msg) = 0;
    DECLARE_META_INTERFACE(HelloWorldService);
};

}; // namespace android


#endif // IHELLOWORLDSERVICE_H

