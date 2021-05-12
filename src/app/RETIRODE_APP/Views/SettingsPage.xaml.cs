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
            if(sipmOknok.Opacity == 0)
            {
                sipmOknok.Opacity = 100;
            }
            else
            {
                sipmOknok.Opacity = 0;
            }
        }

        private void Laser_Switch_Toggled(object sender, ToggledEventArgs e)
        {
            if (laserOknok.Opacity == 0)
            {
                laserOknok.Opacity = 100;
            }
            else
            {
                laserOknok.Opacity = 0;
            }
        }
    }


}