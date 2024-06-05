#include "muduo/net/http/HttpServer.h"
#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/http/HttpResponse.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/Logging.h"

#include "../src/tinypb/tinyPB_protocol.h"
#include "rpc_dispatcher.h"
#include "rpc_application.h"
#include "protobuf.pb.h"

#include "memory"
#include <iostream>
#include <map>
#include <functional>

#ifndef IS_HEX
#define IS_HEX(c) \
    (IS_DIGIT(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#endif

static inline unsigned char hex2i(char hex)
{
    return hex <= '9'   ? hex - '0'
           : hex <= 'F' ? hex - 'A' + 10
                        : hex - 'a' + 10;
}

std::string unescape(const std::string& str)
{
    std::string ostr;
    const char* p = str.c_str();
    while (*p != '\0')
    {
        if (*p == '%' && IS_HEX(p[1]) && IS_HEX(p[2]))
        {
            ostr += ((hex2i(p[1]) << 4) | hex2i(p[2]));
            p += 3;
        }
        else
        {
            if (*p == '+')
            {
                ostr += ' ';
            }
            else
            {
                ostr += *p;
            }
            ++p;
        }
    }
    return ostr;
}

int parse_query_params(const char* query_string,
                       std::map<std::string, std::string>& query_params)
{
    const char* p = strchr(query_string, '?');
    p = p ? p + 1 : query_string;

    enum
    {
        s_key,
        s_value,
    } state = s_key;

    const char* key = p;
    const char* value = NULL;
    int key_len = 0;
    int value_len = 0;
    while (*p != '\0')
    {
        if (*p == '&')
        {
            if (key_len /* && value_len */)
            {
                std::string strkey = std::string(key, key_len);
                std::string strvalue = std::string(value, value_len);
                query_params[unescape(strkey)] = unescape(strvalue);
                key_len = value_len = 0;
            }
            state = s_key;
            key = p + 1;
        }
        else if (*p == '=' && state == s_key)
        {
            state = s_value;
            value = p + 1;
        }
        else
        {
            state == s_key ? ++key_len : ++value_len;
        }
        ++p;
    }
    if (key_len /* && value_len */)
    {
        std::string strkey = std::string(key, key_len);
        std::string strvalue = std::string(value, value_len);
        query_params[unescape(strkey)] = unescape(strvalue);
        key_len = value_len = 0;
    }
    return query_params.size() == 0 ? -1 : 0;
}

using namespace muduo;
using namespace muduo::net;

using namespace tinyrpc;

using func = void(std::shared_ptr<TinyPBProtocol> reqest,
                  std::shared_ptr<TinyPBProtocol> response,
                  const muduo::net::TcpConnectionPtr &ptr);



class UserService : public fixbug::user
{
public:
    bool Login(std::string name, std::string password)
    {
        LOG_INFO << "user name is " + name;
        LOG_INFO << "user password is " + password;
        return true;
    }

    virtual void Login(google::protobuf::RpcController* controller,
                       const fixbug::LoginRequest* requst,
                       fixbug::LoginResponse* response,
                       google::protobuf::Closure* done) override
    {
        std::string name = requst->name();
        std::string password = requst->pwd();
        int res = Login(name, password);
        fixbug::ResultCode* resCode = response->mutable_result();
        resCode->set_errcode(0);
        resCode->set_errmsg("一切正常 very good");
        response->set_sucess(res);
        // Closure是一个抽象类，
        done->Run();
    }
};
tinyrpc::RpcDispatcher provider;
extern char favicon[555];
bool benchmark = false;

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
  std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
  std::cout << "query:" << req.query() << std::endl;
  /****************************************************************************/


  if (!benchmark)
  {
    const std::map<string, string>& headers = req.headers();
    for (const auto& header : headers)
    {
      std::cout << header.first << ": " << header.second << std::endl;
    }
  }

  if (req.path() == "/")
  {
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/html");
    resp->addHeader("Server", "Muduo");
    string now = Timestamp::now().toFormattedString();
    resp->setBody("<html><head><title>This is title</title></head>"
        "<body><h1>Hello</h1>Now is " + now +
        "</body></html>");
  }
  else if (req.path() == "/qps")
  {
      // std::shared_ptr<tinyrpc::TinyPBProtocol> request(std::make_shared<tinyrpc::TinyPBProtocol>());
      // std::shared_ptr<TinyPBProtocol> response(std::make_shared<tinyrpc::TinyPBProtocol>());
      // request->msgId_ = "666";
      // request->methodName_ = "user.Login";
      // fixbug::LoginRequest loginRequest;
      // loginRequest.set_name("root");
      // loginRequest.set_pwd("admin");
      // loginRequest.SerializePartialToString(&request->pbData_);

      std::map<std::string,std::string> queryMap;
      parse_query_params(req.query().c_str(), queryMap);

      try
      {
          std::string methonFullName = queryMap["methon_name"];
          std::string serviceName;
          std::string methodName;

          // if (!parseServiceFullName(methonFullName, serviceName, methodName))
          // {
          //     setTinyPBError(response, ERROR_PARSE_SERVICE_NAME,
          //                    "parse service name error");
          //     return;
          // }

          //response->msgId_ = request->msgId_;
          //response->methodName_ = request->methodName_;

          auto it = serviceMap_.find(serviceName);
          if (it == serviceMap_.end())
          {
              LOG_ERROR << request->msgId_ + "| sericve neame[" + serviceName +
                               "] not found";
              setTinyPBError(response, ERROR_SERVICE_NOT_FOUND,
                             "service not found");
              return;
          }

          servicePtr service = (*it).second;

          const PROTO::MethodDescriptor* method =
              service->GetDescriptor()->FindMethodByName(methodName);
          if (method == NULL)
          {
              LOG_ERROR << request->msgId_ + "method neame[" + methodName +
                               "] not found in service[" + serviceName;
              setTinyPBError(response, ERROR_SERVICE_NOT_FOUND,
                             "method not found");
              return;
          }

          PROTO::Message* req_msg = service->GetRequestPrototype(method).New();

          // 反序列化，将 pb_data 反序列化为 req_msg
          if (!req_msg->ParseFromString(request->pbData_))
          {
              LOG_ERROR << request->msgId_ + "deserilize error" + methodName +
                               serviceName;
              setTinyPBError(response, ERROR_FAILED_DESERIALIZE,
                             "deserilize error");
              return;
          }

          LOG_INFO << request->msgId_ + "| get rpc request" +
                          req_msg->ShortDebugString();
          PROTO::Message* resp_msg =
              service->GetResponsePrototype(method).New();

          RpcController* rpcController = new RpcController();
          rpcController->SetLocalAddr(ptr->localAddress());
          rpcController->SetPeerAddr(ptr->peerAddress());
          rpcController->SetMsgId(request->msgId_);

          RpcClosure* closure = new RpcClosure([req_msg, resp_msg, request,
                                                response, ptr, rpcController,
                                                this]() mutable {
              if (!resp_msg->SerializeToString(&(response->pbData_)))
              {
                  LOG_ERROR << std::string() + request->msgId_ +
                                   "| serilize error, origin message ->" +
                                   resp_msg->ShortDebugString();
                  setTinyPBError(response, ERROR_FAILED_SERIALIZE,
                                 "serilize error");
              }
              else
              {
                  response->errorCode_ = 0;
                  response->errorInfo_ = "";
                  LOG_INFO << std::string() + request->msgId_ +
                                  "| dispatch success, requesut->" +
                                  req_msg->ShortDebugString() + ", response->" +
                                  resp_msg->ShortDebugString();
              }

              // std::vector<AbstractProtocol::s_ptr> replay_messages;
              // replay_messages.emplace_back(rsp_protocol);
              //将rsp_protocol发送client
              // ptr->reply(replay_messages);

              // resp_msg->SerializeToString(&response->pbData_);
              responseToClient(ptr, response);
          });

          service->CallMethod(method, rpcController, req_msg, resp_msg,
                              closure);
      }
      catch (std::exception err)
      {
          LOG_ERROR << err.what();
      }
  }
  else if (req.path() == "/favicon.ico")
  {
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("image/png");
    resp->setBody(string(favicon, sizeof favicon));
  }
  else if (req.path() == "/hello")
  {
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/plain");
    resp->addHeader("Server", "Muduo");
    resp->setBody("hello, world!\n");
  }
  else
  {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
  }
}

int main()
{
  int numThreads = 1;
  // if (argc > 1)
  // {
  //   benchmark = true;
  //   Logger::setLogLevel(Logger::WARN);
  //   numThreads = atoi(argv[1]);
  // }
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "dummy");
  server.setHttpCallback(onRequest);
  server.setThreadNum(numThreads);
  std::shared_ptr<UserService> userService = std::make_shared<UserService>();
  provider.registerService(userService);
   //std::bind(&RpcDispatcher::dispatch, &provider, _1, _2, _3);
  server.start();
  loop.loop();

  
}

char favicon[555] = {
  '\x89', 'P', 'N', 'G', '\xD', '\xA', '\x1A', '\xA',
  '\x0', '\x0', '\x0', '\xD', 'I', 'H', 'D', 'R',
  '\x0', '\x0', '\x0', '\x10', '\x0', '\x0', '\x0', '\x10',
  '\x8', '\x6', '\x0', '\x0', '\x0', '\x1F', '\xF3', '\xFF',
  'a', '\x0', '\x0', '\x0', '\x19', 't', 'E', 'X',
  't', 'S', 'o', 'f', 't', 'w', 'a', 'r',
  'e', '\x0', 'A', 'd', 'o', 'b', 'e', '\x20',
  'I', 'm', 'a', 'g', 'e', 'R', 'e', 'a',
  'd', 'y', 'q', '\xC9', 'e', '\x3C', '\x0', '\x0',
  '\x1', '\xCD', 'I', 'D', 'A', 'T', 'x', '\xDA',
  '\x94', '\x93', '9', 'H', '\x3', 'A', '\x14', '\x86',
  '\xFF', '\x5D', 'b', '\xA7', '\x4', 'R', '\xC4', 'm',
  '\x22', '\x1E', '\xA0', 'F', '\x24', '\x8', '\x16', '\x16',
  'v', '\xA', '6', '\xBA', 'J', '\x9A', '\x80', '\x8',
  'A', '\xB4', 'q', '\x85', 'X', '\x89', 'G', '\xB0',
  'I', '\xA9', 'Q', '\x24', '\xCD', '\xA6', '\x8', '\xA4',
  'H', 'c', '\x91', 'B', '\xB', '\xAF', 'V', '\xC1',
  'F', '\xB4', '\x15', '\xCF', '\x22', 'X', '\x98', '\xB',
  'T', 'H', '\x8A', 'd', '\x93', '\x8D', '\xFB', 'F',
  'g', '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f', 'v',
  'f', '\xDF', '\x7C', '\xEF', '\xE7', 'g', 'F', '\xA8',
  '\xD5', 'j', 'H', '\x24', '\x12', '\x2A', '\x0', '\x5',
  '\xBF', 'G', '\xD4', '\xEF', '\xF7', '\x2F', '6', '\xEC',
  '\x12', '\x20', '\x1E', '\x8F', '\xD7', '\xAA', '\xD5', '\xEA',
  '\xAF', 'I', '5', 'F', '\xAA', 'T', '\x5F', '\x9F',
  '\x22', 'A', '\x2A', '\x95', '\xA', '\x83', '\xE5', 'r',
  '9', 'd', '\xB3', 'Y', '\x96', '\x99', 'L', '\x6',
  '\xE9', 't', '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',
  '\xA7', '\xC4', 'b', '1', '\xB5', '\x5E', '\x0', '\x3',
  'h', '\x9A', '\xC6', '\x16', '\x82', '\x20', 'X', 'R',
  '\x14', 'E', '6', 'S', '\x94', '\xCB', 'e', 'x',
  '\xBD', '\x5E', '\xAA', 'U', 'T', '\x23', 'L', '\xC0',
  '\xE0', '\xE2', '\xC1', '\x8F', '\x0', '\x9E', '\xBC', '\x9',
  'A', '\x7C', '\x3E', '\x1F', '\x83', 'D', '\x22', '\x11',
  '\xD5', 'T', '\x40', '\x3F', '8', '\x80', 'w', '\xE5',
  '3', '\x7', '\xB8', '\x5C', '\x2E', 'H', '\x92', '\x4',
  '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g', '\x98',
  '\xE9', '6', '\x1A', '\xA6', 'g', '\x15', '\x4', '\xE3',
  '\xD7', '\xC8', '\xBD', '\x15', '\xE1', 'i', '\xB7', 'C',
  '\xAB', '\xEA', 'x', '\x2F', 'j', 'X', '\x92', '\xBB',
  '\x18', '\x20', '\x9F', '\xCF', '3', '\xC3', '\xB8', '\xE9',
  'N', '\xA7', '\xD3', 'l', 'J', '\x0', 'i', '6',
  '\x7C', '\x8E', '\xE1', '\xFE', 'V', '\x84', '\xE7', '\x3C',
  '\x9F', 'r', '\x2B', '\x3A', 'B', '\x7B', '7', 'f',
  'w', '\xAE', '\x8E', '\xE', '\xF3', '\xBD', 'R', '\xA9',
  'd', '\x2', 'B', '\xAF', '\x85', '2', 'f', 'F',
  '\xBA', '\xC', '\xD9', '\x9F', '\x1D', '\x9A', 'l', '\x22',
  '\xE6', '\xC7', '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15',
  '\x90', '\x7', '\x93', '\xA2', '\x28', '\xA0', 'S', 'j',
  '\xB1', '\xB8', '\xDF', '\x29', '5', 'C', '\xE', '\x3F',
  'X', '\xFC', '\x98', '\xDA', 'y', 'j', 'P', '\x40',
  '\x0', '\x87', '\xAE', '\x1B', '\x17', 'B', '\xB4', '\x3A',
  '\x3F', '\xBE', 'y', '\xC7', '\xA', '\x26', '\xB6', '\xEE',
  '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
  '\xA', '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X', '\x0',
  '\x27', '\xEB', 'n', 'V', 'p', '\xBC', '\xD6', '\xCB',
  '\xD6', 'G', '\xAB', '\x3D', 'l', '\x7D', '\xB8', '\xD2',
  '\xDD', '\xA0', '\x60', '\x83', '\xBA', '\xEF', '\x5F', '\xA4',
  '\xEA', '\xCC', '\x2', 'N', '\xAE', '\x5E', 'p', '\x1A',
  '\xEC', '\xB3', '\x40', '9', '\xAC', '\xFE', '\xF2', '\x91',
  '\x89', 'g', '\x91', '\x85', '\x21', '\xA8', '\x87', '\xB7',
  'X', '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N', 'N',
  'b', 't', '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
  '\xEC', '\x86', '\x2', 'H', '\x26', '\x93', '\xD0', 'u',
  '\x1D', '\x7F', '\x9', '2', '\x95', '\xBF', '\x1F', '\xDB',
  '\xD7', 'c', '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF',
  '\x22', 'J', '\xC3', '\x87', '\x0', '\x3', '\x0', 'K',
  '\xBB', '\xF8', '\xD6', '\x2A', 'v', '\x98', 'I', '\x0',
  '\x0', '\x0', '\x0', 'I', 'E', 'N', 'D', '\xAE',
  'B', '\x60', '\x82',
};

