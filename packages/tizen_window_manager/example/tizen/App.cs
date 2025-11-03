using Tizen.Flutter.Embedding;

namespace Runner
{
    public class App : FlutterApplication
    {
        protected override void OnCreate()
        {
            WindowOffsetX = 100;
            WindowOffsetY = 100;
            WindowHeight = 700;
            WindowWidth = 1000;
            base.OnCreate();

            GeneratedPluginRegistrant.RegisterPlugins(this);
        }

        static void Main(string[] args)
        {
            var app = new App();
            app.Run(args);
        }
    }
}
