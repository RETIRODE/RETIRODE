using System;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class HomeViewModel : BaseViewModel
    {
        private Boolean _isConnected;
        public Command ConnectCommand { get; }
        public HomeViewModel()
        {
            Title = "Home";
            _isConnected = false;
            Connected = "Disconnected";
            ConnectCommand = new Command(ConnectCom);
        }

        public Command BluetoothLE { get; }

        public void ConnectCom()
        {
            _isConnected = !_isConnected;

            if (_isConnected)
            {
                Connected = "Connected";
            }
            else
            {
                Connected = "Disconnected";
            }
            OnPropertyChanged(nameof(Connected));
        }

        public string Connected { get; set; }
    }
}