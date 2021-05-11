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
        }


        private string _connected;

        public string Connected {
            get { if (App.isConnected)
                    return "Connected";
                else
                    return "Disconected";
            }
            set
            {
                _connected = value;
            } 
        }
    }
}