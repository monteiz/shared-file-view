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

#if BOOST_VERSION < 105500
#pragma message("Found boost version " BOOST_PP_STRINGIZE(BOOST_LIB_VERSION))
#error mmap-object needs at least version 1_55 to maintain compatibility.
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

    u_int64_t file_size = fs::file_size(file_full_path.c_str());

    shared_memory_object::remove(map_shared_memory_name.c_str());
    shared_memory_object shm(create_only, map_shared_memory_name.c_str(), read_write);

    shm.truncate(file_size);

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

  std::string getLine(uint32_t n)
  {

    const uint32_t *index_data = static_cast<const uint32_t *>(region_index.get_address());
    const char *source_data = static_cast<const char *>(region_source.get_address());

    uint32_t start_pos = index_data[n];
    uint32_t end_pos = index_data[n + 1];

    return std::string(source_data + start_pos, end_pos - start_pos);
  }

private:
  std::string file_full_path;
  std::string map_shared_memory_name;
  std::string index_shared_memory_name;
  mapped_region region_source;
  mapped_region region_index;

  static NAN_METHOD(Create);
  static NAN_METHOD(Open);
  static NAN_METHOD(Exists);
  static NAN_METHOD(Close);
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
  friend struct CloseWorker;
};

namespace bip = boost::interprocess;

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

NAN_INDEX_ENUMERATOR(SharedFileView::IndexEnumerator)
{
  // TODO
  // info.GetReturnValue().Set(Nan::New<v8::Array>(v8::None));
}

#define INFO_METHOD(name, type, object)                               \
  NAN_METHOD(SharedFileView::name)                                    \
  {                                                                   \
    auto self = Nan::ObjectWrap::Unwrap<SharedFileView>(info.This()); \
    info.GetReturnValue().Set((type)self->object->name());            \
  }

NAN_METHOD(SharedFileView::Create)
{

  std::string file_full_path = *Nan::Utf8String(info[0]);

  SharedFileView *d = new SharedFileView(file_full_path.c_str());

  d->create();
}

NAN_METHOD(SharedFileView::Open)
{

  std::string file_full_path = *Nan::Utf8String(info[0]);

  SharedFileView *d = new SharedFileView(file_full_path.c_str());

  d->open();

  d->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(SharedFileView::Exists)
{

  std::string file_full_path = *Nan::Utf8String(info[0]);

  SharedFileView *d = new SharedFileView(file_full_path.c_str());

  info.GetReturnValue().Set(d->exists());
}

v8::Local<v8::Function> SharedFileView::init_methods(v8::Local<v8::FunctionTemplate> f_tpl)
{
  // Nan::SetPrototypeMethod(f_tpl, "close", Close);

  auto inst = f_tpl->InstanceTemplate();
  inst->SetInternalFieldCount(1);

  auto fun = Nan::GetFunction(f_tpl).ToLocalChecked();
  constructor().Reset(fun);
  return fun;
}

NAN_MODULE_INIT(SharedFileView::Init)
{
  // The mmap creator class
  v8::Local<v8::FunctionTemplate> create_tpl = Nan::New<v8::FunctionTemplate>(Create);
  create_tpl->SetClassName(Nan::New("CreateMmap").ToLocalChecked());
  Nan::SetAccessor(create_tpl->InstanceTemplate(), Nan::New("length").ToLocalChecked(), SharedFileView::LengthGetter);
  auto create_fun = init_methods(create_tpl);
  Nan::Set(target, Nan::New("Create").ToLocalChecked(), create_fun);

  // The mmap opener class
  v8::Local<v8::FunctionTemplate> open_tpl = Nan::New<v8::FunctionTemplate>(Open);
  open_tpl->SetClassName(Nan::New("OpenMmap").ToLocalChecked());
  Nan::SetAccessor(open_tpl->InstanceTemplate(), Nan::New("length").ToLocalChecked(), SharedFileView::LengthGetter);
  auto open_fun = init_methods(open_tpl);
  Nan::Set(target, Nan::New("Open").ToLocalChecked(), open_fun);

  // The mmap opener class
  v8::Local<v8::FunctionTemplate> exists_tpl = Nan::New<v8::FunctionTemplate>(Exists);
  exists_tpl->SetClassName(Nan::New("ExistsMmap").ToLocalChecked());
  auto exists_fun = init_methods(exists_tpl);
  Nan::Set(target, Nan::New("Exists").ToLocalChecked(), exists_fun);
}

NODE_MODULE(sharedfileview, SharedFileView::Init)