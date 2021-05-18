using System;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class HomeViewModel : BaseViewModel
    {
        public HomeViewModel()
        {
            Title = "Home";
            ckeckConnection();
        }

        public void ckeckConnection()
        {
            if (App.isConnected)
                Connected = "Connected";
            else
                Connected = "Disconnected";
        }

        public string Connected { get;set; }
    }
}