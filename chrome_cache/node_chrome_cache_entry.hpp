#pragma once
#include <nan.h>
#include "chrome_cache.hpp"

class NChromeCacheEntry :public Nan::ObjectWrap {
public:
	static NAN_MODULE_INIT(Init);

	NChromeCacheEntry(ChromeCacheEntry *entry);
	static Nan::Persistent<v8::Function> constructor;

	ChromeCacheEntry *entry_;
	~NChromeCacheEntry();
private:
	//node.js constructor
	static NAN_METHOD(New) {};
	//node.js prototype method
	static NAN_METHOD(Save);
	static NAN_METHOD(GetHeader);

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

NChromeCacheEntry::NChromeCacheEntry(ChromeCacheEntry* entry) :entry_(entry) {}
NChromeCacheEntry::~NChromeCacheEntry() {}

NAN_METHOD(NChromeCacheEntry::Save) {
	string path = info[0]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[0].As<v8::String>()));
	NChromeCacheEntry*obj = Nan::ObjectWrap::Unwrap<NChromeCacheEntry>(info.Holder());
	v8::Isolate* isolate = info.GetIsolate();
	try {
		obj->entry_->save(path);
	} catch (const std::exception& e) {
		isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, e.what())));
	}
}

NAN_METHOD(NChromeCacheEntry::GetHeader) {
	v8::Isolate* isolate = info.GetIsolate();
	NChromeCacheEntry* obj = Nan::ObjectWrap::Unwrap<NChromeCacheEntry>(info.Holder());
	HttpHeader* header = obj->entry_->get_header_ptr();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> object = v8::Object::New(isolate);
	v8::Local<v8::Object> headers = v8::Object::New(isolate);
	auto headers_ = header->headers;
	//for (pair<string, string> p : header->headers) {
	for (auto p = headers_.begin(); p != headers_.end(); ++p) {
		headers->Set(context, v8::String::NewFromUtf8(isolate, p->first.c_str()), v8::String::NewFromUtf8(isolate, p->second.c_str()));
	}
	object->Set(context, v8::String::NewFromUtf8(isolate, "status_code"), Nan::New(header->status_code));
	object->Set(context, v8::String::NewFromUtf8(isolate, "status_source"), v8::String::NewFromUtf8(isolate, header->status_source.c_str()));
	object->Set(context, v8::String::NewFromUtf8(isolate, "protocol"), v8::String::NewFromUtf8(isolate, header->protocol.c_str()));
	object->Set(context, v8::String::NewFromUtf8(isolate, "headers"), headers);
	delete header;
	info.GetReturnValue().Set(object);
}
