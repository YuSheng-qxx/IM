#include<cstdio>
#include<iostream>
#include<mutex>
#include<list>
#include<sstream>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>
#include "mongoose.h"

using namespace std;

namespace im{
#define MYSQL_HOST "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PASS ""
#define MYSQL_DB "im_system"
#define ONLINE "online"
#define OFFLINE "offline"
  class TableUser
  {
    public:
      TableUser()
        :_mysql(NULL)
         //完成数据库操作的初始化
      {
        _mysql = mysql_init(NULL);
        if(_mysql == NULL)
        {
          printf("init mysql instance failed!\n");
          exit(-1);
        }
        if(mysql_real_connect(_mysql, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0) == NULL)
        {
          printf("connect mysql server failed!\n");
          mysql_close(_mysql);
          exit(-1);
        }
        if(mysql_set_character_set(_mysql, "utf8") != 0)
        {
          printf("set client character failed:%s\n", mysql_error(_mysql));
          mysql_close(_mysql);
          exit(-1);
        }
        //mysql_select_db(_mysql, MYSQL_DB);
      }

      ~TableUser()
        //完成数据库句柄的销毁
      {

        if(_mysql)
        {
          mysql_close(_mysql);
        }
      }

      bool Insert(const string &name, const string &passwd)
      {
#define INSERT_USER "insert tb_user value(null, '%s', MD5('%s'), '%s');"
        char tmp_sql[4096] = {0};
        sprintf(tmp_sql, INSERT_USER, name.c_str(), passwd.c_str(), OFFLINE);

        return QuerySql(tmp_sql);
      }

      bool Delete(const string &name)
      {
#define DELETE_USER "delete from tb_user where name = '%s';"
        char tmp_sql[4096] = {0};
        sprintf(tmp_sql, DELETE_USER, name.c_str());

        return QuerySql(tmp_sql);
      }

      bool UpdateStatus(const string &name, const string &status)
      {
#define UPDATE_USER_STATU "update tb_user set status='%s' where name='%s';"

        char tmp_sql[4096] = {0};
        sprintf(tmp_sql, UPDATE_USER_STATU, status.c_str(), name.c_str());

        return QuerySql(tmp_sql);
      }

      bool UpdatePasswd(const string &name, const string &passwd)
      {
#define UPDATE_USER_PASSWD "update tb_user set passwd=MD5('%s') where name='%s';"

        char tmp_sql[4096] = {0};
        sprintf(tmp_sql, UPDATE_USER_PASSWD, passwd.c_str(), name.c_str());

        return QuerySql(tmp_sql);
      }

      bool SelectOne(const string &name, Json::Value *user)
      {
#define SELECT_USER_ONE "select id, passwd, status from tb_user where name = '%s';"
        char tmp_sql[4096] = {0};
        sprintf(tmp_sql, SELECT_USER_ONE, name.c_str());
        _mutex.lock();
        if(QuerySql(tmp_sql) == false)
        {
          _mutex.unlock();
          return false;
        }
        MYSQL_RES *res = mysql_store_result(_mysql);
        _mutex.unlock();
        if(res == NULL)
        {
          printf("select one user store result failed:%s\n", mysql_error(_mysql));
          return false;
        }
        int num_row = mysql_num_rows(res);
        if(num_row != 1)
        {
          printf("one user result count error!\n");
          mysql_free_result(res);
          return false;
        }
        for(int i=0; i<num_row; i++)
        {
          MYSQL_ROW row = mysql_fetch_row(res);
          (*user)["id"] = stoi(row[0]);
          (*user)["name"] = name.c_str();
          (*user)["passwd"] = row[1];
          (*user)["status"] = row[2];
        }
        mysql_free_result(res);
        return true;
      }

      bool SelectAll(Json::Value *users)
      {
#define SELECT_USER_ALL "select id, name, passwd, status from tb_user;"

        _mutex.lock();
        if(QuerySql(SELECT_USER_ALL) == false)
        {
          _mutex.unlock();
          return false;
        }
        MYSQL_RES *res = mysql_store_result(_mysql);
        _mutex.unlock();
        if(res == NULL)
        {
          printf("select all user store result failed:%s\n", mysql_error(_mysql));
          return false;
        }
        int num_row = mysql_num_rows(res);

        for(int i=0; i<num_row; i++)
        {
          MYSQL_ROW row = mysql_fetch_row(res);
          Json::Value user;
          user["id"] = stoi(row[0]);
          user["name"] = row[1];
          user["passwd"] = row[2];
          user["status"] = row[3];
          users->append(user);
        }
        mysql_free_result(res);
        return true;
      }

      bool VerifyUser(const string &name, const string &passwd)
      {
#define VERIFY_USER "select * from tb_user where name='%s' and passwd=MD5('%s');"
        char tmp_sql[4096] = {0};
        sprintf(tmp_sql, VERIFY_USER, name.c_str(), passwd.c_str());
        _mutex.lock();
        if(QuerySql(tmp_sql) == false)
        {
          _mutex.unlock();
          return false;
        }
        MYSQL_RES *res = mysql_store_result(_mysql);
        _mutex.unlock();
        if(res == NULL)
        {
          printf("verify user store result failed:%s\n", mysql_error(_mysql));
          return false;
        }
        int num_row = mysql_num_rows(res);
        if(num_row != 1)
        {
          printf("verify user failed!\n");
          mysql_free_result(res);
          return false;
        }

        mysql_free_result(res);
        return true;
      }

      bool Exises(const string &name)
      {
#define EXISTS_USER "select * from tb_user where name='%s';"
        char tmp_sql[4096] = {0};
        sprintf(tmp_sql, EXISTS_USER, name.c_str());
        _mutex.lock();
        if(QuerySql(tmp_sql) == false)
        {
          _mutex.unlock();
          return false;
        }
        MYSQL_RES *res = mysql_store_result(_mysql);
        _mutex.unlock();
        if(res == NULL)
        {
          printf("exists user store result failed:%s\n", mysql_error(_mysql));
          return false;
        }
        int num_row = mysql_num_rows(res);
        if(num_row != 1)
        {
          printf("have no user!\n");
          mysql_free_result(res);
          return false;
        }

        mysql_free_result(res);
        return true;
      } 
    private:
      bool QuerySql(const string &sql)
      {
        if(mysql_query(_mysql, sql.c_str()) != 0)
        {
          printf("query sql:[%s] failed:%s\n", sql.c_str(), mysql_error(_mysql));
          return false;
        }
        return true;
      }
    private:
      MYSQL *_mysql;
      mutex _mutex;
  };

  struct session{
    uint64_t session_id;
    string name;
    double login_time;
    double last_atime;
    struct mg_connection *conn;
  };

  class IM
  {
    public:
      ~IM()
      {
        mg_mgr_free(&_mgr);
      }
      static bool Init(const string &port = "9000")
      {
        _tb_user = new TableUser();
        mg_mgr_init(&_mgr);
        string addr = "0.0.0.0:";
        addr += port;
        _lst_http = mg_http_listen(&_mgr, addr.c_str(), callback, &_mgr);
        if(_lst_http = NULL)
        {
          cout<<"http listen failed!\n";
          return false;
        }
        return true;
      }
      static bool Run()
      {
        while(1)
        {
          mg_mgr_poll(&_mgr, 1000);
        }
        return true;
      }

    private:
      static int Split(const string &str, const string &sep, vector<string> *list){
        int count = 0;
        size_t pos = 0, idx = 0;
        while(1)
        {
          pos = str.find(sep, idx);//从str字符串的idx位置开始找sep分隔符
          if(pos == string::npos){
            break;
          }
          list->push_back(str.substr(idx, pos-idx));
          idx = pos+sep.size();
          count++;
        }
        if(idx < str.size())
        {
          list->push_back(str.substr(idx));
          count++;
        }
        return count;
      }
      //Cookie:SESSION_ID=1243567859; NAME=zhangsan; path=/
      static bool GetCookie(const string &cookie, const string &key, string *val){
        vector<string> list;
        int count = Split(cookie, "; ", &list);
        for(auto s : list){
          vector<string> arry_cookie;
          Split(s, "=", &arry_cookie);
          if(arry_cookie[0] == key){
            *val = arry_cookie[1];
            return true;
          }
        }
        return false;
      }
      static void createSession(struct session *s,struct mg_connection *c, const string &name)
      {
        s->name = name;
        s->session_id = (uint64_t)(mg_time()*1000000);
        s->login_time = mg_time();
        s->last_atime = mg_time();
        s->conn = c;
        return ;
      }
      static void deleteSession(struct mg_connection *c){
        auto it = _list.begin();
        for(; it != _list.end(); it++)
        {
          if(it->conn == c){
            cout <<"delete session: "<<it->name<<endl;
            _list.erase(it);
            return ;
          }
        }
        return ;
      }
      static struct session *GetSessionByConn(struct mg_connection *c)
      {
        auto it = _list.begin();
        for(; it != _list.end(); it++)
        {
          if(it->conn == c){
            return &(*it);
          }
        }
        return NULL;
      }
      static struct session *GetSessionByName(const string &name)
      {
        auto it = _list.begin();
        for(; it != _list.end(); it++)
        {
          if(it->name == name){
            return &(*it);
          }
        }
        return NULL;
      }
      static bool reg(struct mg_connection *c, struct mg_http_message *hm){
        int status = 200;
        string header = "Content-Type:application/json\r\n";
        //从正文中获取提交的用户信息--json字符串
        string body;
        body.assign(hm->body.ptr, hm->body.len);
        //解析得到用户名和密码
        Json::Value user;
        Json::Reader reader;
        bool ret = reader.parse(body, user);
        if(ret == false)
        {
          status = 400;
          mg_http_reply(c, status, header.c_str(), "{\"reason\":\"请求格式错误\"}");
          return false;
        }
        //判断这个用户名是否已经被占用
        ret = _tb_user->Exises(user["name"].asString());
        if(ret == true)
        {
          status = 400;
          mg_http_reply(c, status, header.c_str(), "{\"reason\":\"用户名被占用\"}");
          return false;
        }
        //将用户信息插入到数据库中
        ret =  _tb_user->Insert(user["name"].asString(), user["passwd"].asString());
        if(ret == false)
        {
          status = 500;
          mg_http_reply(c, status, header.c_str(), "{\"      reason\":\"数据库访问错误\"}");
          return false;
        }
        mg_http_reply(c, status, NULL, "{\"reason\":\"注册成功\"}");
        return true;
      }
      static bool login(struct mg_connection *c, struct mg_http_message *hm){
        int rsp_status = 200;
        string rsp_body = "{\"reason\":\"登录成功\"}";
        string rsp_header ="Content-Type:application/json\r\n"; 
        string req_body;
        req_body.assign(hm->body.ptr, hm->body.len);

        Json::Value user;
        Json::Reader reader;
        bool ret = reader.parse(req_body, user);
        if(ret == false)
        {
          rsp_status = 400;
          rsp_body = "{\"reason\":\"请求格式错误\"}";
          mg_http_reply(c, rsp_status, rsp_header.c_str(), rsp_body.c_str());
          return false;
        }
        ret = _tb_user->VerifyUser(user["name"].asString(), user["passwd"].asString());
        if(ret == false)
        {

          rsp_status = 403;
          rsp_body = "{\"reason\":\"用户名或密码错误\"}";
          mg_http_reply(c, rsp_status, rsp_header.c_str(), rsp_body.c_str());
          return false;
        }
        ret = _tb_user->UpdateStatus(user["name"].asString(), ONLINE);
        if(ret == false)
        {
          rsp_status = 500;
          rsp_body = "{\"reason\":\"修改用户状态出错\"}";
          mg_http_reply(c, rsp_status, rsp_header.c_str(), rsp_body.c_str());
          return false;
        }
        struct session s;
        createSession(&s, c, user["name"].asString());
        _list.push_back(s);
        stringstream cookie;
        cookie << "Set-Cookie: SESSION_ID=" << s.session_id << "; path=/\r\n";
        cookie <<"Set-Cookie: NAME=" <<s.name <<"; path=/\r\n";
        rsp_header += cookie.str();
        mg_http_reply(c, rsp_status, rsp_header.c_str(), rsp_body.c_str());
        return true;
      }
      static void BroadCast(const string &msg)
      {
        struct mg_connection *c;
        for(c=_mgr.conns; c!=NULL; c = c->next)
        {
          if(c->is_websocket)
          {
            mg_ws_send(c, msg.c_str(), msg.size(), WEBSOCKET_OP_TEXT);
          }
        }
        return ;
      }
      static void callback(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
      {
        struct mg_http_message *hm = (struct mg_http_message*)ev_data;
        struct mg_ws_message *wm = (struct mg_ws_message*)ev_data;
        switch(ev)
        {
          case MG_EV_HTTP_MSG:
            if(mg_http_match_uri(hm, "/reg"))
            {
              //注册的提交表单数据请求
              reg(c, hm);
            }
            else if(mg_http_match_uri(hm, "/login"))
            {
              //登录的提交表单数据请求
              login(c, hm);
            }
            else if(mg_http_match_uri(hm, "/websocket"))
            {
              //这是一个websocket的握手请求
              //在建立websocket聊天通道的时候，就应该检测这个客户端是否已经登陆过了
              struct mg_str * cookie_str = mg_http_get_header(hm, "Cookie");
              if(cookie_str == NULL){
                //未登录用户
                string body = R"({"reason":"未登录"})";
                string header = "Content-Type:applicaton/json\r\n";
                mg_http_reply(c, 403, header.c_str(), body.c_str());
                return ;
              }
              string tmp;
              tmp.assign(cookie_str->ptr, cookie_str->len);
              string name;
              GetCookie(tmp, "NAME", &name);
              string msg = name + "加入聊天室...大家欢迎...";
              BroadCast(msg);
              mg_ws_upgrade(c, hm, NULL);
            }
            else{
              //静态页面请求
              //处理登录页面，过来的时候，都应该检测一下Cookie，判断是否登录成功了
              //如果没有检测到Session则应该跳转到登录页面
              if(hm->uri.ptr !=  "/login.html")
              {
                //获取一下cookie，根据name找一下session，没找着就意味着没有登录
                //但是这里存在一个问题：login.html依赖的其他静态资源(图片，css代码)
                //在没有登录成功的状态下获取不到这些资源
              }
              struct mg_http_serve_opts opts = {.root_dir = "./web_root"};
              mg_http_serve_dir(c, hm, &opts);
            }
            break;
          case MG_EV_WS_MSG:
            {
              string msg;
              msg.assign(wm->data.ptr, wm->data.len);
              BroadCast(msg);
            }
            break;
          case MG_EV_CLOSE:
            {
              struct session *ss = GetSessionByConn(c);
              if(ss != NULL){
                string msg = ss->name + "退出聊天室...";
                BroadCast (msg);
                _tb_user->UpdateStatus(ss->name,OFFLINE);
                deleteSession(c);
              }
            }
            break;
          default:
            break;
        }
        return ;
      }

    private:
      string _addr;//监听地址信息
      static TableUser *_tb_user;
      static struct mg_mgr _mgr;//句柄
      static struct mg_connection *_lst_http;//监听连接
      static list<struct session> _list;
  };
  TableUser * IM::_tb_user = NULL;
  struct mg_mgr IM::_mgr;
  struct mg_connection * IM::_lst_http = NULL;
  list<struct session> IM::_list;
}
