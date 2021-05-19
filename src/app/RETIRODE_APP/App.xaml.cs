using RETIRODE_APP.Models;
using Nancy.TinyIoc;
using RETIRODE_APP.Services;
using SQLite;
using System;
using Xamarin.Forms;
using RETIRODE_APP.Services.Interfaces;
using RETIRODE_APP.Views;

namespace RETIRODE_APP
{
    public partial class App : Application
    {
        public static bool IsConnected { get; set; }

        public App()
        {
            InitializeComponent();

            TinyIoCContainer.Current.Register<IMockDataStore<Item>,MockDataStore>();
            TinyIoCContainer.Current.Register<SQLiteAsyncConnection>();
            TinyIoCContainer.Current.Register<IDataStore,SqliteDataStore>();
            TinyIoCContainer.Current.Register<IRangeMeasurementService, RangeMeasurementService>();
            TinyIoCContainer.Current.Register<IBluetoothService, BluetoothService>();
            TinyIoCContainer.Current.Register<BluetoothPage>();
            TinyIoCContainer.Current.Register<SettingsPage>();
            TinyIoCContainer.Current.Register<GraphPage>().UsingConstructor(() => new GraphPage());
            MainPage = new AppShell();
        }

        protected override async void OnStart()
        {
            IDataStore _dataStore = TinyIoCContainer.Current.Resolve<IDataStore>();
            await _dataStore.CreateTableAsync<CalibrationItem>();
            await _dataStore.CreateTableAsync<MeasurementItem>();

        }

        protected override void OnSleep()
        {
        }

        protected override void OnResume()
        {
        }
  
    }
}
