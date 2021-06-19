#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "flog.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using FLogProto::FLogRemoteLogger;
using FLogProto::Response;
using FLogProto::LogLine;

class ServerImpl final {
 public:
  ~ServerImpl() {
    server_->Shutdown();
    cq_->Shutdown();
  }

  void Run() {

    std::string server_address("localhost:50051");
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    HandleRpcs();
  }

 private:
  class CallData {
   public:
    CallData(FLogRemoteLogger::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {

      Proceed();
    }

    void Proceed() {

      if (status_ == CREATE) {

          status_ = PROCESS;
          service_->RequestSendLogLine(&ctx_, &request_, &responder_, cq_, cq_, this);
      } else if (status_ == PROCESS) {

          new CallData(service_, cq_);
          std::string prefix("logged:  ");
          reply_.set_message(prefix + request_.log());
          std::cout << "logline: " << request_.log() << std::endl;
          status_ = FINISH;
          responder_.Finish(reply_, Status::OK, this);
      } else {

          GPR_ASSERT(status_ == FINISH);
          delete this;
      }
    }

   private:
    FLogRemoteLogger::AsyncService* service_;
    ServerCompletionQueue* cq_;
    ServerContext ctx_;
    LogLine request_;
    Response reply_;
    ServerAsyncResponseWriter<Response> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };

  void HandleRpcs() {

    new CallData(&service_, cq_.get());
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {

      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  FLogRemoteLogger::AsyncService service_;
  std::unique_ptr<Server> server_;
};

int main(int argc, char** argv) {
  ServerImpl server;
  server.Run();

  return 0;
}
