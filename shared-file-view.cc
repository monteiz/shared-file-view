#ifndef __ARM_ARCH
#ifdef __linux__
#include <features.h>
#ifdef __GLIBC_PREREQ
#if __GLIBC_PREREQ(2, 13)
__asm__(".symver clock_gettime,clock_gettime@GLIBC_2.2.5");
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
#endif
#endif
#endif
#endif
#include <boost/unordered_map.hpp>
#include <boost/version.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <nan.h>

#if BOOST_VERSION < 108100
#pragma message("Found boost version " BOOST_PP_STRINGIZE(BOOST_LIB_VERSION))
#error mmap-object needs at least version 1_81 to maintain compatibility.
#endif

// For Win32 compatibility
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode)&S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode) (((mode)&S_IFMT) == S_IFREG)
#endif

using namespace std;
namespace bip = boost::interprocess;
namespace fs = boost::filesystem;
using namespace boost::interprocess;

class SharedFileView : public Nan::ObjectWrap
{
  SharedFileView(const std::string &file_full_path) : file_full_path(file_full_path), map_shared_memory_name(file_full_path)
  {

    std::replace(map_shared_memory_name.begin(), map_shared_memory_name.end(), '/', '_');

    std::ifstream file(file_full_path.c_str());

    length = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');

    std::ostringstream oss;
    oss << map_shared_memory_name << "_index";
    index_shared_memory_name = oss.str();
  }

public:
  static NAN_MODULE_INIT(Init);
  uint length;

  void create()
  {

    if (!fs::exists(file_full_path.c_str()))
    {
      std::ostringstream error_stream;
      error_stream << "File " << file_full_path << " not found";
      Nan::ThrowError(error_stream.str().c_str());
      return;
    }

    u_int64_t file_size = fs::file_size(file_full_path.c_str());

    shared_memory_object::remove(map_shared_memory_name.c_str());
    shared_memory_object shm(create_only, map_shared_memory_name.c_str(), read_write);

    shm.truncate(file_size);

    mapped_region region(shm, read_write);

    // Copy file content into mapped region
    std::ifstream ifs(file_full_path.c_str(), std::ios::binary);
    char *dst = static_cast<char *>(region.get_address());
    ifs.read(dst, region.get_size());

    createIndex();
  }

  void createIndex()
  {
    std::vector<size_t> index;
    std::ifstream file(file_full_path);
    std::string line;
    size_t position = 0;
    while (std::getline(file, line))
    {
      index.push_back(position);
      position = file.tellg();
    }
    index.push_back(position);

    shared_memory_object::remove(index_shared_memory_name.c_str());

    shared_memory_object shm(create_only, index_shared_memory_name.c_str(), read_write);
    shm.truncate(index.size() * sizeof(uint32_t));
    mapped_region region(shm, read_write);

    std::copy(index.begin(), index.end(), static_cast<uint32_t *>(region.get_address()));
  }

  void open()
  {
    shared_memory_object shm_source(open_only, map_shared_memory_name.c_str(), read_only);
    region_source = mapped_region(shm_source, read_only);

    shared_memory_object shm_index(open_only, index_shared_memory_name.c_str(), read_only);
    region_index = mapped_region(shm_index, read_only);
  }

  bool exists()
  {
    try
    {
      shared_memory_object shm_source(open_only, map_shared_memory_name.c_str(), read_only);

      std::string name = shm_source.get_name();

      return (!name.empty());
    }
    catch (const std::exception &ex)
    {
      return (false);
    }
  }

  bool remove()
  {
    try
    {
      return shared_memory_object::remove(map_shared_memory_name.c_str());
    }
    catch (const std::exception &ex)
    {
      return (false);
    }
  }

  std::string getLine(uint32_t n)
  {

    const uint32_t *index_data = static_cast<const uint32_t *>(region_index.get_address());
    const char *source_data = static_cast<const char *>(region_source.get_address());

    uint32_t start_pos = index_data[n];
    uint32_t end_pos = index_data[n + 1];

    return std::string(source_data + start_pos, end_pos - start_pos - 1);
  }

private:
  std::string file_full_path;
  std::string map_shared_memory_name;
  std::string index_shared_memory_name;
  mapped_region region_source;
  mapped_region region_index;

  static NAN_METHOD(Create);
  static NAN_METHOD(ArrayFrom);
  static NAN_METHOD(Exists);
  static NAN_METHOD(Remove);
  static NAN_GETTER(LengthGetter);
  static NAN_INDEX_GETTER(IndexGetter);
  static NAN_INDEX_SETTER(IndexSetter);
  static NAN_INDEX_QUERY(IndexQuery);
  static NAN_INDEX_DELETER(IndexDeleter);
  static NAN_INDEX_ENUMERATOR(IndexEnumerator);

  static v8::Local<v8::Function> init_methods(v8::Local<v8::FunctionTemplate> f_tpl);
  static inline Nan::Persistent<v8::Function> &constructor()
  {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};

class ActionWorker : public Nan::AsyncWorker
{
public:
  SharedFileView *sharedFileView;
  std::string action;

  ActionWorker(SharedFileView *sharedFileView, std::string action, Nan::Callback *callback)
      : Nan::AsyncWorker(callback)
  {

    this->sharedFileView = sharedFileView;
    this->action = action;
  }

  void Execute()
  {
    try
    {
      if (action == "create")
      {

        sharedFileView->create();
      }
      else if (action == "remove")
      {
        sharedFileView->remove();
      }
    }
    catch (const std::exception &ex)
    {
      std::cerr << ex.what();
      this->SetErrorMessage(ex.what());
      return;
    }
  }

  void HandleOKCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[] = {
        Nan::Null()};
    Nan::Call(callback->GetFunction(), Nan::GetCurrentContext()->Global(), 1, argv);
  }

  void HandleErrorCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[] = {
        Nan::New(this->ErrorMessage()).ToLocalChecked(), // return error message
    };
    Nan::Call(callback->GetFunction(), Nan::GetCurrentContext()->Global(), 1, argv);
  }
};

NAN_GETTER(SharedFileView::LengthGetter)
{
  auto self = Nan::ObjectWrap::Unwrap<SharedFileView>(info.This());

  info.GetReturnValue().Set(self->length);
}

NAN_INDEX_GETTER(SharedFileView::IndexGetter)
{

  auto self = Nan::ObjectWrap::Unwrap<SharedFileView>(info.This());

  if (index < 0 || index >= self->length)
  {
    info.GetReturnValue().SetUndefined();
    return;
  }

  std::string str = self->getLine(index);

  v8::Local<v8::String> v8str = Nan::New<v8::String>(str).ToLocalChecked();

  info.GetReturnValue().Set(v8str);
}

NAN_INDEX_SETTER(SharedFileView::IndexSetter)
{
  std::ostringstream error_stream;
  error_stream << "Cannot assign to read only property " << index << " of object '[object Array]";
  Nan::ThrowTypeError(error_stream.str().c_str());
  return;
}

NAN_INDEX_DELETER(SharedFileView::IndexDeleter)
{
  std::ostringstream error_stream;
  error_stream << "Cannot delete read only property " << index << " of object '[object Array]";
  Nan::ThrowTypeError(error_stream.str().c_str());

  return;
}

NAN_INDEX_QUERY(SharedFileView::IndexQuery)
{
  // TODO
}

NAN_INDEX_ENUMERATOR(SharedFileView::IndexEnumerator)
{
  // TODO
}

NAN_METHOD(SharedFileView::Create)
{

  if (!info[0]->IsString())
  {
    return Nan::ThrowTypeError(Nan::New("Expected string").ToLocalChecked());
  }

  std::string file_full_path = *Nan::Utf8String(info[0]);

  SharedFileView *d = new SharedFileView(file_full_path.c_str());

  Nan::AsyncQueueWorker(new ActionWorker(
      d,
      "create",
      new Nan::Callback(info[1].As<v8::Function>())));
}

NAN_METHOD(SharedFileView::ArrayFrom)
{

  if (!info[0]->IsString())
  {
    return Nan::ThrowError(Nan::New("Expected String").ToLocalChecked());
  }

  std::string file_full_path = *Nan::Utf8String(info[0]);

  try
  {

    SharedFileView *d = new SharedFileView(file_full_path.c_str());
    d->open();
    d->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }
  catch (const std::exception &ex)
  {
    info.GetReturnValue().SetUndefined();
    return;
  }
}

NAN_METHOD(SharedFileView::Exists)
{

  std::string file_full_path = *Nan::Utf8String(info[0]);

  SharedFileView *d = new SharedFileView(file_full_path.c_str());

  info.GetReturnValue().Set(d->exists());
}

NAN_METHOD(SharedFileView::Remove)
{

  std::string file_full_path = *Nan::Utf8String(info[0]);

  SharedFileView *d = new SharedFileView(file_full_path.c_str());

  Nan::AsyncQueueWorker(new ActionWorker(
      d,
      "remove",
      new Nan::Callback(info[1].As<v8::Function>())));
}

v8::Local<v8::Function> SharedFileView::init_methods(v8::Local<v8::FunctionTemplate> f_tpl)
{
  auto inst = f_tpl->InstanceTemplate();
  inst->SetInternalFieldCount(1);

  Nan::SetIndexedPropertyHandler(inst, IndexGetter, IndexSetter, IndexQuery, IndexDeleter, IndexEnumerator,
                                 Nan::New<v8::String>("instance").ToLocalChecked());

  auto fun = Nan::GetFunction(f_tpl).ToLocalChecked();
  constructor().Reset(fun);
  return fun;
}

NAN_MODULE_INIT(SharedFileView::Init)
{

  Nan::SetMethod(target, "Create", SharedFileView::Create);
  Nan::SetMethod(target, "Exists", SharedFileView::Exists);
  Nan::SetMethod(target, "Remove", SharedFileView::Remove);

  v8::Local<v8::FunctionTemplate> open_tpl = Nan::New<v8::FunctionTemplate>(ArrayFrom);
  open_tpl->SetClassName(Nan::New("SharedFileView").ToLocalChecked());
  Nan::SetAccessor(open_tpl->InstanceTemplate(), Nan::New("length").ToLocalChecked(), SharedFileView::LengthGetter);
  auto open_fun = init_methods(open_tpl);
  Nan::Set(target, Nan::New("ArrayFrom").ToLocalChecked(), open_fun);
}

NODE_MODULE(sharedfileview, SharedFileView::Init)