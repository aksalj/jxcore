// Copyright Fedor Indutny and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef DEPS_DEBUGGER_AGENT_INCLUDE_DEBUGGER_AGENT_H_
#define DEPS_DEBUGGER_AGENT_INCLUDE_DEBUGGER_AGENT_H_

#include "uv.h"
#include "v8.h"
#include "v8-debug.h"

namespace node {

// Forward declaration
class commons;

namespace debugger {

// Forward declaration
class AgentMessage;

class Agent {
 public:
  explicit Agent(commons* com);
  ~Agent();

  typedef void (*DispatchHandler)(commons* com);

  // Start the debugger agent thread
  bool Start(int port, bool wait);
  // Listen for debug events
  void Enable();
  // Stop the debugger agent
  void Stop();

  inline void set_dispatch_handler(DispatchHandler handler) {
    dispatch_handler_ = handler;
  }

  inline commons* parent_env() const { return parent_env_; }
  inline commons* child_env() const { return child_env_; }

 protected:
  void InitAdaptor(commons* com);

  // Worker body
  void WorkerRun();

  static void ThreadCb(Agent* agent);
  static void ParentSignalCb(uv_async_t* signal);
  static void ChildSignalCb(uv_async_t* signal);
  static void MessageHandler(const v8::Debug::Message& message);

  // V8 API
  static Agent* Unwrap(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void NotifyListen(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void NotifyWait(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SendCommand(const v8::FunctionCallbackInfo<v8::Value>& args);

  void EnqueueMessage(AgentMessage* message);

  enum State {
    kNone,
    kRunning
  };

  // TODO(indutny): Verify that there are no races
  State state_;

  int port_;
  bool wait_;

  uv_sem_t start_sem_;
  uv_mutex_t message_mutex_;
  uv_async_t child_signal_;

  uv_thread_t thread_;
  commons* parent_env_;
  commons* child_env_;
  uv_loop_t child_loop_;
  v8::Persistent<v8::Object> api_;

  // QUEUE
  void* messages_[2];

  DispatchHandler dispatch_handler_;
};

}  // namespace debugger
}  // namespace node

#endif  // DEPS_DEBUGGER_AGENT_INCLUDE_DEBUGGER_AGENT_H_
