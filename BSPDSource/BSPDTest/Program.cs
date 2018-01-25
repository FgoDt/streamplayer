using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace BSPDTest
{
    class Program
    {

        [DllImport("BSPD")]
        private static extern void BSPDTest();



        static string yuvfilepath = "./test.yuv";
        static string input = "http://192.168.3.69/vod/cs.mkv";
        static BSPDMediaSource BSPDMS;
        
        static void Main(string[] args)
        {
            BSPDMS = new BSPDMediaSource();
            BSPDMS.MediaStateChage += BSPDMS_MediaStateChage;
            BSPDMS.OpenMedia(input);
          

            BSPDMS.Close();
            while (true)
            {
            //    BSPDMS.CheckUpdate();
            BSPDMS = new BSPDMediaSource();
            BSPDMS.MediaStateChage += BSPDMS_MediaStateChage;
            BSPDMS.OpenMedia(input);
            System.Threading.Thread.Sleep(10);
                BSPDMS.Close();
              
            }

        }

        private static void BSPDMS_MediaStateChage(BSPDMediaSource.BSPDMediaState sate)
        {
            Console.WriteLine("state:" + sate);
        }
    }
}
