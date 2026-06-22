#pragma once

#include "langcad/api/StructuredLogger.hpp"
#include "langcad/geometry/Shape3D.hpp"
#include "langcad/llm/OllamaClient.hpp"

#include <httplib.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace langcad::api {

class ApiServer {
public:
    explicit ApiServer(langcad::llm::OllamaConfig ollama_config);
    ~ApiServer();

    void listen(const char* host, int port);

private:
    httplib::Server server_;
    StructuredLogger logger_;
    langcad::llm::OllamaConfig ollama_config_;
    std::mutex render_mutex_;
    std::condition_variable render_cv_;
    std::unique_ptr<geometry::Shape3D> pending_render_;
    std::shared_ptr<std::atomic_bool> render_stop_;
    bool stopping_;

    void registerRoutes();
    void launchRender(std::unique_ptr<geometry::Shape3D> shape);
    void stopActiveRender();
    void runRenderLoopOnMainThread();
};

} // namespace langcad::api
