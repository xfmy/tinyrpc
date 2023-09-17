//#include <gtest/gtest.h>
#include "protobuf.pb.h"
#include <string>
#include <memory>
int add(int a, int b) { return a + b; }

// TEST(ADD,MIN){
//     EXPECT_EQ(add(1, 2), 3);
//     EXPECT_EQ(add(1, 2), 2);
// }

// TEST(ADD, MAX)
// {
//     EXPECT_EQ(add(100, 200), 300);
//     EXPECT_EQ(add(1000, 2000), 2000);
// }

#include <boost/noncopyable.hpp>

int main()
{
    using namespace fixbug;
    using namespace std::string_literals;
    std::unique_ptr<LoginRequest> ptr(std::make_unique<LoginRequest>());

    //LoginRequest obj;
    std::string name("mayun");
    std::string psw("123123");
    ptr->set_allocated_name(&name);
    ptr->set_allocated_pwd(&psw);
    std::string send_buf;
    ptr->SerializeToString(&send_buf);
    std::cout << "size:" << send_buf.size() << '[' << send_buf << ']' << std::endl;
    // testing::InitGoogleTest();
    // EXPECT_EQ(add(1, 2), 1);
    // //return RUN_ALL_TESTS();
    return 0;
}