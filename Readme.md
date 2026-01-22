# PingNode Plugin
## Overview

**PingNode Plugin** is designed for periodic availability checks of network nodes using ICMP (ping).
The plugin supports both direct IP addresses and domain names (URLs). If a domain name is provided, it will be resolved to an IP address automatically during plugin initialization.

The plugin is loaded and managed by the PluginLoader system.

## Configuration

The configuration file must be located at the following relative path: `./configs/PingNode.json` (relative to the directory where `PluginLoader` is executed).

## Global Parameters

| Parameter           | Description                                     |
| ------------------- | ----------------------------------------------- |
| `ping_interval_sec` | Interval between ping requests, in seconds      |
| `timeout_ms`        | ICMP response timeout, in milliseconds          |
| `icmp_payload`      | Custom ICMP payload represented as a hex string |
| `nodes`             | List of nodes to be monitored                   |

## Node Configuration (nodes)

For each node, you must specify either `ip` or `url`:

- If the IP address is known, set the `ip` field and leave `url` empty.

- If the IP address is unknown but a domain name is available, set the `url` field and leave `ip` empty. In this case, the plugin will attempt to resolve the IP address via DNS during startup.

```json
{
    "ping_interval_sec": "15",
    "timeout_ms": "1000",
    "icmp_payload": "101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f3031323334353637",
    "nodes": [
        {
            "name": "Local",
            "url": "localhost",
            "ip": ""
        },
        {
            "name": "Router",
            "url": "",
            "ip": "192.168.0.1"
        },
        {
            "name": "Google",
            "url": "google.com",
            "ip": ""
        }
    ]
}
```

## Startup and DNS Resolution

When PluginLoader starts, the following steps are performed:

1. Plugins are loaded from the ./Plugins directory

2. The PingNode.json configuration file is read

3. For each node with an empty ip field, the plugin attempts to resolve the IP address using the specified url

4. The DNS resolution results are written to the log

## Example Startup Output

```bash
./PluginLoader
============================================================
d3156::PluginCore - Plugin Loader System
------------------------------------------------------------
Version     : version
Start Time  : date
Hostname    : host
OS          : Linux host Thu Nov 20 10:25:38 UTC 2 (x86_64)
Executable  : ./PluginLoader
============================================================

[CORE] Using standard plugin path ./Plugins
[CORE] Loading plugins from dir "./Plugins"
[CORE] Plugin PingNodePlugin loaded
[CORE] Plugin PrometheusExporterPlugin loaded
[CORE] Model registered successfully [Delete order 10000] MetricsModel
[CORE] Model registered successfully [Delete order 0] PingNodeModel
[PingNodeModel] Trying to resolve IP by URL: localhost
[PingNodeModel] IP resolved: 127.0.0.1
[PingNodeModel] Trying to resolve IP by URL: google.com
[PingNodeModel] IP resolved: 172.253.130.101
```
## Notes

- DNS resolution is performed once during plugin initialization.

- If DNS resolution fails, the corresponding node will not be pinged correctly.

- Timeout and interval values should be chosen based on network latency and expected load.

## Metrics Integration

The PingNode Plugin is integrated with [d3156/MetricsModel](https://github.com/d3156/MetricsModel), which provides a common metrics interface for interaction with metric exporter plugins.

[MetricsModel](https://github.com/d3156/MetricsModel) implements the d3156::PluginCore::IModel interface and acts as a shared abstraction layer between metric producers (such as PingNode Plugin) and metric consumers (exporter plugins).

### MetricsModel Overview

- [d3156/MetricsModel](https://github.com/d3156/MetricsModel) is a centralized model used to collect and expose runtime metrics

- Plugins publish metrics to [MetricsModel](https://github.com/d3156/MetricsModel)

- Exporter plugins consume metrics from [MetricsModel](https://github.com/d3156/MetricsModel) and expose them to external monitoring systems

This design decouples metric generation from metric export and allows multiple exporters to coexist without changes to producer plugins.

### Prometheus Export Example

An example consumer of [MetricsModel](https://github.com/d3156/MetricsModel) is the [d3156/PrometheusExporter](https://github.com/d3156/PrometheusExporterPlugin) plugin.

#### The Prometheus exporter:

- Reads metrics from [MetricsModel](https://github.com/d3156/MetricsModel)

- Translates them into Prometheus-compatible format

- Exposes them via an HTTP endpoint for scraping by Prometheus

When both plugins are enabled: `PingNodePlugin → MetricsModel → PrometheusExporter → Prometheus`

## Model Interface and Integration

The PingNode Plugin exposes its runtime state via a shared model, allowing other plugins to access ping results programmatically.

### PingNodeModel Interface

The plugin registers a model named PingNodeModel, which implements the d3156::PluginCore::IModel interface.

```c++
class PingNodeModel final : public d3156::PluginCore::IModel
{
public:
    struct Node {
        const std::string name        = "host";
        const std::string url         = "localhost";
        const std::string ip          = "127.0.0.1";
        std::atomic<uint32_t> latency = 0;
        std::atomic<bool> available   = false;
    };

    const std::vector<std::unique_ptr<Node>> &get_nodes();
};

```
### Accessing the Model from Another Plugin

Any other plugin can access the PingNodeModel through the shared model registry.

Example model registration and access:

```c++
void MyPlugin::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    model = RegisterModel(
        "PingNodeModel",
        new PingNodeModel(),
        PingNodeModel
    );
}
```
Once registered, the plugin can retrieve the list of nodes and read their current status:

```c++
const auto &nodes = model->get_nodes();

for (const auto &node : nodes) {
    bool is_up = node->available.load();
    uint32_t rtt = node->latency.load();
}
```
