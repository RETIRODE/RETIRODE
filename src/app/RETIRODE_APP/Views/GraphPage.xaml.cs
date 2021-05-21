using RETIRODE_APP.Models;
using RETIRODE_APP.ViewModels;
using System.Threading.Tasks;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace RETIRODE_APP.Views
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class GraphPage : ContentPage
    {
        GraphViewModel graphVM;
        public GraphPage()
        {
            DevExpress.XamarinForms.Charts.Initializer.Init();
            InitializeComponent();
            graphVM = new GraphViewModel();
            BindingContext = graphVM;
        }

        public GraphPage(CalibrationItem calibration)
        {
            DevExpress.XamarinForms.Charts.Initializer.Init();
            InitializeComponent();
            startStopBtn.IsEnabled = false;
            refreshBtn.IsEnabled = false;
            graphVM = new GraphViewModel(calibration);
            BindingContext = graphVM;
        }

        protected override void OnAppearing()
        {
            base.OnAppearing();
            graphVM.Init();
        }

        public async Task LoadMeasuredData()
        {
            await graphVM.LoadValues();
        }

        protected override async void OnDisappearing()
        {
            base.OnDisappearing();
            if (graphVM.Measurement)
            {
                await graphVM.StartStopMeasurement();
            }
        }
    }
}