//
// Generated file. Do not edit.
//

using System;
using System.Runtime.InteropServices;
using Tizen.Flutter.Embedding;

internal class GeneratedPluginRegistrant
{
    [DllImport("flutter_plugins.so")]
    public static extern void IntegrationTestPluginRegisterWithRegistrar(
        FlutterDesktopPluginRegistrar registrar);

    [DllImport("flutter_plugins.so")]
    public static extern void TizenRpcPortPluginRegisterWithRegistrar(
        FlutterDesktopPluginRegistrar registrar);

    public static void RegisterPlugins(IPluginRegistry registry)
    {
        IntegrationTestPluginRegisterWithRegistrar(
            registry.GetRegistrarForPlugin("IntegrationTestPlugin"));
        TizenRpcPortPluginRegisterWithRegistrar(
            registry.GetRegistrarForPlugin("TizenRpcPortPlugin"));
    }
}
