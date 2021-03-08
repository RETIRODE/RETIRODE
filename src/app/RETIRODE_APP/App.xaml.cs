using RETIRODE_APP.Services;
using RETIRODE_APP.Views;
using System;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace RETIRODE_APP
{
    public partial class App : Application
    {

        public App()
        {
            InitializeComponent();

            DependencyService.Register<MockDataStore>();
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
