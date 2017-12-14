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
        static string input = "http://127.0.0.1/vod/cs.mkv";
        static BSPDMediaSource BSPDMS;
        
        static void Main(string[] args)
        {
            BSPDTest();
            BSPDMS = new BSPDMediaSource();
            BSPDMS.OpenMedia(input);
            System.Threading.Thread.Sleep(600);
            BSPDMS.Close();

            BSPDMS = new BSPDMediaSource();
            BSPDMS.OpenMedia(input);
            System.Threading.Thread.Sleep(600);
            BSPDMS.Close();

            BSPDMS = new BSPDMediaSource();
            BSPDMS.OpenMedia(input);
            System.Threading.Thread.Sleep(600);
            BSPDMS.Close();

            BSPDMS = new BSPDMediaSource();
            BSPDMS.OpenMedia(input);
            System.Threading.Thread.Sleep(600);
            BSPDMS.Close();


            BSPDMS = new BSPDMediaSource();
            BSPDMS.OpenMedia(input);
            System.Threading.Thread.Sleep(600);
            BSPDMS.Close();

            while (true)
            {
                BSPDMS.CheckUpdate();
                Console.WriteLine("----");
            }

        }
    }
}
