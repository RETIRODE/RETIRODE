using System;
using System.ComponentModel;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace RETIRODE_APP.Views
{
    public partial class HomePage : ContentPage
    {
        public HomePage()
        {
            InitializeComponent();
        }

        private void StartDepictionButton_Clicked(object sender, EventArgs e)
        {
            Navigation.PushAsync(new DepictionPage());
        }
    }
}