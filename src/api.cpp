
#include "api.hpp"
#include "stl.hpp"
#include "github/client.hpp"
#include "github/types.hpp"
#include "db/sqlite_store.hpp"
#include "ui_interface/user_list_vm.hpp"

using mx3::Api;
using json11::Json;

using namespace std;
using namespace v8;

namespace {
    const string USERNAME_KEY      = "username";
    const string LAUNCH_NUMBER_KEY = "launch_number";
}

shared_ptr<mx3_gen::Api>
mx3_gen::Api::create_api(
    const string& root_path,
    const shared_ptr<mx3_gen::EventLoop>& ui_thread,
    const shared_ptr<Http> & http_impl,
    const shared_ptr<ThreadLauncher> & launcher
) {
    const auto ui_runner = make_shared<mx3::EventLoopRef>(ui_thread);
    const auto bg_runner = make_shared<mx3::EventLoopCpp>(launcher);
    return make_shared<mx3::Api>(root_path, ui_runner, bg_runner, http_impl);
}

Api::Api(
    const string& root_path,
    const shared_ptr<SingleThreadTaskRunner>& ui_runner,
    const shared_ptr<SingleThreadTaskRunner>& bg_runner,
    const shared_ptr<mx3_gen::Http>& http_client
) :
    // todo this needs to use a fs/path abstraction (not yet built)
    m_db { std::make_unique<mx3::SqliteStore>(root_path + "/kv.sqlite") },
    m_ui_thread {ui_runner},
    m_bg_thread {bg_runner},
    m_bg_http {http_client, m_bg_thread}
{
    m_sqlite = mx3::sqlite::Db::open(root_path + "/example.sqlite");
    _setup_db();
    m_read_db = mx3::sqlite::Db::open(root_path + "/example.sqlite");

    auto j_launch_number = m_db->get(LAUNCH_NUMBER_KEY);
    size_t launch = 0;
    if (j_launch_number.is_number()) {
        launch = j_launch_number.number_value() + 1;
    }
    m_db->set("launch_number", static_cast<double>(launch));
}

bool
Api::has_user() {
    return !m_db->get(USERNAME_KEY).is_null();
}

string
Api::get_username() {
    return m_db->get(USERNAME_KEY).string_value();
}

void
Api::set_username(const string& username) {
    m_db->set(USERNAME_KEY, username);
}

shared_ptr<mx3_gen::UserListVmHandle>
Api::observer_user_list() {
    return make_shared<mx3::UserListVmHandle>(m_read_db, m_bg_http, m_ui_thread, m_bg_thread);
}

void
Api::_setup_db() {
    vector<string> setup_commands  {
        "CREATE TABLE IF NOT EXISTS `github_users` ("
            "`login` TEXT, "
            "`id` INTEGER, "
            "`avatar_url` TEXT, "
            "`gravatar_id` TEXT, "
            "`url` TEXT, "
            "`html_url` TEXT, "
            "`followers_url` TEXT, "
            "`following_url` TEXT, "
            "`gists_url` TEXT, "
            "`starred_url` TEXT, "
            "`subscriptions_url` TEXT, "
            "`organizations_url` TEXT, "
            "`repos_url` TEXT, "
            "`events_url` TEXT, "
            "`received_events_url` TEXT, "
            "`type` TEXT, "
            "`site_admin` TEXT, "
            "PRIMARY KEY(id)"
        ");"
    };
    for (const auto& cmd : setup_commands) {
        m_sqlite->exec(cmd);
    }
    m_sqlite->enable_wal();
}

Handle<v8::Value> CallJsFunction(Isolate* isolate, Handle<v8::Object> global, std::string funcName, Handle<Value> argList[], unsigned int argCount) {
    Handle<Value> js_result;
    Handle<v8::Value> value = global->Get(String::NewFromUtf8(isolate, funcName.c_str()));
    Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(value);
    js_result = func->Call(global, argCount, argList);
    return js_result;
}

void AddFromCpp(Isolate* isolate, const v8::FunctionCallbackInfo<v8::Value>& args) {
    int myVal = 0;
    for (int i = 0; i < args.Length(); i++) {
        v8::HandleScope handle_scope(args.GetIsolate());
        myVal += args[i]->Int32Value();
    }
    args.GetReturnValue().Set(v8::Number::New(isolate, myVal));
}
//void CallCppFunction(Isolate* isolate, Handle<v8::Object> global) {
//    global->Set(v8::String::NewFromUtf8(isolate, "add"), v8::FunctionTemplate::New(isolate, AddFromCpp)->GetFunction());
//}

void AddStringToArguments(Isolate* isolate, std::string str, Handle<Value> argList[], unsigned int argPos) {
    argList[argPos] = v8::String::NewFromUtf8(isolate, str.c_str());
}
void AddNumbertoArguments(Isolate* isolate, double num, Handle<Value> argList[], unsigned int argPos) {
    argList[argPos] = v8::Number::New(isolate, num);
}
void AddBooleanToArguments(Isolate* isolate, bool value, Handle<Value> argList[], unsigned int argPos) {
    argList[argPos] = v8::Boolean::New(isolate, value);
}

double
Api::add(double i, double j) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope handle_scope(isolate);
    Handle<ObjectTemplate> global_template = ObjectTemplate::New();
    Local<Context> context = Context::New(isolate, NULL, global_template);
    Context::Scope context_scope(context);
   
    string s = "function add(i, j) { return (i+j); }";
    Handle<String> source = v8::String::NewFromUtf8(isolate, s.c_str());

    Handle<Script> script = Script::Compile(source);
    script->Run();
    Handle<Value> args[2];
    AddNumbertoArguments(isolate, i, args, 0);
    AddNumbertoArguments(isolate, j, args, 1);
    Handle<Value> js_result = CallJsFunction(isolate, context->Global(), "add", args, 2);
    Handle<Number> num = v8::Handle<v8::Number>::Cast(js_result);
    return num->Value();
}
