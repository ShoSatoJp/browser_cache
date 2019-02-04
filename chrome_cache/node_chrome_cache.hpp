#pragma once
#include <nan.h>
#include "chrome_cache.hpp"
#include "node_chrome_cache_entry.hpp"

class NChromeCache :public Nan::ObjectWrap {
public:
	static NAN_MODULE_INIT(Init);

private:
	explicit NChromeCache(string cache_dir, string temp_cache_dir, bool update_index = true);
	~NChromeCache();
	//node.js constructor
	static NAN_METHOD(New);

	//node.js prototype method
	static NAN_METHOD(Keys);
	static NAN_METHOD(FindSave);
	static NAN_METHOD(Find);
	static NAN_METHOD(GetHeader);

	static Nan::Persistent<v8::Function> constructor;
	ChromeCache *cc_;
};

Nan::Persistent<v8::Function> NChromeCache::constructor;

NAN_MODULE_INIT(NChromeCache::Init) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("ChromeCache").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	SetPrototypeMethod(tpl, "keys", Keys);
	SetPrototypeMethod(tpl, "find_save", FindSave);
	SetPrototypeMethod(tpl, "find", Find);
	SetPrototypeMethod(tpl, "get_header", GetHeader);

	constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("ChromeCache").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NChromeCache::NChromeCache(string cache_dir, string temp_cache_dir, bool update_index) :
	cc_(new ChromeCache(cache_dir, temp_cache_dir, update_index)) {}
NChromeCache::~NChromeCache() {
	delete cc_;
}

NAN_METHOD(NChromeCache::New) {
	if (info.IsConstructCall()) {
		string cache_dir = info[0]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[0].As<v8::String>()));
		string temp_cache_dir = info[1]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[1].As<v8::String>()));
		bool update_index = info[2]->IsUndefined() ? true : Nan::To<bool>(info[2]).FromJust();
		NChromeCache* obj = new NChromeCache(cache_dir, temp_cache_dir, update_index);
		obj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	} else {
		const int argc = 3;
		v8::Local<v8::Value> argv[argc] = { info[0],info[1],info[2] };
		v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
		info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
	}
}

NAN_METHOD(NChromeCache::Keys) {
	NChromeCache* obj = Nan::ObjectWrap::Unwrap<NChromeCache>(info.Holder());
	vector<string> keys = obj->cc_->keys();
	size_t length = keys.size();
	v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(length);
	for (int i = 0; i < length; i++)
		jsArray->Set(i, Nan::New(keys.at(i)).ToLocalChecked());
	info.GetReturnValue().Set(jsArray);
}


NAN_METHOD(NChromeCache::FindSave) {
	string key = info[0]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[0].As<v8::String>()));
	string path = info[1]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[1].As<v8::String>()));
	NChromeCache* obj = Nan::ObjectWrap::Unwrap<NChromeCache>(info.Holder());
	v8::Isolate* isolate = info.GetIsolate();
	try {
		obj->cc_->find_save(key, path);
	} catch (const std::exception& e) {
		isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, e.what())));
	}
}

NAN_METHOD(NChromeCache::Find) {
	string key = info[0]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[0].As<v8::String>()));
	NChromeCache* obj = Nan::ObjectWrap::Unwrap<NChromeCache>(info.Holder());
	v8::Isolate* isolate = info.GetIsolate();
	try {
		v8::EscapableHandleScope scope(isolate);
		v8::Handle<v8::Value> argv[] = { v8::Boolean::New(isolate,true) };
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(isolate, NChromeCacheEntry::constructor);
		v8::Handle<v8::Object> object = cons->NewInstance(context, 1, argv).ToLocalChecked();

		NChromeCacheEntry* n = NChromeCacheEntry::Unwrap<NChromeCacheEntry>(object);
		n->entry_ = obj->cc_->find_map_ptr(key);

		info.GetReturnValue().Set(scope.Escape(object));
	} catch (const std::exception& e) {
		isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, e.what())));
	}
}

NAN_METHOD(NChromeCache::GetHeader) {
	string key = info[0]->IsUndefined() ? "" : string(*v8::String::Utf8Value(info[0].As<v8::String>()));
	NChromeCache* obj = Nan::ObjectWrap::Unwrap<NChromeCache>(info.Holder());
	v8::Isolate* isolate = info.GetIsolate();
	try {
		//v8::EscapableHandleScope scope(isolate);

		ChromeCacheEntry * entry= obj->cc_->find_map_ptr(key);
		HttpHeader* header = entry->get_header_ptr();
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


		//info.GetReturnValue().Set(scope.Escape(object));
		info.GetReturnValue().Set(object);
	} catch (const std::exception& e) {
		isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, e.what())));
	}
}

