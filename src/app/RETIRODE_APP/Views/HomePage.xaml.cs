using Plugin.BLE;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Threading.Tasks;
using Xamarin.Essentials;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace RETIRODE_APP.Views
{
    [DesignTimeVisible(false)]
    public partial class HomePage : ContentPage
    {
        public HomePage()
        {
            InitializeComponent();
        }

        private void StartDepictionButton_Clicked(object sender, EventArgs e)
        {
            if (!App.isConnected)
            {
                Navigation.PushAsync(new BluetoothPage());
            }
            else if (!App.isCalibrated)
            {
                Navigation.PushAsync(new SettingsPage());
            }
            else
            {
                Navigation.PushAsync(new DepictionPage());
            }
        }
    }
}