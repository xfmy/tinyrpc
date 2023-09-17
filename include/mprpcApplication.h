#pragma once
#include <boost/noncopyable.hpp>

//mprpc框架的基础类,负责框架的初始化操作
class MprpcApplication : public boost::noncopyable{
public:
    static const MprpcApplication &GetInstance(void)
    {
        static MprpcApplication obj;
        return obj;
    }
    static void init(int argc, char** argv);

private:
};