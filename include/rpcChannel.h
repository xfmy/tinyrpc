#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

namespace PROTO = google::protobuf;

class RpcChannel : public PROTO::RpcChannel
{
public:
    virtual void CallMethod(const PROTO::MethodDescriptor *method,
                            PROTO::RpcController *controller,
                            const PROTO::Message *request,
                            PROTO::Message *response, 
                            PROTO::Closure *done) override;
};
