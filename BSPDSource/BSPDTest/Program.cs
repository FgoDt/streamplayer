using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Runtime.InteropServices;

namespace BSPDTest
{
    class Program
    {

        [DllImport("BSPD")]
        private static extern void BSPDTest();



        static string yuvfilepath = "./test.yuv";
        static string input = "C:/Users/rt-zl/Desktop/ffmpeg/bin/abc.mov";
        static BSPDMediaSource BSPDMS;
        
        static void Main(string[] args)
        {
            BSPDMS = new BSPDMediaSource();
            BSPDMS.OpenMedia(input);

            while (true)
            {
                BSPDMS.CheckUpdate();
                Console.WriteLine("----");
            }

        }
    }
}
