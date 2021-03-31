using System;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class HomeViewModel : BaseViewModel
    {
        public Boolean isConnected;
        public Command ConnectCommand { get; }
        public HomeViewModel()
        {
            Title = "Home";
            isConnected = false;
            ConnectCommand = new Command(ConnectCom);

            if (isConnected)
            {
                Connected = "Connected";
            }
            else
            {
                Connected = "Disconnected";
            }
        }

        public Command BluetoothLE { get; }

        public void ConnectCom()
        {
            isConnected = !isConnected;
        }

        public string Connected { get; }
    }
}