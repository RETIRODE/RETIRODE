using RETIRODE_APP.ViewModels;
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

        protected override void OnAppearing()
        {
            base.OnAppearing();
            graphVM.Init();
        }

        protected override async void OnDisappearing()
        {
            base.OnDisappearing();
            await graphVM.StopMeasurement();
        }
    }
}