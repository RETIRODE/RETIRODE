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

            TinyIoCContainer.Current.Register<IMockDataStore<Item>,MockDataStore>();
            TinyIoCContainer.Current.Register<SQLiteAsyncConnection>();
            TinyIoCContainer.Current.Register<IDataStore,SqliteDataStore>();
            TinyIoCContainer.Current.Register<IRangeMeasurementService, RangeMeasurementService>();
            TinyIoCContainer.Current.Register<IBluetoothService, BluetoothService>();
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
