using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public class Point
    {
        public float x { get; set; }
        public float y { get; set; }
        public float z { get; set; }

        public static List<Point> GenerateRandomPoints(int count)
        {
            List<Point> points = new List<Point>();

            var rand = new Random();

            for (int i=0; i< count; i++)
            {
                float x = (float)rand.NextDouble() + rand.Next(-1, 1);
                float y = (float)rand.NextDouble() + rand.Next(-1, 1);
                float z = (float)rand.NextDouble() + rand.Next(-1, 1);

                points.Add(new Point { x = x, y = y, z = z });
            }

            return points;
        }
    }
}
