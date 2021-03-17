using RETIRODE_APP.Models;
using Nancy.TinyIoc;
using RETIRODE_APP.Services;
using RETIRODE_APP.Views;
using SQLite;
using System;
using Xamarin.Forms;

namespace RETIRODE_APP
{
    public partial class App : Application
    {
        public App()
        {
            InitializeComponent();

            DependencyService.Register<MockDataStore>();
            DependencyService.Register<SQLiteAsyncConnection>();
            DependencyService.Register<IDataStore,SqliteDataStore>();
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
