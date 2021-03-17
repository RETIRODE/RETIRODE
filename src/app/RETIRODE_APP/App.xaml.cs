using Nancy.TinyIoc;
using RETIRODE_APP.Services;
using Xamarin.Forms;

namespace RETIRODE_APP
{
    public partial class App : Application
    {

        public App()
        {
            InitializeComponent();

            TinyIoCContainer.Current.Register<MockDataStore>();
            MainPage = new AppShell();
        }

        protected override void OnStart()
        {
        }

        protected override void OnSleep()
        {
        }

        protected override void OnResume()
        {
        }
    }
}
