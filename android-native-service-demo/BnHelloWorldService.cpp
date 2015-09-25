#include <binder/Parcel.h>
#include "BnHelloWorldService.h"

namespace android {

status_t BnHelloWorldService::onTransact(uint32_t code, const Parcel& data, 
    Parcel* reply, uint32_t flags) {
    status_t status;
    switch (code) {
    // PRC function
    case HW_HELLOWORLD : {
        // RPC Interface
        CHECK_INTERFACE(IHelloWorldService, data, reply);
        // RPC function arguments
        const char* str = data.readCString();
        reply->writeInt32(helloWorld(str));
        status = NO_ERROR;
        break;
    }
        
    default:
        status = BBinder::onTransact(code, data, reply, flags);
    }
    
    return status;
}


}; // namespace androidtuple
