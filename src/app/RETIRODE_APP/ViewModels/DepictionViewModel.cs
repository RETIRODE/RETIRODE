﻿using RETIRODE_APP.Models;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Urho;
using Urho.Actions;
using Urho.Gui;

namespace RETIRODE_APP.ViewModels
{
    class DepictionViewModel : Application
    {
        protected const float TouchSensitivity = 2;
        protected float Yaw { get; set; }
        protected float Pitch { get; set; }
        protected bool TouchEnabled { get; set; }
        protected Node CameraNode { get; set; }

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

            await Create3DObjects();
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

            int moveSpeed = 1;
            if (Input.GetKeyDown(Key.W)) CameraNode.Translate(Vector3.UnitZ * moveSpeed * timeStep);
            if (Input.GetKeyDown(Key.S)) CameraNode.Translate(-Vector3.UnitZ * moveSpeed * timeStep);
            if (Input.GetKeyDown(Key.A)) CameraNode.Translate(-Vector3.UnitX * moveSpeed * timeStep);
            if (Input.GetKeyDown(Key.D)) CameraNode.Translate(Vector3.UnitX * moveSpeed * timeStep);

            base.OnUpdate(timeStep);
        }

        private async Task Create3DObjects()
        {
            // Scene
            var scene = new Scene();
            scene.CreateComponent<Octree>();

            var points = Point.GenerateRandomPoints(200);

            foreach(Point p in points)
            {
                plotNode = scene.CreateChild();
                plotNode.Position = new Vector3(p.x, p.y, p.z + 5);
                plotNode.SetScale(0.05f);

                // Sphere Model
                StaticModel modelObject = plotNode.CreateComponent<StaticModel>();
                modelObject.Model = ResourceCache.GetModel("Models/Sphere.mdl");
            }

            // Light
            Node light = scene.CreateChild(name: "light");
            light.SetDirection(new Vector3(0.4f, -0.5f, 0.3f));
            light.CreateComponent<Light>();

            // Camera
            CameraNode = scene.CreateChild(name: "camera");
            Camera camera = CameraNode.CreateComponent<Camera>();

            // Viewport
            Renderer.SetViewport(0, new Viewport(scene, camera, null));

            // Action
            await plotNode.RunActionsAsync(new EaseBackOut(new RotateBy(1, 0, 90, 0)));
            movementsEnabled = true;
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
                    Yaw += TouchSensitivity * camera.Fov / graphics.Height * state.Delta.X;
                    Pitch += TouchSensitivity * camera.Fov / graphics.Height * state.Delta.Y;
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
    }
}
