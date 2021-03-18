using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Input;

namespace RETIRODE_APP.ViewModels
{
    public class BluetoothViewModel : BaseViewModel
    {
        public BluetoothViewModel()
        {
            Title = "Bluetooth";
        }

        public ICommand OpenWebCommand { get; }
    }
}
