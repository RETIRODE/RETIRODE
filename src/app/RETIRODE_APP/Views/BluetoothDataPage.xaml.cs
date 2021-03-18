using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Bluetooth;
using RETIRODE_APP.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Essentials;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace RETIRODE_APP.Views
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class BluetoothDataPage : ContentPage
    {
        public BluetoothDataPage()
        {
            InitializeComponent();
            BindingContext = new NewItemViewModel();
        }
    }
}