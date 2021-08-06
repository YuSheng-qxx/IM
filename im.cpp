#include "im.hpp"

void test()
{
  im::TableUser user;
  //user.Insert("lisi", "111111");
  //user.UpdatePasswd("zhangsan", "1111111");
  //cout<<user.VerifyUser("lisi", "111111")<<endl;
  //cout<<user.Exises("zhangsan")<<endl;
  Json::Value value;
  user.SelectAll(&value);
  Json::StyledWriter writer;
  //cout<<writer.write(value)<<endl; 
  user.Delete("lisi");
}

int main()
{
  im::IM im_server;
  im_server.Init();
  im_server.Run();
  //test();
  return 0;
}
