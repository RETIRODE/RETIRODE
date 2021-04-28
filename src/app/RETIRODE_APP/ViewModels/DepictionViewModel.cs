﻿using Nancy.TinyIoc;
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
        protected const float TouchSensitivity = 2;
        protected float Yaw { get; set; }
        protected float Pitch { get; set; }
        protected bool TouchEnabled { get; set; }
        protected Node CameraNode { get; set; }
        protected Scene Scene { get; set; }

        bool movementsEnabled;
        Node plotNode;
        public DepictionViewModel(ApplicationOptions options = null) : base(options) { }
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
            _measurementService.MeasuredDataResponseEvent += _measurementService_MeasuredDataResponseEvent;
            await _measurementService.StartMeasurement();

            await Create3DObjects();
        }

        private async void _measurementService_MeasuredDataResponseEvent(int[] obj)
        {
            foreach (var item in obj)
            {
                var distance = await CalculateDistanceFromTdc(item);
                var point = CreatePoint(distance, 0f, 0f);
                AddPointToScene(point);
            }
        }

        protected override async void Stop()
        {
            _measurementService.MeasuredDataResponseEvent -= _measurementService_MeasuredDataResponseEvent;
            await _measurementService.StopMeasurement(); 
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

        private async Task Create3DObjects()
        {
            // Scene
            Scene = new Scene();
            Scene.CreateComponent<Octree>();

            var points = Point.GenerateRandomPoints(200);

            foreach(Point p in points)
            {
                AddPointToScene(p);
            }

            // Light
            Node light = Scene.CreateChild(name: "light");
            light.SetDirection(new Vector3(0.4f, -0.5f, 0.3f));
            light.CreateComponent<Light>();

            // Camera
            CameraNode = Scene.CreateChild(name: "camera");
            Camera camera = CameraNode.CreateComponent<Camera>();

            // Viewport
            Renderer.SetViewport(0, new Viewport(Scene, camera, null));

            movementsEnabled = true;
        }

        protected void AddPointToScene(Point point)
        {
            plotNode = Scene.CreateChild();
            plotNode.Position = new Vector3(point.x, point.y, point.z + 5);
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
        float Distance(IntVector2 v1, IntVector2 v2)
        {
            return (float)Math.Sqrt((v1.X - v2.X) * (v1.X - v2.X) + (v1.Y - v2.Y) * (v1.Y - v2.Y));
        }

        async Task<float> CalculateDistanceFromTdc(float tdcValue)
        {
            var calibrationList = await _dataStore.GetEntitiesAsync<CalibrationItem>();
            var calibration = new List<CalibrationItem>(calibrationList).FindLast(x => x.Id > 0);

            float tdc0Value = calibration.Tdc_0;
            float tdc62Value = calibration.Tdc_62;
            float tdc125Value = calibration.Tdc_125;

            float tofValue;

            if(tdcValue < 0)
            {
                tofValue = 0f;
            }
            else if(tdcValue < 62.5f)
            {
                tofValue = 0f + (62.5f * ((tdcValue - tdc0Value) / (tdc62Value - tdc0Value)));
            }
            else if (tdcValue < 125f)
            {
                tofValue = 62.5f + (62.5f * ((tdcValue - tdc62Value) / (tdc125Value - tdc62Value)));
            }
            else
            {
                tofValue = 125f + (62.5f * ((tdcValue - tdc125Value) / (tdc125Value - tdc62Value)));
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
    }
}
