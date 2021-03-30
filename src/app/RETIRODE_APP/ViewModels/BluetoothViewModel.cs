using Nancy.TinyIoc;
using RETIRODE_APP.Services;
using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.ViewModels
{
    class BluetoothViewModel : BaseViewModel
    {
        private IBluetoothService _btService => TinyIoCContainer.Current.Resolve<IBluetoothService>();

        public BluetoothViewModel()
        {
            //sample
            _btService.GetBluetoothDevices();
        }
    }
}
