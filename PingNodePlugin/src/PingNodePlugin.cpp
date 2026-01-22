#include <PluginCore/IPlugin.hpp>
#include <PluginCore/IModel.hpp>

#include <PingNodeModel.hpp>

class PingNodePlugin final : public d3156::PluginCore::IPlugin {
public:
    void registerArgs(d3156::Args::Builder& bldr) override {
        // TODO: bldr.addOption(...) / bldr.addParam(...) / bldr.addFlag(...)
        (void)bldr;
    }

    void registerModels(d3156::PluginCore::ModelsStorage& models) override {
        models.registerModel(new PingNodeModel());
        (void)models;
    }

    void postInit() override {
        // TODO: start threads / async jobs here if needed
    }
};

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin* create_plugin() {
    return new PingNodePlugin();
}

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin* p) {
    delete p;
}
