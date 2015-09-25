#include <binder/Parcel.h>
#include "BpHelloWorldService.h"

namespace android {

BpHelloWorldService::BpHelloWorldService(const sp<IBinder>& impl) 
    : BpInterface<IHelloWorldService>(impl) {
}

status_t BpHelloWorldService::helloWorld(const char* msg) {
    Parcel data; 
    Parcel reply;
    // Specify RPC Interface
    data.writeInterfaceToken(IHelloWorldService::getInterfaceDescriptor());
    // Specify RPC function arguments
    data.writeCString(msg);
    // Sepcify RPC function by 'HW_HELLOWORD' 
    status_t status = remote()->transact(HW_HELLOWORLD, data, &reply);
    if (status != NO_ERROR) {
        LOGE("helloworld error: %s", strerror(-status));
    } else {
        // Get RPC return value
        status = reply.readInt32();
    }
    return status;
}


}; // namespace androidtuple
