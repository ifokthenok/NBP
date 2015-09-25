#ifndef BNHELLOWORLDSERVICE_H
#define BNHELLOWORLDSERVICE_H

#include <binder/Parcel.h>
#include "IHelloWorldService.h"

namespace android {

class BnHelloWorldService : public BnInterface<IHelloWorldService> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data, 
        Parcel* reply, uint32_t flags = 0);
};


}; // namespace androidtuple


#endif // HELLOWORLDSERVICE
