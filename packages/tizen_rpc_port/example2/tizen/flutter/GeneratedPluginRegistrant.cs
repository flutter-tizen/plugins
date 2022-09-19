//
// Generated file. Do not edit.
//

using System;
using System.Runtime.InteropServices;
using Tizen.Flutter.Embedding;

namespace Runner
{
    internal class GeneratedPluginRegistrant
    {
        [DllImport("flutter_plugins.so")]
        public static extern void TizenRpcPortPluginRegisterWithRegistrar(
            FlutterDesktopPluginRegistrar registrar);

        public static void RegisterPlugins(IPluginRegistry registry)
        {
            TizenRpcPortPluginRegisterWithRegistrar(
                registry.GetRegistrarForPlugin("TizenRpcPortPlugin"));
        }
    }
}
