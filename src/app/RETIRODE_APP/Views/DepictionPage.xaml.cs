using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

using Urho;
using RETIRODE_APP.ViewModels;

namespace RETIRODE_APP.Views
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class DepictionPage : ContentPage
    {
        public DepictionPage()
        {
            InitializeComponent();
        }

        protected override async void OnAppearing()
        {
            await DepictionSurface.Show<DepictionViewModel>(new ApplicationOptions(assetsFolder: null) { Orientation = ApplicationOptions.OrientationType.LandscapeAndPortrait });

        }
    }
}