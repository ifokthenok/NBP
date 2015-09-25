#ifndef HELLOWORLDSERVICE_H
#define HELLOWORLDSERVICE_H

#include "BnHelloWorldService.h"

namespace android {

class HelloWorldService : public BnHelloWorldService {
public:
    static void instantiate();
    virtual status_t helloWorld(const char* msg);
    virtual status_t onTransact(uint32_t code, const Parcel& data, 
        Parcel* reply, uint32_t flags = 0);   
private:
     HelloWorldService();
     virtual ~HelloWorldService();
};

}; // namespace android


#endif // HELLOWORLDSERVICE_H
