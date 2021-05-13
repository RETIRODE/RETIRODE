using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Urho;
using Urho.Gui;

namespace RETIRODE_APP.ViewModels
{
    class DepictionViewModel : Application
    {
        private IDataStore _dataStore => TinyIoCContainer.Current.Resolve<IDataStore>();
        private IRangeMeasurementService _measurementService => TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
        private const float TouchSensitivity = 2;
        private float Yaw { get; set; }
        private float Pitch { get; set; }
        private Node CameraNode { get; set; }
        private Scene Scene { get; set; }
        private DateTime StartMeasuringTime { get; set; }
        private float Tdc0Value { get; set; }
        private float Tdc62Value { get; set; }
        private float Tdc125Value { get; set; }
        private CalibrationItem Calibration { get; set; }


        public IRangeMeasurementService rangeMeasurementService;

        bool movementsEnabled;
        Node plotNode;
        public DepictionViewModel(ApplicationOptions options = null) : base(options) {
            rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            rangeMeasurementService.StartMeasurement();
        }

        private void RangeMeasurementService_MeasuredDataResponseEvent(int[] obj)
        {
            
        }

        private static ApplicationOptions SetOptions(ApplicationOptions options)
        {
            options.TouchEmulation = true;
            return options;
        }
        static DepictionViewModel()
        {
            UnhandledException += (s, e) =>
            {
                if (Debugger.IsAttached)
                    Debugger.Break();
                e.Handled = true;
            };
        }
        protected override async void Start()
        {
            base.Start();
            SetCalibration();
            //_measurementService.MeasuredDataResponseEvent += _measurementService_MeasuredDataResponseEvent;
            //await _measurementService.StartMeasurement();
            StartMeasuringTime = DateTime.Now;
            await CreateScene();
            AddTestDataFromDB();
        }

        private void _measurementService_MeasuredDataResponseEvent(int[] obj)
        {
            foreach (var item in obj)
            {
                var distance = CalculateDistanceFromTdc(item);
                var point = CreatePoint(distance, 0f, 0f);
                AddPointToScene(point);
            }
        }

        protected override async void Stop()
        {
           // _measurementService.MeasuredDataResponseEvent -= _measurementService_MeasuredDataResponseEvent;
           // await _measurementService.StopMeasurement(); 
        }


        protected override void OnUpdate(float timeStep)
        {
            if (Input.NumTouches > 0)
            {
                // move
                if (Input.NumTouches == 1)
                {
                    MoveCameraByTouches(timeStep);

                }
                else if (Input.NumTouches == 2)
                {
                    TouchState state1 = Input.GetTouch(0);
                    TouchState state2 = Input.GetTouch(1);

                    var distance1 = Distance(state1.Position, state2.Position);
                    var distance2 = Distance(state1.LastPosition, state2.LastPosition);

                    CameraNode.Translate(new Vector3(0, 0, (distance1 - distance2) / 300f));
                }


            }

            int moveSpeed = 2;
            if (Input.GetKeyDown(Key.W)) CameraNode.Translate(Vector3.UnitZ * moveSpeed * timeStep);
            if (Input.GetKeyDown(Key.S)) CameraNode.Translate(-Vector3.UnitZ * moveSpeed * timeStep);
            if (Input.GetKeyDown(Key.A)) CameraNode.Translate(-Vector3.UnitX * moveSpeed * timeStep);
            if (Input.GetKeyDown(Key.D)) CameraNode.Translate(Vector3.UnitX * moveSpeed * timeStep);
            if (Input.GetKeyDown(Key.E)) AddPointToScene(new Point { x = 0, y = 0, z = 0});

            base.OnUpdate(timeStep);
        }

        private async Task CreateScene()
        {
            // Scene
            Scene = new Scene();
            Scene.CreateComponent<Octree>();

            // Light
            
            Node zoneNode = Scene.CreateChild(name: "zone");
            Zone zone = zoneNode.CreateComponent<Zone>();
            zone.SetBoundingBox(new BoundingBox(-10000.0f, 10000.0f));
            zone.AmbientColor = new Urho.Color(0.5f, 0.5f, 0.5f);
            // Camera
            CameraNode = Scene.CreateChild(name: "camera");
            CameraNode.Position = new Vector3(1.2f, 0f, -5f);
            Camera camera = CameraNode.CreateComponent<Camera>();
            

            // Viewport
            Renderer.SetViewport(0, new Viewport(Scene, camera, null));

            movementsEnabled = true;
        }
        protected async void AddTestDataFromDB()
        {
            
            var list = await _dataStore.ListMeasurementByCalibrationAsync(Calibration.Id);
            
            await Urho.Application.ToMainThreadAsync();
            StartMeasuringTime = DateTime.Now;
            foreach (var item in list)
            {
                TimeSpan span = DateTime.Now - StartMeasuringTime;
                var p = new Point
                {
                    x = (float)item.Id/10,
                    y = CalculateDistanceFromTdc(item.Tdc_value),
                };
                AddPointToScene(p);
            }
                

        }

        protected async void AddPointToScene(Point point)
        {
            await Urho.Application.ToMainThreadAsync();
            plotNode = Scene.CreateChild();
            plotNode.Position = new Vector3(point.x, point.y, point.z+0.5f);
            plotNode.SetScale(0.05f);

            // Sphere Model
            StaticModel modelObject = plotNode.CreateComponent<StaticModel>();
            modelObject.Model = ResourceCache.GetModel("Models/Sphere.mdl");
        }

        protected void MoveCameraByTouches(float timeStep)
        {
            if (!movementsEnabled || CameraNode == null)
                return;

            var input = Input;
            for (uint i = 0, num = input.NumTouches; i < num; ++i)
            {
                TouchState state = input.GetTouch(i);
                if (state.TouchedElement != null)
                    continue;

                if (state.Delta.X != 0 || state.Delta.Y != 0)
                {
                    var camera = CameraNode.GetComponent<Camera>();
                    if (camera == null)
                        return;

                    var graphics = Graphics;
                    Yaw -= TouchSensitivity * camera.Fov / graphics.Height * state.Delta.X;
                    Pitch -= TouchSensitivity * camera.Fov / graphics.Height * state.Delta.Y;
                    CameraNode.Rotation = new Quaternion(Pitch, Yaw, 0);
                }
                else
                {
                    var cursor = UI.Cursor;
                    if (cursor != null && cursor.Visible)
                        cursor.Position = state.Position;
                }
            }
        }
        private float Distance(IntVector2 v1, IntVector2 v2)
        {
            return (float)Math.Sqrt((v1.X - v2.X) * (v1.X - v2.X) + (v1.Y - v2.Y) * (v1.Y - v2.Y));
        }

        private float CalculateDistanceFromTdc(float tdcValue)
        {
            float tofValue;

            if(tdcValue < 0)
            {
                tofValue = 0f;
            }
            else if(tdcValue < 62.5f)
            {
                tofValue = 0f + (62.5f * ((tdcValue - Tdc0Value) / (Tdc62Value - Tdc0Value)));
            }
            else if (tdcValue < 125f)
            {
                tofValue = 62.5f + (62.5f * ((tdcValue - Tdc62Value) / (Tdc125Value - Tdc62Value)));
            }
            else
            {
                tofValue = 125f + (62.5f * ((tdcValue - Tdc125Value) / (Tdc125Value - Tdc62Value)));
            }

            float distance = 0.15f * tofValue;
            return distance;
        }

        Point CreatePoint(float distance, float angle_1, float angle_2)
        {
            float Px = distance * (float)(Math.Sin(angle_1) * Math.Cos(angle_2));
            float Py = distance * (float)(Math.Sin(angle_1) * Math.Sin(angle_2));
            float Pz = distance * (float)Math.Cos(angle_1);

            return (new Point { x = Px, y = Py, z = Pz });
        }

        private async void SetCalibration()
        {
            var calibrationList = await _dataStore.GetEntitiesAsync<CalibrationItem>();
            var calibration = new List<CalibrationItem>(calibrationList).FindLast(x => x.Id > 0);

            Tdc0Value = calibration.Tdc_0;
            Tdc62Value = calibration.Tdc_62;
            Tdc125Value = calibration.Tdc_125;
            Calibration = calibration;
        }
    }
}
