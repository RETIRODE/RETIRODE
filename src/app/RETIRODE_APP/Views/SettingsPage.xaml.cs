using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace RETIRODE_APP.Views
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class SettingsPage : ContentPage
    {
        public SettingsPage()
        {
            try
            {
                InitializeComponent();
            }
            catch (Exception ex)
            {
                
            }
            
        }

        private void Sipm_Switch_Toggled(object sender, ToggledEventArgs e)
        {
            if(sipmActual.Text.Equals("-"))
            {
                sipmActual.Text = "0";
                sipmOknok.BackgroundColor = Color.FromHex("#78D778");
                sipmOknok.Text = "Ok";
            }
            else
            {
                sipmActual.Text = "-";
                sipmOknok.BackgroundColor = Color.FromHex("#737475");
                sipmOknok.Text = "-";
            }
        }

        private void Laser_Switch_Toggled(object sender, ToggledEventArgs e)
        {
            if (laserActual.Text.Equals("-"))
            {
                laserActual.Text = "0";
                laserOknok.BackgroundColor = Color.FromHex("#D6676C");
                laserOknok.Text = "Not Ok";
            }
            else
            {
                laserActual.Text = "-";
                laserOknok.BackgroundColor = Color.FromHex("#737475");
                laserOknok.Text = "-";
            }
        }
    }


}