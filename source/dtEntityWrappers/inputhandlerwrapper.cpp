/*
* dtEntity Game and Simulation Engine
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Martin Scheffler
*/

#include <dtEntity/inputhandler.h>
#include <dtEntity/applicationcomponent.h>
#include <dtEntityWrappers/inputhandlerwrapper.h>
#include <dtEntityWrappers/wrappermanager.h>
#include <dtEntityWrappers/v8helpers.h>
#include <dtEntity/basemessages.h>
#include <iostream>
#include <v8.h>

using namespace v8;

namespace dtEntityWrappers
{

   ////////////////////////////////////////////////////////////////////////////////
   class InputCallbackWrapper
      : public dtEntity::InputCallbackInterface
   {
   public:

      InputCallbackWrapper(Handle<Object> obj)
      {
         mObject = Persistent<Object>::New(obj);
         
         if(obj->Has(String::New("keyDown"))) 
         {
            mKeyDownFunc = Persistent<Function>::New(Handle<Function>::Cast(obj->Get(String::New("keyDown"))));
         }
         if(obj->Has(String::New("keyUp"))) 
         {
            mKeyUpFunc = Persistent<Function>::New(Handle<Function>::Cast(obj->Get(String::New("keyUp"))));
         }
         if(obj->Has(String::New("mouseButtonDown"))) 
         {
            mMouseDownFunc = Persistent<Function>::New(Handle<Function>::Cast(obj->Get(String::New("mouseButtonDown"))));
         }
         if(obj->Has(String::New("mouseButtonUp"))) 
         {
            mMouseUpFunc = Persistent<Function>::New(Handle<Function>::Cast(obj->Get(String::New("mouseButtonUp"))));
         }
         if(obj->Has(String::New("mouseWheel"))) 
         {
            mMouseWheelFunc = Persistent<Function>::New(Handle<Function>::Cast(obj->Get(String::New("mouseWheel"))));
         }
         if(obj->Has(String::New("mouseMove"))) 
         {
            mMouseMoveFunc = Persistent<Function>::New(Handle<Function>::Cast(obj->Get(String::New("mouseMove"))));
         }
      }

      virtual void KeyUp(const std::string& name, bool handled) {
         if(!mKeyUpFunc.IsEmpty())
         {
            Context::Scope context_scope(GetGlobalContext());
            HandleScope scope;
            TryCatch try_catch;
            Handle<Value> argv[2] = { String::New(name.c_str()), Boolean::New(handled) };
            Handle<Value> ret = mKeyUpFunc->Call(mObject, 2, argv);

            if(ret.IsEmpty()) 
            {
               ReportException(&try_catch);
            }
         }
      }

      virtual void KeyDown(const std::string& name, bool handled)
      {
         if(!mKeyDownFunc.IsEmpty())
         {
            Context::Scope context_scope(GetGlobalContext());
            HandleScope scope;
            TryCatch try_catch;
            Handle<Value> argv[2] = { String::New(name.c_str()), Boolean::New(handled) };
            Handle<Value> ret = mKeyDownFunc->Call(mObject, 2, argv);

            if(ret.IsEmpty()) 
            {
               ReportException(&try_catch);
            }
         }
      }

      virtual void MouseButtonUp(int button, bool handled)
      {
         if(!mMouseUpFunc.IsEmpty())
         {
            Context::Scope context_scope(GetGlobalContext());
            HandleScope scope;
            TryCatch try_catch;
            Handle<Value> argv[2] = { Integer::New(button), Boolean::New(handled) };
            Handle<Value> ret = mMouseUpFunc->Call(mObject, 2, argv);

            if(ret.IsEmpty()) 
            {
               ReportException(&try_catch);
            }
         }
      }

      virtual void MouseButtonDown(int button, bool handled)
      {
         if(!mMouseDownFunc.IsEmpty())
         {
            Context::Scope context_scope(GetGlobalContext());
            HandleScope scope;
            TryCatch try_catch;
            Handle<Value> argv[2] = { Integer::New(button), Boolean::New(handled) };
            Handle<Value> ret = mMouseDownFunc->Call(mObject, 2, argv);

            if(ret.IsEmpty()) 
            {
               ReportException(&try_catch);
            }
         }
      }

      virtual void MouseWheel(int dir, bool handled)
      {
         if(!mMouseWheelFunc.IsEmpty())
         {
            Context::Scope context_scope(GetGlobalContext());
            HandleScope scope;
            TryCatch try_catch;
            Handle<Value> argv[2] = { Integer::New(dir), Boolean::New(handled) };
            Handle<Value> ret = mMouseWheelFunc->Call(mObject, 2, argv);

            if(ret.IsEmpty()) 
            {
               ReportException(&try_catch);
            }
         }
      }

      virtual void MouseMove(float x, float y, bool handled)
      {
         if(!mMouseMoveFunc.IsEmpty())
         {
            Context::Scope context_scope(GetGlobalContext());
            HandleScope scope;
            TryCatch try_catch;
            Handle<Value> argv[3] = { Number::New(x), Number::New(y), Boolean::New(handled) };
            Handle<Value> ret = mMouseMoveFunc->Call(mObject, 3, argv);

            if(ret.IsEmpty()) 
            {
               ReportException(&try_catch);
            }
         }
      }

      Persistent<Function> mKeyUpFunc;
      Persistent<Function> mKeyDownFunc;
      Persistent<Function> mMouseUpFunc;
      Persistent<Function> mMouseDownFunc;
      Persistent<Function> mMouseWheelFunc;
      Persistent<Function> mMouseMoveFunc;
      Persistent<Object> mObject;
   };

   std::vector<InputCallbackWrapper*> s_inputCallbackWrappers;

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetKey(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Boolean::New(input->GetKey(ToStdString(args[0])));
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetKeyUp(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Boolean::New(input->GetKeyUp(ToStdString(args[0])));
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetKeyDown(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Boolean::New(input->GetKeyDown(ToStdString(args[0])));
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHAnyKey(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Boolean::New(input->AnyKeyDown());
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetInputString(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return String::New(input->GetInputString().c_str());
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetMouseButton(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Boolean::New(input->GetMouseButton(args[0]->Int32Value()));
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetMouseButtonUp(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Boolean::New(input->GetMouseButtonUp(args[0]->Int32Value()));
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetMouseButtonDown(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Boolean::New(input->GetMouseButtonDown(args[0]->Int32Value()));
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetAxis(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Number::New(input->GetAxis(args[0]->Uint32Value()));
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetMouseWheelState(const Arguments& args)
   {
      dtEntity::InputHandler* input = UnwrapInputHandler(args.Holder());
      return Integer::New((int)input->GetMouseWheelState());
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetMultiTouchEnabled(const Arguments& args)
   {
      dtEntity::InputHandler* ih; GetInternal(args.This(), 0, ih);
      return Boolean::New(ih->GetMultiTouchEnabled());
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetNumTouches(const Arguments& args)
   {
      dtEntity::InputHandler* ih; GetInternal(args.This(), 0, ih);
      return Integer::New(ih->GetNumTouches());
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHGetTouches(const Arguments& args)
   {
      dtEntity::InputHandler* ih; GetInternal(args.This(), 0, ih);
      HandleScope scope;
      Handle<Array> jstouches = Array::New();

      Handle<String> tapcount = String::New("tapcount");
      Handle<String> id = String::New("id");
      Handle<String> x = String::New("x");
      Handle<String> y = String::New("y");
      Handle<String> phase = String::New("phase");

      const std::vector<dtEntity::TouchPoint>& touches = ih->GetTouches();
      unsigned int count = 0;
      for(std::vector<dtEntity::TouchPoint>::const_iterator i = touches.begin(); i != touches.end(); ++i)
      {
         dtEntity::TouchPoint tp = *i;
         Handle<Object> o = Object::New();
         o->Set(id, Uint32::New(tp.mId));
         o->Set(tapcount, Uint32::New(tp.mTapCount));
         o->Set(x, Number::New(tp.mX));
         o->Set(y, Number::New(tp.mY));
         o->Set(phase, Integer::New(tp.mTouchPhase));
         jstouches->Set(count++, o);
      }

      return scope.Close(jstouches);
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHToString(const Arguments& args)
   {
      return String::New("<Input>");
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHAddInputCallback(const Arguments& args)
   {
      Handle<Object> o = Handle<Object>::Cast(args[0]);
      InputCallbackWrapper* wr = new InputCallbackWrapper(o);

      dtEntity::InputHandler* ih; GetInternal(args.This(), 0, ih);
      ih->AddInputCallback(wr);
      s_inputCallbackWrappers.push_back(wr);

      return Undefined();
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHRemoveInputCallback(const Arguments& args)
   {
      for(std::vector<InputCallbackWrapper*>::iterator i = s_inputCallbackWrappers.begin();
         i != s_inputCallbackWrappers.end(); ++i)
      {
         if((*i)->mObject == args[0])
         {
            dtEntity::InputHandler* ih; GetInternal(args.This(), 0, ih);
            ih->RemoveInputCallback(*i);
            return True();
         }
      }
      
      return False();
   }

   ////////////////////////////////////////////////////////////////////////////////
   Handle<Value> IHPrintKeys(const Arguments& args)
   {
      dtEntity::InputHandler* ih; GetInternal(args.This(), 0, ih);
      const dtEntity::InputHandler::KeyNames k = ih->GetKeyNames();
      
      dtEntity::InputHandler::KeyNames::const_iterator i;
      for(i = k.begin(); i != k.end(); ++i)
      {
         std::cout << i->first << "\n";         
      }
      return Undefined();
   }


   ////////////////////////////////////////////////////////////////////////////////
   v8::Handle<v8::Object> WrapInputHandler(dtEntity::InputHandler* v)
   {
      HandleScope handle_scope;
      Handle<Context> context = GetGlobalContext();
      Context::Scope context_scope(context);

      Handle<FunctionTemplate> templt = FunctionTemplate::New();
      templt->SetClassName(String::New("Input"));
      templt->InstanceTemplate()->SetInternalFieldCount(1);

      Handle<ObjectTemplate> proto = templt->PrototypeTemplate();

      proto->Set("getAxis", FunctionTemplate::New(IHGetAxis));
      proto->Set("getKey", FunctionTemplate::New(IHGetKey));
      proto->Set("getKeyDown", FunctionTemplate::New(IHGetKeyDown));
      proto->Set("getKeyUp", FunctionTemplate::New(IHGetKeyUp));
      proto->Set("anyKeyDown", FunctionTemplate::New(IHAnyKey));

      proto->Set("getInputString", FunctionTemplate::New(IHGetInputString));

      proto->Set("getMouseButton", FunctionTemplate::New(IHGetMouseButton));
      proto->Set("getMouseButtonUp", FunctionTemplate::New(IHGetMouseButtonUp));
      proto->Set("getMouseButtonDown", FunctionTemplate::New(IHGetMouseButtonDown));
      proto->Set("getMouseWheelState", FunctionTemplate::New(IHGetMouseWheelState));
      
      proto->Set("getMultiTouchEnabled", FunctionTemplate::New(IHGetMultiTouchEnabled));
      proto->Set("getNumTouches", FunctionTemplate::New(IHGetNumTouches));
      proto->Set("getTouches", FunctionTemplate::New(IHGetTouches));
      proto->Set("printKeys", FunctionTemplate::New(IHPrintKeys));
      proto->Set("toString", FunctionTemplate::New(IHToString));
      proto->Set("addInputCallback", FunctionTemplate::New(IHAddInputCallback));
      proto->Set("removeInputCallback", FunctionTemplate::New(IHRemoveInputCallback));
      
      Local<Object> instance = templt->GetFunction()->NewInstance();
      instance->SetInternalField(0, External::New(v));

      return handle_scope.Close(instance);
   }

   ////////////////////////////////////////////////////////////////////////////////
   dtEntity::InputHandler* UnwrapInputHandler(Handle<Value> val)
   {
      Handle<Object> obj = Handle<Object>::Cast(val);
      dtEntity::InputHandler* v;
      GetInternal(obj, 0, v);
      return v;      
   }

   ////////////////////////////////////////////////////////////////////////////////
   v8::Handle<v8::Object> WrapTouchPhases()
   {
      HandleScope scope;
      Handle<Object> obj = Object::New();
      obj->Set(String::New("Unknown"), Integer::New(dtEntity::TouchPhase::UNKNOWN));
      obj->Set(String::New("Began"), Integer::New(dtEntity::TouchPhase::BEGAN));
      obj->Set(String::New("Moved"), Integer::New(dtEntity::TouchPhase::MOVED));
      obj->Set(String::New("Stationary"), Integer::New(dtEntity::TouchPhase::STATIONARY));
      obj->Set(String::New("Ended"), Integer::New(dtEntity::TouchPhase::ENDED));
      return scope.Close(obj);
   }

   ////////////////////////////////////////////////////////////////////////////////
   v8::Handle<v8::Object> WrapKeys(dtEntity::InputHandler* ih)
   {
      const dtEntity::InputHandler::KeyNames& keynames = ih->GetKeyNames();
      dtEntity::InputHandler::KeyNames::const_iterator i;
      
      HandleScope scope;
      Handle<Object> obj = Object::New();
      
      for(i = keynames.begin(); i != keynames.end(); ++i)
      {
         obj->Set(String::New(i->first.c_str()), Integer::New(i->second));
      }

      return scope.Close(obj);
   }

   ////////////////////////////////////////////////////////////////////////////////
   v8::Handle<v8::Object> WrapAxes(dtEntity::InputHandler* ih)
   {
            
      HandleScope scope;
      Handle<Object> obj = Object::New();
      
      obj->Set(String::New("MouseX"), Uint32::New(dtEntity::InputHandler::MouseXId));
      obj->Set(String::New("MouseY"), Uint32::New(dtEntity::InputHandler::MouseYId));
      obj->Set(String::New("MouseXRaw"), Uint32::New(dtEntity::InputHandler::MouseXRawId));
      obj->Set(String::New("MouseYRaw"), Uint32::New(dtEntity::InputHandler::MouseYRawId));
      obj->Set(String::New("MouseDeltaX"), Uint32::New(dtEntity::InputHandler::MouseDeltaXId));
      obj->Set(String::New("MouseDeltaY"), Uint32::New(dtEntity::InputHandler::MouseDeltaYId));
      obj->Set(String::New("MouseDeltaXRaw"), Uint32::New(dtEntity::InputHandler::MouseDeltaXRawId));
      obj->Set(String::New("MouseDeltaYRaw"), Uint32::New(dtEntity::InputHandler::MouseDeltaYRawId));
      
      return scope.Close(obj);
   }

   ////////////////////////////////////////////////////////////////////////////////
   v8::Handle<v8::Object> WrapMouseWheelStates()
   {            
      HandleScope scope;
      Handle<Object> obj = Object::New();
      
      obj->Set(String::New("None"), Uint32::New(osgGA::GUIEventAdapter::SCROLL_NONE));
      obj->Set(String::New("Left"), Uint32::New(osgGA::GUIEventAdapter::SCROLL_LEFT));
      obj->Set(String::New("Right"), Uint32::New(osgGA::GUIEventAdapter::SCROLL_RIGHT));
      obj->Set(String::New("Up"), Uint32::New(osgGA::GUIEventAdapter::SCROLL_UP));
      obj->Set(String::New("Down"), Uint32::New(osgGA::GUIEventAdapter::SCROLL_DOWN));
      obj->Set(String::New("2D"), Uint32::New(osgGA::GUIEventAdapter::SCROLL_2D));
      
      return scope.Close(obj);
   }
}