#include "langcad/api/ApiServer.hpp"
#include "langcad/llm/OllamaClient.hpp"

#include <cstdlib>
#include <iostream>

namespace {

int portFromEnvironment() {
    const char* value = std::getenv("LANGCAD_API_PORT");

    if (!value) {
        return 8080;
    }

    return std::stoi(value);
}

} // namespace

int main() {
    try {
        int port = portFromEnvironment();
        auto ollama_config = langcad::llm::ollamaConfigFromEnvironment();

        langcad::api::ApiServer server(ollama_config);

        std::cout << "langcad_api listening on http://127.0.0.1:" << port << '\n';
        std::cout << "ollama url: " << ollama_config.base_url << ", model: " << ollama_config.model << '\n';
        std::cout << "this process stays open and serves the API only; it does not open the UI\n";
        std::cout << "web UI: open another terminal, run `cd ui-test && python3 -m http.server 5173`, then browse http://127.0.0.1:5173\n";
        std::cout << "API smoke tests also run from another terminal: tests/api_prompt_injection.sh\n";

        server.listen("127.0.0.1", port);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
