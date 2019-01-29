#pragma once
#include <nan.h>
#include "chrome_cache.hpp"

class NChromeCacheEntry :public Nan::ObjectWrap {
public:
	static NAN_MODULE_INIT(Init);

private:
	explicit NChromeCacheEntry(ChromeCacheEntry *entry);
	~NChromeCacheEntry();
	//node.js constructor
	static NAN_METHOD(New);
	static NAN_METHOD(NewInstance);
	//node.js prototype method
	static NAN_METHOD(Save);
	static NAN_METHOD(GetHeader);

	static Nan::Persistent<v8::Function> constructor;
	ChromeCacheEntry *entry_;
};

Nan::Persistent<v8::Function> NChromeCacheEntry::constructor;

NAN_MODULE_INIT(NChromeCacheEntry::Init) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("ChromeCacheEntry").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	SetPrototypeMethod(tpl, "save", Save);
	SetPrototypeMethod(tpl, "get_header", GetHeader);

	constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("ChromeCacheEntry").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NChromeCacheEntry::NChromeCacheEntry(ChromeCacheEntry *entry) :entry_(entry) {}
NChromeCacheEntry::~NChromeCacheEntry() {}

NAN_METHOD(NChromeCacheEntry::New) {
	string cache_dir = info[0]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[0].As<v8::String>()));
	string temp_cache_dir = info[1]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[1].As<v8::String>()));
	bool update_index = info[2]->IsUndefined() ? true : Nan::To<bool>(info[2]).FromJust();
	NChromeCacheEntry* obj = new NChromeCacheEntry(cache_dir, temp_cache_dir, update_index);
	obj->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NChromeCacheEntry::NewInstance) {
	v8::Isolate* isolate = info.GetIsolate();

	const unsigned argc = 3;
	v8::Local<v8::Value> argv[argc] = { info[0],info[1],info[2] };
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(isolate, constructor);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

	info.GetReturnValue().Set(instance);
}

NAN_METHOD(NChromeCacheEntry::Save) {
	string path = info[0]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[0].As<v8::String>()));
	NChromeCacheEntry* obj = Nan::ObjectWrap::Unwrap<NChromeCacheEntry>(info.Holder());
	v8::Isolate* isolate = info.GetIsolate();
	try {
		obj->entry_->save(path);
	} catch (const std::exception& e) {
		isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, e.what())));
	}
}

NAN_METHOD(NChromeCacheEntry::GetHeader) {
	NChromeCacheEntry* obj = Nan::ObjectWrap::Unwrap<NChromeCacheEntry>(info.Holder());
	unique_ptr<HttpHeader> header = obj->entry_->get_header();
	v8::Isolate* isolate = info.GetIsolate();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> object = v8::Object::New(isolate);
	object->Set(context, v8::String::NewFromUtf8(isolate, "status_code"), Nan::New(header->status_code));
	object->Set(context, v8::String::NewFromUtf8(isolate, "status_source"), v8::String::NewFromUtf8(isolate, header->status_source.c_str()));
	object->Set(context, v8::String::NewFromUtf8(isolate, "protocol"), v8::String::NewFromUtf8(isolate, header->protocol.c_str()));
	info.GetReturnValue().Set(object);

}

